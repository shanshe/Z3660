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

#ifndef _EXFAT_H
#define _EXFAT_H

#include "ef_exfatfs.h"

#define EXFAT_VERSION_MAJOR 1
#define EXFAT_VERSION_MINOR 0
#define EXFAT_VERSION_PATCH 1

#define EXFAT_NAME_MAX 256
#define EXFAT_ATTRIB_CONTIGUOUS 0x10000
#define EXFAT_ATTRIB_CACHED     0x20000
#define EXFAT_ATTRIB_DIRTY      0x40000
#define EXFAT_ATTRIB_UNLINKED   0x80000

#define IS_CONTIGUOUS(node) (((node).en_Flags & EXFAT_ATTRIB_CONTIGUOUS) != 0)
#define SECTOR_SIZE(sb) (1 << (sb).es_SectorBits)
#define CLUSTER_SIZE(sb) (SECTOR_SIZE(sb) << (sb).es_SpcBits)
#define CLUSTER_INVALID(c) ((c) < EXFAT_FIRST_DATA_CLUSTER || (c) > EXFAT_LAST_DATA_CLUSTER)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define DIV_ROUND_UP(x, d) (((x) + (d) - 1) / (d))
#define ROUND_UP(x, d) (DIV_ROUND_UP(x, d) * (d))

#define BMAP_GET(bitmap, index) (((UBYTE *)bitmap)[(index) / 8] & (1u << ((index) % 8)))
#define BMAP_SET(bitmap, index) ((UBYTE *)bitmap)[(index) / 8] |= (1u << ((index) % 8))
#define BMAP_CLR(bitmap, index) ((UBYTE *)bitmap)[(index) / 8] &= ~(1u << ((index) % 8))

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#ifdef __MORPHOS__
	#pragma pack(2)
#endif

struct exFAT_Cache
{
	UBYTE						 ec_Data[BLOCKSIZE];
	UQUAD						 ec_LBA;
	ULONG						 ec_Usage;
};

struct exFAT_Device
{
	struct exFAT_Cache	*ed_Cache;

	struct IOStdReq		*ed_IO;
	struct MsgPort			*ed_Port;

	UQUAD						 ed_First;
	UQUAD						 ed_Last;
	UQUAD						 ed_Size;

	UQUAD						 ed_Offset;
};

struct exFAT_Node
{
	struct exFAT_Node		*en_Parent;
	struct exFAT_Node		*en_Child;
	struct exFAT_Node		*en_Next;
	struct exFAT_Node		*en_Prev;

	int						 en_References;
	ULONG						 en_FPtrIndex;
	ULONG						 en_FPtrCluster;
	ULONG						 en_EntryCluster;
	UQUAD						 en_EntryOffset;
	ULONG 					 en_StartCluster;
	int						 en_Flags;
	UQUAD						 en_Size;
	struct DateStamp		 en_MTime;
	struct DateStamp		 en_ATime;
	le16_t					 en_Name[EXFAT_NAME_MAX + 1];
};

struct exFAT
{
	APTR						 ef_Pool;
	LONG						 ef_PoolUsage;
	struct exFAT_Device	*ef_Dev;
	struct exFAT_Super	*ef_SB;
	le16_t					*ef_UpCase;
	size_t					 ef_UpCaseChars;
	struct exFAT_Node		*ef_Root;
	struct {
		ULONG start_cluster;
		ULONG size;			/* in bits */
		UBYTE *chunk;
		ULONG chunk_size;	/* in bits */
		BOOL dirty;
	}							 ef_CMap;
	char						 ef_Label[EXFAT_ENAME_MAX*6 + 1]; /* a character can occupy up to 6 bytes in UTF-8 */
	void						*ef_ZeroCluster;
	int						 ef_DMask;
	int						 ef_FMask;
	UWORD						 ef_UID;
	UWORD						 ef_GID;
	BOOL						 ef_ReadOnly;
	BOOL						 ef_NoAtime;
};

struct exFAT_Iterator
{
	struct exFAT_Node 	*ei_Parent;
	struct exFAT_Node 	*ei_Current;
};

struct exFAT_HumanBytes
{
	UQUAD						 eh_Value;
	const char 				*eh_Unit;
};

#ifdef __MORPHOS__
	#pragma pack()
#endif

