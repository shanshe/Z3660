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

/*------------------------------------------------------------------------*/

inline UWORD le16_to_cpu(le16_t v) { return SWAPWORD(v); }
inline ULONG le32_to_cpu(le32_t v) { return SWAPLONG(v); }
inline UQUAD le64_to_cpu(le64_t v) { return SWAPQUAD(v); }

inline le16_t cpu_to_le16(UWORD v) { return SWAPWORD(v); }
inline le32_t cpu_to_le32(ULONG v) { return SWAPLONG(v); }
inline le64_t cpu_to_le64(UQUAD v) { return SWAPQUAD(v); }


/*------------------------------------------------------------------------*/

/*void exfat_stat(const struct exFAT* ef, const struct exFAT_Node* node, struct stat* stbuf)
{
	memset(stbuf, 0, sizeof(struct stat));
	if (node->flags & EXFAT_ATTRIB_DIR)
		stbuf->st_mode = S_IFDIR | (0777 & ~ef->dmask);
	else
		stbuf->st_mode = S_IFREG | (0777 & ~ef->fmask);
	stbuf->st_nlink = 1;
	stbuf->st_uid = ef->uid;
	stbuf->st_gid = ef->gid;
	stbuf->st_size = node->en_Size;
	stbuf->st_blocks = DIV_ROUND_UP(node->en_Size, CLUSTER_SIZE(*ef->ef_SB)) * CLUSTER_SIZE(*ef->ef_SB) / 512;
	stbuf->st_mtime = node->mtime;
	stbuf->st_atime = node->atime;
	//set ctime to mtime to ensure we don't break programs that rely on ctime (e.g. rsync)
	stbuf->st_ctime = node->mtime;
}*/

/*------------------------------------------------------------------------*/

void exfat_fib(const struct exFAT *ef, const struct exFAT_Node *node, struct FileInfoBlock *fib)
{
	memset(fib, 0, sizeof(struct FileInfoBlock));

	if (node == ef->ef_Root) {
		fib->fib_DiskKey = (ULONG)node->en_Child;
		fib->fib_DirEntryType = fib->fib_EntryType = ST_ROOT;
	}
	else if (node->en_Flags & EXFAT_ATTRIB_DIR) {
		fib->fib_DiskKey = (ULONG)node->en_Child;
		fib->fib_DirEntryType = fib->fib_EntryType = ST_USERDIR;
	} else {
		fib->fib_DiskKey = (ULONG)node->en_Next;
		fib->fib_DirEntryType = fib->fib_EntryType = ST_FILE;
	}

	if (fib->fib_DirEntryType != ST_ROOT) {
		char name[EXFAT_NAME_MAX + 1];

		exfat_get_name(node, name, EXFAT_NAME_MAX);
		if (*name) {
			char iso[EXFAT_NAME_MAX + 1];

			if (utf8_to_iso8859(iso, (const char *)name, strlen(name)) == 0) {
				cstr2bstr_buf(iso, (char *)fib->fib_FileName, AMIGA_NAME_MAX-1);

				if (strlen(iso) >= AMIGA_NAME_MAX)
					exfat_warn("filename '%s' too long", iso);
			} else
				exfat_fatal("failed to convert name from UTF-8");
		}

		if (node->en_Flags & EXFAT_ATTRIB_RO) fib->fib_Protection |= (FIBF_DELETE | FIBF_WRITE);
		if (node->en_Flags & EXFAT_ATTRIB_ARCH) fib->fib_Protection |= FIBF_ARCHIVE;

		exfat_copy_ds((struct DateStamp *)&node->en_MTime, &fib->fib_Date); /* modification time */
		/* exfat_copy_ds((struct DateStamp *)&node->en_ATime, &ds); access time - not supported by AmigaOS/MorphOS */

		/* not supported by exFAT
		fib->fib_Comment[0] = '\0';
		fib->fib_OwnerUID = 0;
		fib->fib_OwnerGID = 0; */

		if (fib->fib_DirEntryType == ST_FILE) {
			if (node->en_Size > FILESIZE_LIMIT_OS3) {
#ifdef __MORPHOS__
				fib->fib_Size = FILESIZE_LIMIT_OS3;
				fib->fib_NumBlocks = (LONG)(ROUND_UP(FILESIZE_LIMIT_OS3, CLUSTER_SIZE(*ef->ef_SB)) / SECTOR_SIZE(*ef->ef_SB));
#else
				/* mark file as link with no size and rigths */
				fib->fib_DirEntryType = fib->fib_EntryType = ST_LINKFILE;
				fib->fib_Size = 0l;
				fib->fib_NumBlocks = 0l;
				fib->fib_Protection = FIBF_DELETE | FIBF_EXECUTE | FIBF_WRITE | FIBF_READ | FIBF_ARCHIVE;
#endif
			} else {
				fib->fib_Size = (LONG)node->en_Size;
				fib->fib_NumBlocks = (LONG)(ROUND_UP(node->en_Size, CLUSTER_SIZE(*ef->ef_SB)) / SECTOR_SIZE(*ef->ef_SB));
			}
		}
	}
}

#ifdef __MORPHOS__
void exfat_fib64(const struct exFAT *ef, const struct exFAT_Node *node, struct FileInfoBlock *fib)
{
	exfat_fib(ef, node, fib);

	if (fib->fib_DirEntryType == ST_FILE) {
		fib->fib_Size64 = node->en_Size;
		fib->fib_NumBlocks64 = (UQUAD)(ROUND_UP(node->en_Size, CLUSTER_SIZE(*ef->ef_SB)) / SECTOR_SIZE(*ef->ef_SB));
	}
}
#endif

/*------------------------------------------------------------------------*/

