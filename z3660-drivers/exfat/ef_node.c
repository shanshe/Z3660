/*--------------------------------------------------------------------------

  exFAT filesystem for MorphOS:
  Copyright © 2014-2015 Rupert Hausberger

  FAT12/16/32 filesystem handler:
  Copyright © 2006 Marek Szyprowski
  Copyright © 2007-2008 The AROS Development Team

  exFAT implementation library:
  Copyright © 2010-2013 Andrew Nayenko


  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  The full GNU General Public License is included in this distribution in the
  file called LICENSE.

--------------------------------------------------------------------------*/

#include "include.h"

#ifdef __MORPHOS__
	#pragma pack(2)
#endif

/* on-disk nodes iterator */
struct iterator {
	ULONG cluster;
	UQUAD offset;
	int contiguous;
	char* chunk;
};

#ifdef __MORPHOS__
	#pragma pack()
#endif

/*------------------------------------------------------------------------*/

struct exFAT_Node* exfat_get_node(struct exFAT_Node* node)
{
	/* if we switch to multi-threaded mode we will need atomic
	increment here and atomic decrement in exfat_put_node() */
	node->en_References++;
	return node;
}

void exfat_put_node(struct exFAT* ef, struct exFAT_Node* node)
{
	if (--node->en_References < 0)
	{
		char buffer[EXFAT_NAME_MAX + 1];
		exfat_get_name(node, buffer, EXFAT_NAME_MAX);
		exfat_fatal("reference counter of '%s' is below zero", buffer);
	}

	if (node->en_References == 0)
	{
		if (node->en_Flags & EXFAT_ATTRIB_DIRTY)
			exfat_flush_node(ef, node);
		if (node->en_Flags & EXFAT_ATTRIB_UNLINKED)
		{
			/* free all clusters and node structure itself */
			exfat_truncate(ef, node, 0);
			exfat_free(ef, node);
		}
		if (ef->ef_CMap.dirty)
			exfat_flush_cmap(ef);
	}
}

/*------------------------------------------------------------------------*/
/**
 * Cluster + offset from the beginning of the directory to absolute offset.
 */
static UQUAD co2o(struct exFAT* ef, ULONG cluster, UQUAD offset)
{
	return exfat_c2o(ef, cluster) + offset % CLUSTER_SIZE(*ef->ef_SB);
}

static int opendir(struct exFAT* ef, const struct exFAT_Node* dir, struct iterator* it)
{
	if (!(dir->en_Flags & EXFAT_ATTRIB_DIR)) {
		exfat_fatal("not a directory");
		return -ERROR_OBJECT_WRONG_TYPE;
	}
	it->cluster = dir->en_StartCluster;
	it->offset = 0;
	it->contiguous = IS_CONTIGUOUS(*dir);
	if (!(it->chunk = exfat_alloc(ef, CLUSTER_SIZE(*ef->ef_SB), FALSE))) {
		exfat_error("out of memory");
		return -ERROR_NO_FREE_STORE;
	}
	exfat_pread(ef->ef_Dev, it->chunk, CLUSTER_SIZE(*ef->ef_SB), exfat_c2o(ef, it->cluster));
	return 0;
}

static void closedir(struct exFAT* ef, struct iterator* it)
{
	it->cluster = 0;
	it->offset = 0;
	it->contiguous = 0;
	exfat_free(ef, it->chunk);
	it->chunk = NULL;
}

static int fetch_next_entry(struct exFAT* ef, const struct exFAT_Node* parent, struct iterator* it)
{
	/* move iterator to the next entry in the directory */
	it->offset += sizeof(struct exFAT_Entry);
	/* fetch the next cluster if needed */
	if ((it->offset & (CLUSTER_SIZE(*ef->ef_SB) - 1)) == 0)
	{
		/* reached the end of directory; the caller should check this
		condition too */
		if (it->offset >= parent->en_Size)
			return 0;
		it->cluster = exfat_next_cluster(ef, parent, it->cluster);
		if (CLUSTER_INVALID(it->cluster))
		{
			exfat_error("invalid cluster 0x%lx while reading directory", it->cluster);
			return 1;
		}
		exfat_pread(ef->ef_Dev, it->chunk, CLUSTER_SIZE(*ef->ef_SB), exfat_c2o(ef, it->cluster));
	}
	return 0;
}

static struct exFAT_Node* allocate_node(struct exFAT* ef)
{
	struct exFAT_Node *node;

	if (!(node = exfat_alloc(ef, sizeof(struct exFAT_Node), TRUE)))
		exfat_error("failed to allocate node");

	return node;
}

static void init_node_meta1(struct exFAT_Node* node, const struct exFAT_EntryMeta1* meta1)
{
	node->en_Flags = le16_to_cpu(meta1->ee_Attrib);
	exfat_exfat2amiga(meta1->ee_MDate, meta1->ee_MTime, meta1->ee_MTimeCS, &node->en_MTime);
	exfat_exfat2amiga(meta1->ee_ADate, meta1->ee_ATime, 0, &node->en_ATime);
}

