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

static UQUAD s2o(const struct exFAT* ef, UQUAD sector) {
	return sector << ef->ef_SB->es_SectorBits;
}

static UQUAD c2s(const struct exFAT* ef, ULONG cluster) {
	if (cluster < EXFAT_FIRST_DATA_CLUSTER)
		exfat_fatal("invalid cluster number %lu", cluster);

	return le32_to_cpu(ef->ef_SB->es_ClusterSectorStart) + ((UQUAD)(cluster - EXFAT_FIRST_DATA_CLUSTER) << ef->ef_SB->es_SpcBits);
}

UQUAD exfat_c2o(const struct exFAT* ef, ULONG cluster) {
	return s2o(ef, c2s(ef, cluster));
}

static ULONG s2c(const struct exFAT* ef, UQUAD sector) {
	return ((sector - le32_to_cpu(ef->ef_SB->es_ClusterSectorStart)) >> ef->ef_SB->es_SpcBits) + EXFAT_FIRST_DATA_CLUSTER;
}

static ULONG bytes2clusters(const struct exFAT* ef, UQUAD bytes) {
	UQUAD cluster_size = CLUSTER_SIZE(*ef->ef_SB);
	return (bytes + cluster_size - 1) / cluster_size;
}

/*------------------------------------------------------------------------*/

ULONG exfat_next_cluster(const struct exFAT* ef, const struct exFAT_Node* node, ULONG cluster)
{
	le32_t next;
	UQUAD fat_offset;

	if (cluster < EXFAT_FIRST_DATA_CLUSTER)
		exfat_fatal("bad cluster 0x%lx", cluster);

	if (IS_CONTIGUOUS(*node))
		return cluster + 1;

	fat_offset = s2o(ef, le32_to_cpu(ef->ef_SB->es_FatSectorStart)) + cluster * sizeof(ULONG);
	exfat_pread(ef->ef_Dev, &next, sizeof(next), fat_offset);
	return le32_to_cpu(next);
}

ULONG exfat_advance_cluster(const struct exFAT* ef, struct exFAT_Node* node, ULONG count)
{
	ULONG i;

	if (node->en_FPtrIndex > count)
	{
		node->en_FPtrIndex = 0;
		node->en_FPtrCluster = node->en_StartCluster;
	}

	for (i = node->en_FPtrIndex; i < count; i++)
	{
		node->en_FPtrCluster = exfat_next_cluster(ef, node, node->en_FPtrCluster);
		if (CLUSTER_INVALID(node->en_FPtrCluster))
			break; /* the caller should handle this and print appropriate error message */
	}
	node->en_FPtrIndex = count;
	return node->en_FPtrCluster;
}

/*------------------------------------------------------------------------*/

static ULONG find_bit_and_set(UBYTE* bitmap, size_t start, size_t end)
{
	const size_t start_index = start / 8;
	const size_t end_index = DIV_ROUND_UP(end, 8);
	size_t i;
	size_t c;

	for (i = start_index; i < end_index; i++)
	{
		if (bitmap[i] == 0xff)
			continue;
		for (c = MAX(i * 8, start); c < MIN((i + 1) * 8, end); c++)
			if (BMAP_GET(bitmap, c) == 0)
			{
				BMAP_SET(bitmap, c);
				return c + EXFAT_FIRST_DATA_CLUSTER;
			}
	}
	return EXFAT_CLUSTER_END;
}

void exfat_flush_cmap(struct exFAT* ef)
{
	exfat_pwrite(ef->ef_Dev, ef->ef_CMap.chunk, (ef->ef_CMap.chunk_size + 7) / 8, exfat_c2o(ef, ef->ef_CMap.start_cluster));
	ef->ef_CMap.dirty = FALSE;
}

/*------------------------------------------------------------------------*/

static void set_next_cluster(const struct exFAT* ef, int contiguous, ULONG current, ULONG next)
{
	UQUAD fat_offset;
	le32_t next_le32;

	if (contiguous)
		return;

	fat_offset = s2o(ef, le32_to_cpu(ef->ef_SB->es_FatSectorStart)) + current * sizeof(ULONG);
	next_le32 = cpu_to_le32(next);
	exfat_pwrite(ef->ef_Dev, &next_le32, sizeof(next_le32), fat_offset);
}

static ULONG allocate_cluster(struct exFAT* ef, ULONG hint)
{
	ULONG cluster;

	hint -= EXFAT_FIRST_DATA_CLUSTER;
	if (hint >= ef->ef_CMap.chunk_size)
		hint = 0;

	cluster = find_bit_and_set(ef->ef_CMap.chunk, hint, ef->ef_CMap.chunk_size);
	if (cluster == EXFAT_CLUSTER_END)
		cluster = find_bit_and_set(ef->ef_CMap.chunk, 0, hint);
	if (cluster == EXFAT_CLUSTER_END)
	{
		exfat_error("no free space left");
		return EXFAT_CLUSTER_END;
	}

	ef->ef_CMap.dirty = TRUE;
	return cluster;
}

