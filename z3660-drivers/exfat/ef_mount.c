/*--------------------------------------------------------------------------

  exFAT filesystem for MorphOS:
  Copyright � 2014-2015 Rupert Hausberger

  FAT12/16/32 filesystem handler:
  Copyright � 2006 Marek Szyprowski
  Copyright � 2007-2008 The AROS Development Team

  exFAT implementation library:
  Copyright � 2010-2013 Andrew Nayenko


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

#define IS_EXFAT(n) (n[0] == 'E' && n[1] == 'X' && n[2] == 'F' && n[3] == 'A' && n[4] == 'T' && n[5] == ' ' && n[6] == ' ' && n[7] == ' ')

/*------------------------------------------------------------------------*/

static UQUAD rootdir_size(const struct exFAT* ef)
{
	UQUAD clusters = 0;
	ULONG rootdir_cluster = le32_to_cpu(ef->ef_SB->es_RootdirCluster);

	while (!CLUSTER_INVALID(rootdir_cluster)) {
		clusters++;
		/* root directory cannot be contiguous because there is no flag to indicate this */
		rootdir_cluster = exfat_next_cluster(ef, ef->ef_Root, rootdir_cluster);
	}
	return clusters * CLUSTER_SIZE(*ef->ef_SB);
}

static int verify_vbr_checksum(struct exFAT_Device* dev, void* sector, UQUAD sector_size)
{
	ULONG vbr_checksum;
	int i;

	exfat_pread(dev, sector, sector_size, 0);
	vbr_checksum = exfat_vbr_start_checksum(sector, sector_size);
	for (i = 1; i < 11; i++) {
		exfat_pread(dev, sector, sector_size, i * sector_size);
		vbr_checksum = exfat_vbr_add_checksum(sector, sector_size, vbr_checksum);
	}
	exfat_pread(dev, sector, sector_size, i * sector_size);
	for (i = 0; i < sector_size / sizeof(vbr_checksum); i++) {
		if (le32_to_cpu(((const le32_t*) sector)[i]) != vbr_checksum) {
			exfat_error("invalid VBR checksum 0x%lx (expected 0x%lx)", le32_to_cpu(((const le32_t*) sector)[i]), vbr_checksum);
			return 1;
		}
	}
	return 0;
}

static int commit_super_block(const struct exFAT* ef)
{
	exfat_pwrite(ef->ef_Dev, ef->ef_SB, sizeof(struct exFAT_Super), 0);
	return exfat_sync(ef->ef_Dev);
}

static int prepare_super_block(const struct exFAT* ef)
{
	if (le16_to_cpu(ef->ef_SB->es_VolumeState) & EXFAT_STATE_MOUNTED)
		exfat_warn("volume was not unmounted cleanly");

	if (ef->ef_ReadOnly)
		return 0;

	ef->ef_SB->es_VolumeState = cpu_to_le16(le16_to_cpu(ef->ef_SB->es_VolumeState) | EXFAT_STATE_MOUNTED);
	return commit_super_block(ef);
}