static void init_node_meta2(struct exFAT_Node* node, const struct exFAT_EntryMeta2* meta2)
{
	node->en_Size = le64_to_cpu(meta2->ee_Size);
	node->en_StartCluster = le32_to_cpu(meta2->ee_StartCluster);
	node->en_FPtrCluster = node->en_StartCluster;
	if (meta2->ee_Flags & EXFAT_FLAG_CONTIGUOUS)
		node->en_Flags |= EXFAT_ATTRIB_CONTIGUOUS;
}

static const struct exFAT_Entry* get_entry_ptr(const struct exFAT* ef, const struct iterator* it)
{
	return (const struct exFAT_Entry*)(it->chunk + it->offset % CLUSTER_SIZE(*ef->ef_SB));
}

/*
 * Reads one entry in directory at position pointed by iterator and fills
 * node structure.
 */
static int readdir(struct exFAT* ef, const struct exFAT_Node* parent, struct exFAT_Node** node, struct iterator* it)
{
	int rc = -ERROR_OBJECT_NOT_FOUND; //-EIO;
	const struct exFAT_Entry* entry;
	const struct exFAT_EntryMeta1* meta1;
	const struct exFAT_EntryMeta2* meta2;
	const struct exFAT_EntryName* file_name;
	const struct exFAT_EntryUpperCase* upcase;
	const struct exFAT_EntryBitmap* bitmap;
	const struct exFAT_EntryLabel* label;
	UBYTE continuations = 0;
	le16_t* namep = NULL;
	UWORD reference_checksum = 0;
	UWORD actual_checksum = 0;
	UQUAD real_size = 0;

	*node = NULL;

	for (;;)
	{
		if (it->offset >= parent->en_Size)
		{
			if (continuations != 0)
			{
				exfat_error("expected 0x%02x continuations", continuations);
				goto error;
			}
			//return -ENOENT; /* that's OK, means end of directory */
			return -ERROR_NO_MORE_ENTRIES; /* that's OK, means end of directory */
		}

		entry = get_entry_ptr(ef, it);
		switch (entry->ee_Type)
		{
		case EXFAT_ENTRY_FILE:
			if (continuations != 0)
			{
				exfat_error("expected 0x%02x continuations before new entry", continuations);
				goto error;
			}
			meta1 = (const struct exFAT_EntryMeta1*) entry;
			continuations = meta1->ee_Continuations;
			/* each file entry must have at least 2 continuations:
			info and name */
			if (continuations < 2)
			{
				exfat_error("too few continuations (0x%02x)", continuations);
				goto error;
			}
			reference_checksum = le16_to_cpu(meta1->ee_Checksum);
			actual_checksum = exfat_start_checksum(meta1);
			*node = allocate_node(ef);
			if (*node == NULL)
			{
				rc = -ERROR_NO_FREE_STORE;
				goto error;
			}
			/* new node has zero reference counter */
			(*node)->en_EntryCluster = it->cluster;
			(*node)->en_EntryOffset = it->offset;
			init_node_meta1(*node, meta1);
			namep = (*node)->en_Name;
			break;

		case EXFAT_ENTRY_FILE_INFO:
			if (continuations < 2)
			{
				exfat_error("unexpected continuation (0x%02x)", continuations);
				goto error;
			}
			meta2 = (const struct exFAT_EntryMeta2*) entry;
			if (meta2->ee_Flags & ~(EXFAT_FLAG_ALWAYS1 | EXFAT_FLAG_CONTIGUOUS))
			{
				exfat_error("unknown flags in meta2 (0x%02x)", meta2->ee_Flags);
				goto error;
			}
			init_node_meta2(*node, meta2);
			actual_checksum = exfat_add_checksum(entry, actual_checksum);
			real_size = le64_to_cpu(meta2->ee_RealSize);
			/* empty files must be marked as non-contiguous */
			if ((*node)->en_Size == 0 && (meta2->ee_Flags & EXFAT_FLAG_CONTIGUOUS))
			{
				exfat_error("empty file marked as contiguous (0x%02x)", meta2->ee_Flags);
				goto error;
			}
			/* directories must be aligned on at cluster boundary */
			if (((*node)->en_Flags & EXFAT_ATTRIB_DIR) &&
				(*node)->en_Size % CLUSTER_SIZE(*ef->ef_SB) != 0)
			{
				exfat_error("directory has invalid size %llu bytes", (*node)->en_Size);
				goto error;
			}
			--continuations;
			break;

		case EXFAT_ENTRY_FILE_NAME:
			if (continuations == 0)
			{
				exfat_error("unexpected continuation");
				goto error;
			}
			file_name = (const struct exFAT_EntryName*) entry;
			actual_checksum = exfat_add_checksum(entry, actual_checksum);

			CopyMem((CONST APTR)file_name->ee_Name, namep, EXFAT_ENAME_MAX * sizeof(le16_t));
			namep += EXFAT_ENAME_MAX;
			if (--continuations == 0)
			{
				/*
				There are two fields that contain file size. Maybe they
				plan to add compression support in the future and one of
				those fields is visible (uncompressed) size and the other
				is real (compressed) size. Anyway, currently it looks like
				exFAT does not support compression and both fields must be
				equal.

				There is an exception though: pagefile.sys (its real_size
				is always 0).
				*/
				if (real_size != (*node)->en_Size)
				{
					char buffer[EXFAT_NAME_MAX + 1];

					exfat_get_name(*node, buffer, EXFAT_NAME_MAX);
					exfat_error("'%s' real size does not equal to size (%llu != %llu)", buffer, real_size, (*node)->en_Size);
					goto error;
				}
				if (actual_checksum != reference_checksum)
				{
					char buffer[EXFAT_NAME_MAX + 1];

					exfat_get_name(*node, buffer, EXFAT_NAME_MAX);
					exfat_error("'%s' has invalid ee_Checksum (0x%x != 0x%x)", buffer, actual_checksum, reference_checksum);
					goto error;
				}
				if (fetch_next_entry(ef, parent, it) != 0)
					goto error;
				return 0; /* entry completed */
			}
			break;

		case EXFAT_ENTRY_UPCASE:
			if (ef->ef_UpCase != NULL)
				break;
			upcase = (const struct exFAT_EntryUpperCase*) entry;
			if (CLUSTER_INVALID(le32_to_cpu(upcase->ee_StartCluster)))
			{
				exfat_error("invalid cluster 0x%lx in upcase table", le32_to_cpu(upcase->ee_StartCluster));
				goto error;
			}
			if (le64_to_cpu(upcase->ee_Size) == 0 ||
				le64_to_cpu(upcase->ee_Size) > 0xffff * sizeof(UWORD) ||
				le64_to_cpu(upcase->ee_Size) % sizeof(UWORD) != 0)
			{
				exfat_error("bad upcase table size (%llu bytes)", le64_to_cpu(upcase->ee_Size));
				goto error;
			}
			if (!(ef->ef_UpCase = exfat_alloc(ef, le64_to_cpu(upcase->ee_Size), FALSE))) {
				exfat_error("failed to allocate upcase table (%llu bytes)", le64_to_cpu(upcase->ee_Size));
				rc = -ERROR_NO_FREE_STORE;
				goto error;
			}
			ef->ef_UpCaseChars = le64_to_cpu(upcase->ee_Size) / sizeof(le16_t);
			exfat_pread(ef->ef_Dev, ef->ef_UpCase, le64_to_cpu(upcase->ee_Size), exfat_c2o(ef, le32_to_cpu(upcase->ee_StartCluster)));
			break;

		case EXFAT_ENTRY_BITMAP:
			bitmap = (const struct exFAT_EntryBitmap*) entry;
			ef->ef_CMap.start_cluster = le32_to_cpu(bitmap->ee_StartCluster);
			if (CLUSTER_INVALID(ef->ef_CMap.start_cluster))
			{
				exfat_error("invalid cluster 0x%lx in clusters bitmap", ef->ef_CMap.start_cluster);
				goto error;
			}
			ef->ef_CMap.size = le32_to_cpu(ef->ef_SB->es_ClusterCount) - EXFAT_FIRST_DATA_CLUSTER;
			if (le64_to_cpu(bitmap->ee_Size) < (ef->ef_CMap.size + 7) / 8)
			{
				exfat_error("invalid clusters bitmap size: %llu (expected at least %lu)", le64_to_cpu(bitmap->ee_Size), (ef->ef_CMap.size + 7) / 8);
				goto error;
			}
			/* FIXME bitmap can be rather big, up to 512 MB */
			ef->ef_CMap.chunk_size = ef->ef_CMap.size;
			if (!(ef->ef_CMap.chunk = exfat_alloc(ef, le64_to_cpu(bitmap->ee_Size), FALSE))) {
				exfat_error("failed to allocate clusters bitmap chunk (%llu bytes)", le64_to_cpu(bitmap->ee_Size));
				rc = -ERROR_NO_FREE_STORE;
				goto error;
			}
			exfat_pread(ef->ef_Dev, ef->ef_CMap.chunk, le64_to_cpu(bitmap->ee_Size), exfat_c2o(ef, ef->ef_CMap.start_cluster));
			break;

		case EXFAT_ENTRY_LABEL:
			label = (const struct exFAT_EntryLabel*) entry;
			if (label->ee_Length > EXFAT_ENAME_MAX)
			{
				exfat_error("too long label (%u chars)", label->ee_Length);
				goto error;
			}
			if (utf16_to_utf8(ef->ef_Label, label->ee_Name, sizeof(ef->ef_Label), EXFAT_ENAME_MAX) != 0)
				goto error;
			break;

		default:
			if (entry->ee_Type & EXFAT_ENTRY_VALID)
			{
				exfat_error("unknown entry type 0x%02x", entry->ee_Type);
				goto error;
			}
			break;
		}

		if (fetch_next_entry(ef, parent, it) != 0)
			goto error;
	}
	/* we never reach here */

error:
	exfat_free(ef, *node);
	*node = NULL;
	return rc;
}