/* log */
extern int exfat_errors;
void exfat_fatal(const char *fmt, ...) __attribute__((format(printf, 1, 2))); 
void exfat_error(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void exfat_warn(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void exfat_debug(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

/* io */
void exfat_cacheflush(struct exFAT_Device *ed);
struct exFAT_Device *exfat_open(int *err);
int exfat_close(struct exFAT_Device *dev);
int exfat_sync(struct exFAT_Device* dev);
UQUAD exfat_seek(struct exFAT_Device* dev, UQUAD offset, int whence);
LONG exfat_read(struct exFAT_Device* dev, void* buffer, size_t size);
LONG exfat_write(struct exFAT_Device* dev, const void* buffer, size_t size);
LONG exfat_pread(struct exFAT_Device* dev, void* buffer, size_t size, UQUAD offset);
LONG exfat_pwrite(struct exFAT_Device* dev, const void* buffer, size_t size, UQUAD offset);
LONG exfat_generic_pread(const struct exFAT* ef, struct exFAT_Node* node, void* buffer, size_t size, UQUAD offset);
LONG exfat_generic_pwrite(struct exFAT* ef, struct exFAT_Node* node, const void* buffer, size_t size, UQUAD offset);

/* lookup */
int exfat_opendir(struct exFAT* ef, struct exFAT_Node* dir, struct exFAT_Iterator* it);
void exfat_closedir(struct exFAT* ef, struct exFAT_Iterator* it);
struct exFAT_Node* exfat_readdir(struct exFAT* ef, struct exFAT_Iterator* it);
int exfat_lookup(struct exFAT* ef, struct exFAT_Node** node, const char* path);
int exfat_lookup_cluster(struct exFAT* ef, struct exFAT_Node** node, ULONG cluster, ULONG index);
int exfat_split(struct exFAT* ef, struct exFAT_Node** parent, struct exFAT_Node** node, le16_t* name, const char* path);

/* cluster */
UQUAD exfat_c2o(const struct exFAT* ef, ULONG cluster);
ULONG exfat_next_cluster(const struct exFAT* ef, const struct exFAT_Node* node, ULONG cluster);
ULONG exfat_advance_cluster(const struct exFAT* ef, struct exFAT_Node* node, ULONG count);
void exfat_flush_cmap(struct exFAT* ef);
int exfat_truncate(struct exFAT* ef, struct exFAT_Node* node, UQUAD size);
ULONG exfat_count_free_clusters(const struct exFAT* ef);
int exfat_find_used_sectors(const struct exFAT* ef, UQUAD* a, UQUAD* b);

/* utils */
UWORD le16_to_cpu(le16_t v);
ULONG le32_to_cpu(le32_t v);
UQUAD le64_to_cpu(le64_t v);
le16_t cpu_to_le16(UWORD v);
le32_t cpu_to_le32(ULONG v);
le64_t cpu_to_le64(UQUAD v);
//void exfat_stat(const struct exFAT* ef, const struct exFAT_Node* node, struct stat* stbuf);
void exfat_fib(const struct exFAT *ef, const struct exFAT_Node *node, struct FileInfoBlock *fib);
#ifdef __MORPHOS__
void exfat_fib64(const struct exFAT *ef, const struct exFAT_Node *node, struct FileInfoBlock *fib);
#endif
void exfat_get_name(const struct exFAT_Node* node, char* buffer, size_t n);
UWORD exfat_start_checksum(const struct exFAT_EntryMeta1* entry);
UWORD exfat_add_checksum(const void* entry, UWORD sum);
le16_t exfat_calc_checksum(const struct exFAT_EntryMeta1* meta1, const struct exFAT_EntryMeta2* meta2, const le16_t* name);
ULONG exfat_vbr_start_checksum(const void* sector, size_t size);
ULONG exfat_vbr_add_checksum(const void* sector, size_t size, ULONG sum);
le16_t exfat_calc_name_hash(const struct exFAT* ef, const le16_t* name);
void exfat_humanize_bytes(UQUAD value, struct exFAT_HumanBytes *eh);
void exfat_print_info(const struct exFAT_Super* sb, ULONG free_clusters);
APTR exfat_alloc(struct exFAT* ef, ULONG size, BOOL clear);
void exfat_free(struct exFAT* ef, APTR data);

/* utf */
int utf16_to_utf8(char* output, const le16_t* input, size_t outsize, size_t insize);
int utf8_to_utf16(le16_t* output, const char* input, size_t outsize, size_t insize);
size_t utf16_length(const le16_t* str);
int utf8_to_iso8859(char *output, const char *input, size_t insize);
int iso8859_to_utf8(char *output, const char *input, size_t insize);

/* node */
struct exFAT_Node* exfat_get_node(struct exFAT_Node* node);
void exfat_put_node(struct exFAT* ef, struct exFAT_Node* node);
int exfat_cache_directory(struct exFAT* ef, struct exFAT_Node* dir);
void exfat_reset_cache(struct exFAT* ef);
void exfat_flush_node(struct exFAT* ef, struct exFAT_Node* node);
int exfat_unlink(struct exFAT* ef, struct exFAT_Node* node);
int exfat_rmdir(struct exFAT* ef, struct exFAT_Node* node);
int exfat_mknod(struct exFAT* ef, const char* path);
int exfat_mkdir(struct exFAT* ef, const char* path);
int exfat_rename(struct exFAT* ef, const char* old_path, const char* new_path);
//void exfat_utimes(struct exFAT_Node* node, const struct timespec tv[2]);
void exfat_update_atime(struct exFAT_Node* node);
void exfat_update_mtime(struct exFAT_Node* node);
//void exfat_set_mtime(struct exFAT_Node* node, time_t time);
void exfat_set_mtime(struct exFAT_Node* node, struct DateStamp *ds);
void exfat_set_flags(struct exFAT_Node* node, int flags);
const char* exfat_get_label(struct exFAT* ef);
int exfat_set_label(struct exFAT* ef, const char* label);

/* mount */
int exfat_mount(struct exFAT *ef);
void exfat_unmount(struct exFAT *ef);

/* format */
int exfat_format(struct exFAT_Device* dev, int sector, int spc, const char* volume_label, ULONG volume_serial, UQUAD first_sector, UQUAD last_sector);

/* time */
extern long exfat_timezone;
void exfat_tzset(void);
void exfat_copy_ds(struct DateStamp *s, struct DateStamp *d);
void exfat_exfat2amiga(le16_t date, le16_t time, UBYTE centisec, struct DateStamp *ds);
void exfat_amiga2exfat(struct DateStamp *ds, le16_t *date, le16_t *time, UBYTE *centisec);

#endif /* _EXFAT_H */