static void free_cluster(struct exFAT* ef, ULONG cluster)
{
	if (CLUSTER_INVALID(cluster))
		exfat_fatal("freeing invalid cluster 0x%lx", cluster);
	if (cluster - EXFAT_FIRST_DATA_CLUSTER >= ef->ef_CMap.size)
		exfat_fatal("freeing non-existing cluster 0x%lx (0x%lx)", cluster, ef->ef_CMap.size);

	BMAP_CLR(ef->ef_CMap.chunk, cluster - EXFAT_FIRST_DATA_CLUSTER);
	ef->ef_CMap.dirty = TRUE;
}

static void make_noncontiguous(const struct exFAT* ef, ULONG first,
		ULONG last)
{
	ULONG c;

	for (c = first; c < last; c++)
		set_next_cluster(ef, 0, c, c + 1);
}

static int shrink_file(struct exFAT* ef, struct exFAT_Node* node, ULONG current, ULONG difference);

static int grow_file(struct exFAT* ef, struct exFAT_Node* node, ULONG current, ULONG difference)
{
	ULONG previous;
	ULONG next;
	ULONG allocated = 0;

	if (difference == 0)
		exfat_fatal("zero clusters count passed");

	if (node->en_StartCluster != EXFAT_CLUSTER_FREE)
	{
		/* get the last cluster of the file */
		previous = exfat_advance_cluster(ef, node, current - 1);
		if (CLUSTER_INVALID(previous))
		{
			exfat_error("invalid cluster 0x%lx while growing", previous);
			return -ERROR_NO_FREE_STORE; //EIO;
		}
	}
	else
	{
		if (node->en_FPtrIndex != 0)
			exfat_fatal("non-zero pointer index (%lu)", node->en_FPtrIndex);
		/* file does not have clusters (i.e. is empty), allocate
		the first one for it */
		previous = allocate_cluster(ef, 0);
		if (CLUSTER_INVALID(previous))
			return -ERROR_DISK_FULL; //ENOSPC;
		node->en_FPtrCluster = node->en_StartCluster = previous;
		allocated = 1;
		/* file consists of only one cluster, so it's contiguous */
		node->en_Flags |= EXFAT_ATTRIB_CONTIGUOUS;
	}

	while (allocated < difference)
	{
		next = allocate_cluster(ef, previous + 1);
		if (CLUSTER_INVALID(next))
		{
			if (allocated != 0)
				shrink_file(ef, node, current + allocated, allocated);
			return -ERROR_DISK_FULL; //ENOSPC;
		}
		if (next != previous - 1 && IS_CONTIGUOUS(*node))
		{
			/* it's a pity, but we are not able to keep the file contiguous
			anymore */
			make_noncontiguous(ef, node->en_StartCluster, previous);
			node->en_Flags &= ~EXFAT_ATTRIB_CONTIGUOUS;
			node->en_Flags |= EXFAT_ATTRIB_DIRTY;
		}
		set_next_cluster(ef, IS_CONTIGUOUS(*node), previous, next);
		previous = next;
		allocated++;
	}

	set_next_cluster(ef, IS_CONTIGUOUS(*node), previous, EXFAT_CLUSTER_END);
	return 0;
}

static int shrink_file(struct exFAT* ef, struct exFAT_Node* node, ULONG current, ULONG difference)
{
	ULONG previous;
	ULONG next;

	if (difference == 0)
		exfat_fatal("zero difference passed");
	if (node->en_StartCluster == EXFAT_CLUSTER_FREE)
		exfat_fatal("unable to shrink empty file (%lu clusters)", current);
	if (current < difference)
		exfat_fatal("file underflow (%lu < %lu)", current, difference);

	/* crop the file */
	if (current > difference)
	{
		ULONG last = exfat_advance_cluster(ef, node, current - difference - 1);
		if (CLUSTER_INVALID(last))
		{
			exfat_error("invalid cluster 0x%lx while shrinking", last);
			return -ERROR_NO_FREE_STORE; //EIO;
		}
		previous = exfat_next_cluster(ef, node, last);
		set_next_cluster(ef, IS_CONTIGUOUS(*node), last, EXFAT_CLUSTER_END);
	}
	else
	{
		previous = node->en_StartCluster;
		node->en_StartCluster = EXFAT_CLUSTER_FREE;
	}
	node->en_FPtrIndex = 0;
	node->en_FPtrCluster = node->en_StartCluster;

	/* free remaining clusters */
	while (difference--)
	{
		if (CLUSTER_INVALID(previous))
		{
			exfat_error("invalid cluster 0x%lx while freeing after shrink", previous);
			return -ERROR_NO_FREE_STORE; //EIO;
		}
		next = exfat_next_cluster(ef, node, previous);
		set_next_cluster(ef, IS_CONTIGUOUS(*node), previous, EXFAT_CLUSTER_FREE);
		free_cluster(ef, previous);
		previous = next;
	}
	return 0;
}