int exfat_cache_directory(struct exFAT* ef, struct exFAT_Node* dir)
{
	struct iterator it;
	int rc;
	struct exFAT_Node* node;
	struct exFAT_Node* current = NULL;

	if (dir->en_Flags & EXFAT_ATTRIB_CACHED)
		return 0; /* already cached */

	rc = opendir(ef, dir, &it);
	if (rc != 0)
		return rc;
	while ((rc = readdir(ef, dir, &node, &it)) == 0)
	{
		node->en_Parent = dir;
		if (current != NULL)
		{
			current->en_Next = node;
			node->en_Prev = current;
		}
		else
			dir->en_Child = node;

		current = node;
	}
	closedir(ef, &it);

	//if (rc != -ENOENT)
	if (rc != -ERROR_NO_MORE_ENTRIES)
	{
		/* rollback */
		for (current = dir->en_Child; current; current = node)
		{
			node = current->en_Next;
			exfat_free(ef, current);
		}
		dir->en_Child = NULL;
		return rc;
	}

	dir->en_Flags |= EXFAT_ATTRIB_CACHED;
	return 0;
}

/*------------------------------------------------------------------------*/

static void tree_attach(struct exFAT_Node* dir, struct exFAT_Node* node)
{
	node->en_Parent = dir;
	if (dir->en_Child)
	{
		dir->en_Child->en_Prev = node;
		node->en_Next = dir->en_Child;
	}
	dir->en_Child = node;
}