int exfat_mount(struct exFAT *ef)
{
	int rc;

	exfat_tzset();

	memset(ef, 0, sizeof(struct exFAT));
#ifdef __READ_ONLY__
	ef->ef_ReadOnly = TRUE;
#endif
	ef->ef_NoAtime = TRUE;

	if (!(ef->ef_Dev = exfat_open(&rc))) {
		return rc;
	}
	if (!(ef->ef_Pool = CreatePool(MEMF_PUBLIC, 4096, 4096))) {
		exfat_error("failed to create memory pool");
		return -ERROR_NO_FREE_STORE;
	}
	if (!(ef->ef_SB = exfat_alloc(ef, sizeof(struct exFAT_Super), TRUE))) {
		exfat_close(ef->ef_Dev);
		DeletePool(ef->ef_Pool);
		exfat_error("failed to allocate memory for the super block");
		return -ERROR_NO_FREE_STORE;
	}
	exfat_pread(ef->ef_Dev, ef->ef_SB, sizeof(struct exFAT_Super), 0);
	if (!IS_EXFAT(ef->ef_SB->es_OemName)) {
		char message[100];
		sprintf(message,"exFAT file system is not found %s %p\n",ef->ef_SB->es_OemName,ef->ef_SB);
		Pop(LF, message);
		exfat_close(ef->ef_Dev);
		DeletePool(ef->ef_Pool);
		exfat_error("exFAT file system is not found");

		return -ERROR_OBJECT_WRONG_TYPE; //EIO;
	}
	if (ef->ef_SB->es_Version.es_Major != 1 || ef->ef_SB->es_Version.es_Minor != 0) {
		exfat_close(ef->ef_Dev);
		DeletePool(ef->ef_Pool);
		exfat_error("unsupported exFAT version: %hhu.%hhu", ef->ef_SB->es_Version.es_Major, ef->ef_SB->es_Version.es_Minor);
		char message[100];
		sprintf(message,"unsupported exFAT version: %hhu.%hhu", ef->ef_SB->es_Version.es_Major, ef->ef_SB->es_Version.es_Minor);
		Pop(LF, message);
		return -ERROR_OBJECT_WRONG_TYPE; //EIO;
	}
	if (ef->ef_SB->es_FatCount != 1) {
		exfat_close(ef->ef_Dev);
		DeletePool(ef->ef_Pool);
		exfat_error("unsupported FAT count: %hhu", ef->ef_SB->es_FatCount);
		char message[100];
		sprintf(message,"unsupported FAT count: %hhu", ef->ef_SB->es_FatCount);
		Pop(LF, message);
		return -ERROR_OBJECT_WRONG_TYPE; //EIO;
	}
	/* officially exFAT supports cluster size up to 32 MB */
	if ((int) ef->ef_SB->es_SectorBits + (int) ef->ef_SB->es_SpcBits > 25) {
		exfat_close(ef->ef_Dev);
		DeletePool(ef->ef_Pool);
		exfat_error("too big cluster size: 2^%d", (int) ef->ef_SB->es_SectorBits + (int) ef->ef_SB->es_SpcBits);
		char message[100];
		sprintf(message,"too big cluster size: 2^%d", (int) ef->ef_SB->es_SectorBits + (int) ef->ef_SB->es_SpcBits);
		Pop(LF, message);
		return -ERROR_OBJECT_WRONG_TYPE; //EIO;
	}
	if (!(ef->ef_ZeroCluster = exfat_alloc(ef, CLUSTER_SIZE(*ef->ef_SB), TRUE))) {
		exfat_close(ef->ef_Dev);
		DeletePool(ef->ef_Pool);
		exfat_error("failed to allocate zero sector");
		return -ERROR_NO_FREE_STORE;
	}
	/* use zero_cluster as a temporary buffer for VBR checksum verification */
	if (verify_vbr_checksum(ef->ef_Dev, ef->ef_ZeroCluster, SECTOR_SIZE(*ef->ef_SB)) != 0)
	{
		exfat_close(ef->ef_Dev);
		DeletePool(ef->ef_Pool);
		char message[100];
		sprintf(message,"verify_vbr_checksum");
		Pop(LF, message);
		return -ERROR_OBJECT_WRONG_TYPE; //EIO;
	}
	memset(ef->ef_ZeroCluster, 0, CLUSTER_SIZE(*ef->ef_SB));

	if (!(ef->ef_Root = exfat_alloc(ef, sizeof(struct exFAT_Node), TRUE))) {
		exfat_close(ef->ef_Dev);
		DeletePool(ef->ef_Pool);
		exfat_error("failed to allocate root node");
		return -ERROR_NO_FREE_STORE;
	}
	ef->ef_Root->en_Flags = EXFAT_ATTRIB_DIR;
	ef->ef_Root->en_StartCluster = le32_to_cpu(ef->ef_SB->es_RootdirCluster);
	ef->ef_Root->en_FPtrCluster = ef->ef_Root->en_StartCluster;
	ef->ef_Root->en_Name[0] = cpu_to_le16('\0');
	ef->ef_Root->en_Size = rootdir_size(ef);
	/* exFAT does not have time attributes for the root directory */
	//ef->ef_Root->en_MTime = 0;
	//ef->ef_Root->en_ATime = 0;
	/* always keep at least 1 reference to the root node */
	exfat_get_node(ef->ef_Root);

	rc = exfat_cache_directory(ef, ef->ef_Root);
	if (rc != 0)
		goto error;

	if (ef->ef_UpCase == NULL) {
		exfat_error("upcase table is not found");
		goto error;
	}
	if (ef->ef_CMap.chunk == NULL) {
		exfat_error("clusters bitmap is not found");
		goto error;
	}

	if (prepare_super_block(ef) != 0)
		goto error;

	return 0;

error:
	exfat_put_node(ef, ef->ef_Root);
	exfat_reset_cache(ef);
	exfat_close(ef->ef_Dev);
	DeletePool(ef->ef_Pool);
	return -ERROR_OBJECT_WRONG_TYPE; //EIO;
}

/*------------------------------------------------------------------------*/

static void finalize_super_block(struct exFAT* ef)
{
	if (ef->ef_ReadOnly)
		return;

	ef->ef_SB->es_VolumeState = cpu_to_le16(le16_to_cpu(ef->ef_SB->es_VolumeState) & ~EXFAT_STATE_MOUNTED);

	/* Some implementations set the percentage of allocated space to 0xff
	on FS creation and never update it. In this case leave it as is. */
	if (ef->ef_SB->es_AllocatedPercent != 0xff)
	{
		ULONG free, total;

		free = exfat_count_free_clusters(ef);
		total = le32_to_cpu(ef->ef_SB->es_ClusterCount);
		ef->ef_SB->es_AllocatedPercent = ((total - free) * 100 + total / 2) / total;
	}

	commit_super_block(ef);
}

void exfat_unmount(struct exFAT* ef)
{
	exfat_put_node(ef, ef->ef_Root);
	exfat_reset_cache(ef);
	exfat_free(ef, ef->ef_Root);
	ef->ef_Root = NULL;
	finalize_super_block(ef);
	exfat_close(ef->ef_Dev);
	ef->ef_Dev = NULL;
	exfat_free(ef, ef->ef_ZeroCluster);
	ef->ef_ZeroCluster = NULL;
	exfat_free(ef, ef->ef_CMap.chunk);
	ef->ef_CMap.chunk = NULL;
	exfat_free(ef, ef->ef_SB);
	ef->ef_SB = NULL;
	exfat_free(ef, ef->ef_UpCase);
	ef->ef_UpCase = NULL;
	ef->ef_UpCaseChars = 0;
	DeletePool(ef->ef_Pool);
}