static void erase_raw(struct exFAT* ef, size_t size, UQUAD offset)
{
	exfat_pwrite(ef->ef_Dev, ef->ef_ZeroCluster, size, offset);
}

static int erase_range(struct exFAT* ef, struct exFAT_Node* node, UQUAD begin, UQUAD end)
{
	UQUAD cluster_boundary;
	ULONG cluster;

	if (begin >= end)
		return 0;

	cluster_boundary = (begin | (CLUSTER_SIZE(*ef->ef_SB) - 1)) + 1;
	cluster = exfat_advance_cluster(ef, node, begin / CLUSTER_SIZE(*ef->ef_SB));
	if (CLUSTER_INVALID(cluster))
	{
		exfat_error("invalid cluster 0x%lx while erasing", cluster);
		return -ERROR_NO_FREE_STORE;// EIO;
	}
	/* erase from the beginning to the closest cluster boundary */
	erase_raw(ef, MIN(cluster_boundary, end) - begin, exfat_c2o(ef, cluster) + begin % CLUSTER_SIZE(*ef->ef_SB));
	/* erase whole clusters */
	while (cluster_boundary < end)
	{
		cluster = exfat_next_cluster(ef, node, cluster);
		/* the cluster cannot be invalid because we have just allocated it */
		if (CLUSTER_INVALID(cluster))
			exfat_fatal("invalid cluster 0x%lx after allocation", cluster);
		erase_raw(ef, CLUSTER_SIZE(*ef->ef_SB), exfat_c2o(ef, cluster));
		cluster_boundary += CLUSTER_SIZE(*ef->ef_SB);
	}
	return 0;
}

int exfat_truncate(struct exFAT* ef, struct exFAT_Node* node, UQUAD size)
{
	ULONG c1 = bytes2clusters(ef, node->en_Size);
	ULONG c2 = bytes2clusters(ef, size);
	int rc = 0;

	if (node->en_References == 0 && node->en_Parent)
		exfat_fatal("no references, node changes can be lost");

	if (node->en_Size == size)
		return 0;

	if (c1 < c2)
		rc = grow_file(ef, node, c1, c2 - c1);
	else if (c1 > c2)
		rc = shrink_file(ef, node, c1, c1 - c2);

	if (rc != 0)
		return rc;

	rc = erase_range(ef, node, node->en_Size, size);
	if (rc != 0)
		return rc;

	exfat_update_mtime(node);
	node->en_Size = size;
	node->en_Flags |= EXFAT_ATTRIB_DIRTY;
	return 0;
}

/*------------------------------------------------------------------------*/

ULONG exfat_count_free_clusters(const struct exFAT* ef)
{
	ULONG free_clusters = 0;
	ULONG i;

	for (i = 0; i < ef->ef_CMap.size; i++)
		if (BMAP_GET(ef->ef_CMap.chunk, i) == 0)
			free_clusters++;
	return free_clusters;
}

/*------------------------------------------------------------------------*/

static int find_used_clusters(const struct exFAT* ef, ULONG* a, ULONG* b)
{
	const ULONG end = le32_to_cpu(ef->ef_SB->es_ClusterCount);

	/* find first used cluster */
	for (*a = *b + 1; *a < end; (*a)++)
		if (BMAP_GET(ef->ef_CMap.chunk, *a - EXFAT_FIRST_DATA_CLUSTER))
			break;
	if (*a >= end)
		return 1;

	/* find last contiguous used cluster */
	for (*b = *a; *b < end; (*b)++)
		if (BMAP_GET(ef->ef_CMap.chunk, *b - EXFAT_FIRST_DATA_CLUSTER) == 0)
		{
			(*b)--;
			break;
		}

	return 0;
}

int exfat_find_used_sectors(const struct exFAT* ef, UQUAD* a, UQUAD* b)
{
	ULONG ca, cb;

	if (*a == 0 && *b == 0)
		ca = cb = EXFAT_FIRST_DATA_CLUSTER - 1;
	else
	{
		ca = s2c(ef, *a);
		cb = s2c(ef, *b);
	}
	if (find_used_clusters(ef, &ca, &cb) != 0)
		return 1;
	if (*a != 0 || *b != 0)
		*a = c2s(ef, ca);
	*b = c2s(ef, cb) + (CLUSTER_SIZE(*ef->ef_SB) - 1) / SECTOR_SIZE(*ef->ef_SB);
	return 0;
}