static void tree_detach(struct exFAT_Node* node)
{
	if (node->en_Prev)
		node->en_Prev->en_Next = node->en_Next;
	else /* this is the first node in the list */
		node->en_Parent->en_Child = node->en_Next;
	if (node->en_Next)
		node->en_Next->en_Prev = node->en_Prev;
	node->en_Parent = NULL;
	node->en_Prev = NULL;
	node->en_Next = NULL;
}

static void reset_cache(struct exFAT* ef, struct exFAT_Node* node)
{
	while (node->en_Child)
	{
		struct exFAT_Node* p = node->en_Child;
		reset_cache(ef, p);
		tree_detach(p);
		exfat_free(ef, p);
	}
	node->en_Flags &= ~EXFAT_ATTRIB_CACHED;
	if (node->en_References != 0)
	{
		char buffer[EXFAT_NAME_MAX + 1];
		exfat_get_name(node, buffer, EXFAT_NAME_MAX);
		exfat_warn("non-zero reference counter (%d) for '%s'", node->en_References, buffer);
	}
	while (node->en_References)
		exfat_put_node(ef, node);
}

void exfat_reset_cache(struct exFAT* ef)
{
	reset_cache(ef, ef->ef_Root);
}

/*------------------------------------------------------------------------*/

void next_entry(struct exFAT* ef, const struct exFAT_Node* parent, ULONG* cluster, UQUAD* offset)
{
	*offset += sizeof(struct exFAT_Entry);
	if (*offset % CLUSTER_SIZE(*ef->ef_SB) == 0)
		/* next cluster cannot be invalid */
		*cluster = exfat_next_cluster(ef, parent, *cluster);
}

void exfat_flush_node(struct exFAT* ef, struct exFAT_Node* node)
{
	ULONG cluster;
	UQUAD offset;
	UQUAD meta1_offset, meta2_offset;
	struct exFAT_EntryMeta1 meta1;
	struct exFAT_EntryMeta2 meta2;

	if (ef->ef_ReadOnly)
		exfat_fatal("unable to flush node to read-only FS");

	if (node->en_Parent == NULL)
		return; /* do not flush unlinked node */

	cluster = node->en_EntryCluster;
	offset = node->en_EntryOffset;
	meta1_offset = co2o(ef, cluster, offset);
	next_entry(ef, node->en_Parent, &cluster, &offset);
	meta2_offset = co2o(ef, cluster, offset);

	exfat_pread(ef->ef_Dev, &meta1, sizeof(meta1), meta1_offset);
	if (meta1.ee_Type != EXFAT_ENTRY_FILE)
		exfat_fatal("invalid type of meta1: 0x%hhx", meta1.ee_Type);
	meta1.ee_Attrib = cpu_to_le16(node->en_Flags);
	exfat_amiga2exfat(&node->en_MTime, &meta1.ee_MDate, &meta1.ee_MTime, &meta1.ee_MTimeCS);
	exfat_amiga2exfat(&node->en_ATime, &meta1.ee_ADate, &meta1.ee_ATime, NULL);

	exfat_pread(ef->ef_Dev, &meta2, sizeof(meta2), meta2_offset);
	if (meta2.ee_Type != EXFAT_ENTRY_FILE_INFO)
		exfat_fatal("invalid type of meta2: 0x%hhx", meta2.ee_Type);
	meta2.ee_Size = meta2.ee_RealSize = cpu_to_le64(node->en_Size);
	meta2.ee_StartCluster = cpu_to_le32(node->en_StartCluster);
	meta2.ee_Flags = EXFAT_FLAG_ALWAYS1;
	/* empty files must not be marked as contiguous */
	if (node->en_Size != 0 && IS_CONTIGUOUS(*node))
		meta2.ee_Flags |= EXFAT_FLAG_CONTIGUOUS;
	/* name hash remains unchanged, no need to recalculate it */

	meta1.ee_Checksum = exfat_calc_checksum(&meta1, &meta2, node->en_Name);

	exfat_pwrite(ef->ef_Dev, &meta1, sizeof(meta1), meta1_offset);
	exfat_pwrite(ef->ef_Dev, &meta2, sizeof(meta2), meta2_offset);

	node->en_Flags &= ~EXFAT_ATTRIB_DIRTY;
}