void exfat_get_name(const struct exFAT_Node* node, char* buffer, size_t n)
{
	if (utf16_to_utf8(buffer, node->en_Name, n, EXFAT_NAME_MAX) != 0)
		exfat_fatal("failed to convert name to UTF-8");
}

/*------------------------------------------------------------------------*/

UWORD exfat_start_checksum(const struct exFAT_EntryMeta1* entry)
{
	UWORD sum = 0;
	int i;

	for (i = 0; i < sizeof(struct exFAT_Entry); i++)
		if (i != 2 && i != 3) /* skip checksum field itself */
			sum = ((sum << 15) | (sum >> 1)) + ((const UBYTE*) entry)[i];
	return sum;
}

UWORD exfat_add_checksum(const void* entry, UWORD sum)
{
	int i;

	for (i = 0; i < sizeof(struct exFAT_Entry); i++)
		sum = ((sum << 15) | (sum >> 1)) + ((const UBYTE*) entry)[i];
	return sum;
}

le16_t exfat_calc_checksum(const struct exFAT_EntryMeta1* meta1, const struct exFAT_EntryMeta2* meta2, const le16_t* name)
{
	UWORD checksum;
	const int name_entries = DIV_ROUND_UP(utf16_length(name), EXFAT_ENAME_MAX);
	int i;

	checksum = exfat_start_checksum(meta1);
	checksum = exfat_add_checksum(meta2, checksum);
	for (i = 0; i < name_entries; i++) {
		struct exFAT_EntryName name_entry = {EXFAT_ENTRY_FILE_NAME, 0};
		CopyMem((CONST APTR)(name + i * EXFAT_ENAME_MAX), name_entry.ee_Name, EXFAT_ENAME_MAX * sizeof(le16_t));
		checksum = exfat_add_checksum(&name_entry, checksum);
	}
	return cpu_to_le16(checksum);
}

ULONG exfat_vbr_start_checksum(const void* sector, size_t size)
{
	size_t i;
	ULONG sum = 0;

	for (i = 0; i < size; i++)
		/* skip volume_state and allocated_percent fields */
		if (i != 0x6a && i != 0x6b && i != 0x70)
			sum = ((sum << 31) | (sum >> 1)) + ((const UBYTE*) sector)[i];
	return sum;
}

ULONG exfat_vbr_add_checksum(const void* sector, size_t size, ULONG sum)
{
	size_t i;

	for (i = 0; i < size; i++)
		sum = ((sum << 31) | (sum >> 1)) + ((const UBYTE*) sector)[i];
	return sum;
}

/*------------------------------------------------------------------------*/

le16_t exfat_calc_name_hash(const struct exFAT* ef, const le16_t* name)
{
	size_t i;
	size_t length = utf16_length(name);
	UWORD hash = 0;

	for (i = 0; i < length; i++) {
		UWORD c = le16_to_cpu(name[i]);

		/* convert to upper case */
		if (c < ef->ef_UpCaseChars)
			c = le16_to_cpu(ef->ef_UpCase[c]);

		hash = ((hash << 15) | (hash >> 1)) + (c & 0xff);
		hash = ((hash << 15) | (hash >> 1)) + (c >> 8);
	}
	return cpu_to_le16(hash);
}

/*------------------------------------------------------------------------*/

void exfat_humanize_bytes(UQUAD value, struct exFAT_HumanBytes *eh)
{
	size_t i;
	const char *units[] = {"bytes", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB"};
	UQUAD divisor = 1;
	UQUAD temp = 0;

	for (i = 0; ; i++, divisor *= 1024) {
		temp = (value + divisor / 2) / divisor;

		if (temp == 0)
			break;
		if (temp / 1024 * 1024 == temp)
			continue;
		if (temp < 10240)
			break;
	}
	eh->eh_Value = temp;
	eh->eh_Unit = units[i];
}

void exfat_print_info(const struct exFAT_Super* sb, ULONG free_clusters)
{
	struct exFAT_HumanBytes eh;
	UQUAD total_space = le64_to_cpu(sb->es_SectorCount) * SECTOR_SIZE(*sb);
	UQUAD avail_space = (UQUAD) free_clusters * CLUSTER_SIZE(*sb);

	exfat_debug("File system version           %u.%u", sb->es_Version.es_Major, sb->es_Version.es_Minor);

	exfat_humanize_bytes(SECTOR_SIZE(*sb), &eh);
	exfat_debug("Sector size          %10llu %s", eh.eh_Value, eh.eh_Unit);

	exfat_humanize_bytes(CLUSTER_SIZE(*sb), &eh);
	exfat_debug("Cluster size         %10llu %s", eh.eh_Value, eh.eh_Unit);

	exfat_humanize_bytes(total_space, &eh);
	exfat_debug("Volume size          %10llu %s", eh.eh_Value, eh.eh_Unit);

	exfat_humanize_bytes(total_space - avail_space, &eh);
	exfat_debug("Used space           %10llu %s", eh.eh_Value, eh.eh_Unit);

	exfat_humanize_bytes(avail_space, &eh);
	exfat_debug("Available space      %10llu %s", eh.eh_Value, eh.eh_Unit);
}

/*------------------------------------------------------------------------*/

APTR exfat_alloc(struct exFAT* ef, ULONG size, BOOL clear)
{
	if (size) {
		APTR data;

		if ((data = AllocVecPooled(ef->ef_Pool, size))) {
			ef->ef_PoolUsage += (LONG)size;
			if (clear)
				memset(data, 0, size);
		}
		return data;
	}
	return NULL;
}

void exfat_free(struct exFAT* ef, APTR data)
{
	if (data) {
		ULONG size = *(((ULONG *)data) - 1) - sizeof(ULONG);

		ef->ef_PoolUsage -= (LONG)size;
		FreeVecPooled(ef->ef_Pool, data);
	}
}