/*------------------------------------------------------------------------*/

static void erase_entry(struct exFAT* ef, struct exFAT_Node* node)
{
	ULONG cluster = node->en_EntryCluster;
	UQUAD offset = node->en_EntryOffset;
	int name_entries = DIV_ROUND_UP(utf16_length(node->en_Name), EXFAT_ENAME_MAX);
	UBYTE entry_type;

	entry_type = EXFAT_ENTRY_FILE & ~EXFAT_ENTRY_VALID;
	exfat_pwrite(ef->ef_Dev, &entry_type, 1, co2o(ef, cluster, offset));

	next_entry(ef, node->en_Parent, &cluster, &offset);
	entry_type = EXFAT_ENTRY_FILE_INFO & ~EXFAT_ENTRY_VALID;
	exfat_pwrite(ef->ef_Dev, &entry_type, 1, co2o(ef, cluster, offset));

	while (name_entries--)
	{
		next_entry(ef, node->en_Parent, &cluster, &offset);
		entry_type = EXFAT_ENTRY_FILE_NAME & ~EXFAT_ENTRY_VALID;
		exfat_pwrite(ef->ef_Dev, &entry_type, 1, co2o(ef, cluster, offset));
	}
}

static int shrink_directory(struct exFAT* ef, struct exFAT_Node* dir, UQUAD deleted_offset)
{
	const struct exFAT_Node* node;
	const struct exFAT_Node* last_node;
	UQUAD entries = 0;
	UQUAD new_size;
	int rc;

	if (!(dir->en_Flags & EXFAT_ATTRIB_DIR))
		exfat_fatal("attempted to shrink a file");
	if (!(dir->en_Flags & EXFAT_ATTRIB_CACHED))
		exfat_fatal("attempted to shrink uncached directory");

	for (last_node = node = dir->en_Child; node; node = node->en_Next)
	{
		if (deleted_offset < node->en_EntryOffset)
		{
			/* there are other entries after the removed one, no way to shrink
			this directory */
			return 0;
		}
		if (last_node->en_EntryOffset < node->en_EntryOffset)
			last_node = node;
	}

	if (last_node)
	{
		/* offset of the last entry */
		entries += last_node->en_EntryOffset / sizeof(struct exFAT_Entry);
		/* two subentries with meta info */
		entries += 2;
		/* subentries with file name */
		entries += DIV_ROUND_UP(utf16_length(last_node->en_Name), EXFAT_ENAME_MAX);
	}

	new_size = DIV_ROUND_UP(entries * sizeof(struct exFAT_Entry), CLUSTER_SIZE(*ef->ef_SB)) * CLUSTER_SIZE(*ef->ef_SB);
	if (new_size == 0) /* directory always has at least 1 cluster */
		new_size = CLUSTER_SIZE(*ef->ef_SB);
	if (new_size == dir->en_Size)
		return 0;
	rc = exfat_truncate(ef, dir, new_size);
	if (rc != 0)
		return rc;
	return 0;
}

static int delete(struct exFAT* ef, struct exFAT_Node* node)
{
	struct exFAT_Node* parent = node->en_Parent;
	UQUAD deleted_offset = node->en_EntryOffset;
	int rc;

	exfat_get_node(parent);
	erase_entry(ef, node);
	exfat_update_mtime(parent);
	tree_detach(node);
	rc = shrink_directory(ef, parent, deleted_offset);
	exfat_put_node(ef, parent);
	/* file clusters will be freed when node reference counter becomes 0 */
	node->en_Flags |= EXFAT_ATTRIB_UNLINKED;
	return rc;
}

int exfat_unlink(struct exFAT* ef, struct exFAT_Node* node)
{
	if (node->en_Flags & EXFAT_ATTRIB_DIR)
		return -ERROR_OBJECT_WRONG_TYPE; //-EISDIR;
	return delete(ef, node);
}

int exfat_rmdir(struct exFAT* ef, struct exFAT_Node* node)
{
	if (!(node->en_Flags & EXFAT_ATTRIB_DIR))
		return -ERROR_OBJECT_WRONG_TYPE; //-ENOTDIR;
	/* check that directory is empty */
	exfat_cache_directory(ef, node);
	if (node->en_Child)
		return -ERROR_OBJECT_IN_USE; //ENOTEMPTY;

	return delete(ef, node);
}

/*------------------------------------------------------------------------*/

static int grow_directory(struct exFAT* ef, struct exFAT_Node* dir, UQUAD asize, ULONG difference)
{
	return exfat_truncate(ef, dir, DIV_ROUND_UP(asize + difference, CLUSTER_SIZE(*ef->ef_SB)) * CLUSTER_SIZE(*ef->ef_SB));
}

static int find_slot(struct exFAT* ef, struct exFAT_Node* dir, ULONG* cluster, UQUAD* offset, int subentries)
{
	struct iterator it;
	int rc;
	const struct exFAT_Entry* entry;
	int contiguous = 0;

	rc = opendir(ef, dir, &it);
	if (rc != 0)
		return rc;
	for (;;)
	{
		if (contiguous == 0)
		{
			*cluster = it.cluster;
			*offset = it.offset;
		}
		entry = get_entry_ptr(ef, &it);
		if (entry->ee_Type & EXFAT_ENTRY_VALID)
			contiguous = 0;
		else
			contiguous++;
		if (contiguous == subentries)
			break;	/* suitable slot is found */
		if (it.offset + sizeof(struct exFAT_Entry) >= dir->en_Size)
		{
			rc = grow_directory(ef, dir, dir->en_Size, (subentries - contiguous) * sizeof(struct exFAT_Entry));
			if (rc != 0)
			{
				closedir(ef, &it);
				return rc;
			}
		}
		if (fetch_next_entry(ef, dir, &it) != 0)
		{
			closedir(ef, &it);
			return -ERROR_OBJECT_NOT_FOUND; //EIO;
		}
	}
	closedir(ef, &it);
	return 0;
}

static int write_entry(struct exFAT* ef, struct exFAT_Node* dir, const le16_t* name, ULONG cluster, UQUAD offset, UWORD attrib)
{
	struct exFAT_Node* node;
	struct exFAT_EntryMeta1 meta1;
	struct exFAT_EntryMeta2 meta2;
	const size_t name_length = utf16_length(name);
	const int name_entries = DIV_ROUND_UP(name_length, EXFAT_ENAME_MAX);
	struct DateStamp ds;
	int i;

	DateStamp(&ds);

	node = allocate_node(ef);
	if (node == NULL)
		return -ERROR_NO_FREE_STORE;
	node->en_EntryCluster = cluster;
	node->en_EntryOffset = offset;
	CopyMem((CONST APTR)name, node->en_Name, name_length * sizeof(le16_t));

	memset(&meta1, 0, sizeof(meta1));
	meta1.ee_Type = EXFAT_ENTRY_FILE;
	meta1.ee_Continuations = 1 + name_entries;
	meta1.ee_Attrib = cpu_to_le16(attrib);
	exfat_amiga2exfat(&ds, &meta1.ee_CrDate, &meta1.ee_CrTime, &meta1.ee_CrTimeCS);
	meta1.ee_ADate = meta1.ee_MDate = meta1.ee_CrDate;
	meta1.ee_ATime = meta1.ee_MTime = meta1.ee_CrTime;
	meta1.ee_MTimeCS = meta1.ee_CrTimeCS; /* there is no atime_cs */

	memset(&meta2, 0, sizeof(meta2));
	meta2.ee_Type = EXFAT_ENTRY_FILE_INFO;
	meta2.ee_Flags = EXFAT_FLAG_ALWAYS1;
	meta2.ee_NameLength = name_length;
	meta2.ee_NameHash = exfat_calc_name_hash(ef, node->en_Name);
	meta2.ee_StartCluster = cpu_to_le32(EXFAT_CLUSTER_FREE);

	meta1.ee_Checksum = exfat_calc_checksum(&meta1, &meta2, node->en_Name);

	exfat_pwrite(ef->ef_Dev, &meta1, sizeof(meta1), co2o(ef, cluster, offset));
	next_entry(ef, dir, &cluster, &offset);
	exfat_pwrite(ef->ef_Dev, &meta2, sizeof(meta2), co2o(ef, cluster, offset));
	for (i = 0; i < name_entries; i++)
	{
		struct exFAT_EntryName name_entry = {EXFAT_ENTRY_FILE_NAME, 0};
		CopyMem(node->en_Name + i * EXFAT_ENAME_MAX, name_entry.ee_Name, EXFAT_ENAME_MAX * sizeof(le16_t));
		next_entry(ef, dir, &cluster, &offset);
		exfat_pwrite(ef->ef_Dev, &name_entry, sizeof(name_entry), co2o(ef, cluster, offset));
	}

	init_node_meta1(node, &meta1);
	init_node_meta2(node, &meta2);

	tree_attach(dir, node);
	exfat_update_mtime(dir);
	return 0;
}

static int create(struct exFAT* ef, const char* path, UWORD attrib)
{
	struct exFAT_Node* dir;
	struct exFAT_Node* existing;
	ULONG cluster = EXFAT_CLUSTER_BAD;
	UQUAD offset = -1;
	le16_t name[EXFAT_NAME_MAX + 1];
	int rc;

	rc = exfat_split(ef, &dir, &existing, name, path);
	if (rc != 0)
		return rc;
	if (existing != NULL)
	{
		exfat_put_node(ef, existing);
		exfat_put_node(ef, dir);
		return -ERROR_OBJECT_EXISTS;//EEXIST;
	}

	rc = find_slot(ef, dir, &cluster, &offset, 2 + DIV_ROUND_UP(utf16_length(name), EXFAT_ENAME_MAX));
	if (rc != 0)
	{
		exfat_put_node(ef, dir);
		return rc;
	}
	rc = write_entry(ef, dir, name, cluster, offset, attrib);
	exfat_put_node(ef, dir);
	return rc;
}

int exfat_mknod(struct exFAT* ef, const char* path)
{
	return create(ef, path, EXFAT_ATTRIB_ARCH);
}

int exfat_mkdir(struct exFAT* ef, const char* path)
{
	int rc;
	struct exFAT_Node* node;

	rc = create(ef, path, EXFAT_ATTRIB_ARCH | EXFAT_ATTRIB_DIR);
	if (rc != 0)
		return rc;
	rc = exfat_lookup(ef, &node, path);
	if (rc != 0)
		return 0;
	/* directories always have at least one cluster */
	rc = exfat_truncate(ef, node, CLUSTER_SIZE(*ef->ef_SB));
	if (rc != 0)
	{
		delete(ef, node);
		exfat_put_node(ef, node);
		return rc;
	}
	exfat_put_node(ef, node);
	return 0;
}

/*------------------------------------------------------------------------*/

static void rename_entry(struct exFAT* ef, struct exFAT_Node* dir,
		struct exFAT_Node* node, const le16_t* name, ULONG new_cluster,
		UQUAD new_offset)
{
	struct exFAT_EntryMeta1 meta1;
	struct exFAT_EntryMeta2 meta2;
	ULONG old_cluster = node->en_EntryCluster;
	UQUAD old_offset = node->en_EntryOffset;
	const size_t name_length = utf16_length(name);
	const int name_entries = DIV_ROUND_UP(name_length, EXFAT_ENAME_MAX);
	int i;

	exfat_pread(ef->ef_Dev, &meta1, sizeof(meta1), co2o(ef, old_cluster, old_offset));
	next_entry(ef, node->en_Parent, &old_cluster, &old_offset);
	exfat_pread(ef->ef_Dev, &meta2, sizeof(meta2), co2o(ef, old_cluster, old_offset));
	meta1.ee_Continuations = 1 + name_entries;
	meta2.ee_NameHash = exfat_calc_name_hash(ef, name);
	meta2.ee_NameLength = name_length;
	meta1.ee_Checksum = exfat_calc_checksum(&meta1, &meta2, name);

	erase_entry(ef, node);

	node->en_EntryCluster = new_cluster;
	node->en_EntryOffset = new_offset;

	exfat_pwrite(ef->ef_Dev, &meta1, sizeof(meta1), co2o(ef, new_cluster, new_offset));
	next_entry(ef, dir, &new_cluster, &new_offset);
	exfat_pwrite(ef->ef_Dev, &meta2, sizeof(meta2), co2o(ef, new_cluster, new_offset));

	for (i = 0; i < name_entries; i++)
	{
		struct exFAT_EntryName name_entry = {EXFAT_ENTRY_FILE_NAME, 0};
		CopyMem((CONST APTR)(name + i * EXFAT_ENAME_MAX), name_entry.ee_Name, EXFAT_ENAME_MAX * sizeof(le16_t));
		next_entry(ef, dir, &new_cluster, &new_offset);
		exfat_pwrite(ef->ef_Dev, &name_entry, sizeof(name_entry), co2o(ef, new_cluster, new_offset));
	}

	CopyMem((CONST APTR)name, node->en_Name, (EXFAT_NAME_MAX + 1) * sizeof(le16_t));
	tree_detach(node);
	tree_attach(dir, node);
}

int exfat_rename(struct exFAT* ef, const char* old_path, const char* new_path)
{
	struct exFAT_Node* node;
	struct exFAT_Node* existing;
	struct exFAT_Node* dir;
	ULONG cluster = EXFAT_CLUSTER_BAD;
	UQUAD offset = -1;
	le16_t name[EXFAT_NAME_MAX + 1];
	int rc;

	rc = exfat_lookup(ef, &node, old_path);
	if (rc != 0)
		return rc;

	rc = exfat_split(ef, &dir, &existing, name, new_path);
	if (rc != 0)
	{
		exfat_put_node(ef, node);
		return rc;
	}

	/* check that target is not a subdirectory of the source */
	if (node->en_Flags & EXFAT_ATTRIB_DIR)
	{
		struct exFAT_Node* p;

		for (p = dir; p; p = p->en_Parent)
			if (node == p)
			{
				if (existing != NULL)
					exfat_put_node(ef, existing);
				exfat_put_node(ef, dir);
				exfat_put_node(ef, node);
				return -ERROR_OBJECT_EXISTS; //EINVAL;
			}
	}

	if (existing != NULL)
	{
		/* remove target if it's not the same node as source */
		if (existing != node)
		{
			if (existing->en_Flags & EXFAT_ATTRIB_DIR)
			{
				if (node->en_Flags & EXFAT_ATTRIB_DIR)
					rc = exfat_rmdir(ef, existing);
				else
					rc = -ERROR_OBJECT_WRONG_TYPE;//ENOTDIR;
			}
			else
			{
				if (!(node->en_Flags & EXFAT_ATTRIB_DIR))
					rc = exfat_unlink(ef, existing);
				else
					rc = -ERROR_OBJECT_WRONG_TYPE;//EISDIR;
			}
			exfat_put_node(ef, existing);
			if (rc != 0)
			{
				exfat_put_node(ef, dir);
				exfat_put_node(ef, node);
				return rc;
			}
		}
		else
			exfat_put_node(ef, existing);
	}

	rc = find_slot(ef, dir, &cluster, &offset, 2 + DIV_ROUND_UP(utf16_length(name), EXFAT_ENAME_MAX));
	if (rc != 0)
	{
		exfat_put_node(ef, dir);
		exfat_put_node(ef, node);
		return rc;
	}
	rename_entry(ef, dir, node, name, cluster, offset);
	exfat_put_node(ef, dir);
	exfat_put_node(ef, node);
	return 0;
}

/*------------------------------------------------------------------------*/

/*void exfat_utimes(struct exFAT_Node* node, const struct timespec tv[2])
{
	node->en_ATime = tv[0].tv_sec;
	node->en_MTime = tv[1].tv_sec;
	node->en_Flags |= EXFAT_ATTRIB_DIRTY;
}*/

void exfat_update_atime(struct exFAT_Node* node)
{
	DateStamp(&node->en_ATime);
	node->en_Flags |= EXFAT_ATTRIB_DIRTY;
}

void exfat_update_mtime(struct exFAT_Node* node)
{
	DateStamp(&node->en_MTime);
	node->en_Flags |= EXFAT_ATTRIB_DIRTY;
}

void exfat_set_mtime(struct exFAT_Node* node, struct DateStamp *ds)
{
	exfat_copy_ds(ds, &node->en_MTime);
	node->en_Flags |= EXFAT_ATTRIB_DIRTY;
}

void exfat_set_flags(struct exFAT_Node* node, int flags)
{
	node->en_Flags = flags;
	node->en_Flags |= EXFAT_ATTRIB_DIRTY;
}

/*------------------------------------------------------------------------*/

const char* exfat_get_label(struct exFAT* ef)
{
	return ef->ef_Label;
}

/*------------------------------------------------------------------------*/

static int find_label(struct exFAT* ef, ULONG* cluster, UQUAD* offset)
{
	struct iterator it;
	int rc;

	rc = opendir(ef, ef->ef_Root, &it);
	if (rc != 0)
		return rc;

	for (;;)
	{
		if (it.offset >= ef->ef_Root->en_Size)
		{
			closedir(ef, &it);
			return -ERROR_NO_MORE_ENTRIES;
		}

		if (get_entry_ptr(ef, &it)->ee_Type == EXFAT_ENTRY_LABEL)
		{
			*cluster = it.cluster;
			*offset = it.offset;
			closedir(ef, &it);
			return 0;
		}

		if (fetch_next_entry(ef, ef->ef_Root, &it) != 0)
		{
			closedir(ef, &it);
			return -ERROR_OBJECT_NOT_FOUND;
		}
	}
}

int exfat_set_label(struct exFAT* ef, const char* label)
{
	le16_t label_utf16[EXFAT_ENAME_MAX + 1];
	int rc;
	ULONG cluster;
	UQUAD offset;
	struct exFAT_EntryLabel entry;

	memset(label_utf16, 0, sizeof(label_utf16));
	rc = utf8_to_utf16(label_utf16, label, EXFAT_ENAME_MAX, strlen(label));
	if (rc != 0)
		return rc;

	rc = find_label(ef, &cluster, &offset);
	if (rc == -ERROR_NO_MORE_ENTRIES)
		rc = find_slot(ef, ef->ef_Root, &cluster, &offset, 1);
	if (rc != 0)
		return rc;

	entry.ee_Type = EXFAT_ENTRY_LABEL;
	entry.ee_Length = utf16_length(label_utf16);
	CopyMem(label_utf16, entry.ee_Name, sizeof(entry.ee_Name));
	if (entry.ee_Length == 0)
		entry.ee_Type ^= EXFAT_ENTRY_VALID;

	exfat_pwrite(ef->ef_Dev, &entry, sizeof(struct exFAT_EntryLabel), co2o(ef, cluster, offset));
	return 0;
}

