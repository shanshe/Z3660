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
#include <stdio.h>
/*------------------------------------------------------------------------*/

#define CACHE_SIZE 8

/*------------------------------------------------------------------------*/

struct exFAT_Device *exfat_open(int *err)
{
	struct DosEnvec *de = BADDR(global->g_FSSM->fssm_Environ);
	ULONG bs = GetBlockSize(de);
	UQUAD first = GetLowBlock(de); 
	UQUAD last = GetHighBlock(de); 
	struct exFAT_Device *dev;

	/*
	Log(LD, "BS %lu\n", bs);
	Log(LD, "Lo block %llu\n", first);
	Log(LD, "Hi block %llu\n", last);
	Log(LD, "Size %llu\n", last - first + 1);
	Log(LD, "Size %llu\n", (last - first + 1) << BLOCKSIZE_SHIFT);
	*/

	if (bs != BLOCKSIZE) {
		exfat_error("Invalid SectorSize of %lu, must be %d", bs, BLOCKSIZE);
		*err = -ERROR_BAD_NUMBER;
		return NULL;
	}
#ifndef __MORPHOS__
	if (last <= first) {
		char message[100];
		sprintf(message,"bs= %ld, last= %lld, first %lld\n", bs, last, first);
		exfat_error(message);
		Pop(LF, message);
		*err = -ERROR_OBJECT_TOO_LARGE;
		return NULL;
	}
	if (last - first + 1 > DISKSIZE_LIMIT_OS3) {
		char message[100];
		sprintf(message,"bs= %ld, last= %lld, first %lld\n", bs, last, first);
		exfat_error(message);
		Pop(LF, message);
		*err = -ERROR_OBJECT_TOO_LARGE;
		return NULL;
	}
#endif

	if ((dev = AllocVec(sizeof(struct exFAT_Device), MEMF_PUBLIC | MEMF_CLEAR)))
	{
		if ((dev->ed_Cache = AllocVec(sizeof(struct exFAT_Cache) * CACHE_SIZE, MEMF_PUBLIC | MEMF_CLEAR)))
		{
			dev->ed_First = first;
			dev->ed_Last = last;
			dev->ed_Size = (dev->ed_Last - dev->ed_First + 1) << BLOCKSIZE_SHIFT;
			dev->ed_Offset = 0ull;

			if ((dev->ed_Port = CreateMsgPort()))
			{
				if ((dev->ed_IO = CreateIORequest(dev->ed_Port, sizeof(struct IOStdReq))))
				{
					char device[64];
					ULONG unit = global->g_FSSM->fssm_Unit;
					ULONG flags = global->g_FSSM->fssm_Flags;

					bstr2cstr_buf(BADDR(global->g_FSSM->fssm_Device), device, sizeof(device));

					if (OpenDevice((CONST_STRPTR)device, unit, (struct IORequest *)dev->ed_IO, flags) == 0)
					{
						if (SCSI_Query(dev->ed_IO) == 0)
						{
							if (last < global->g_DiskSize)
								return dev;
							else {
								char message[100];
								sprintf(message,"Last block %llu out of disk at %s:%lu", last, device, unit);
								exfat_error(message);
								Pop(LF, message);
								*err = -ERROR_OBJECT_TOO_LARGE;							
							} 
						} else {
							char message[100];
							sprintf(message,"SCSI-query failed at %s:%lu", device, unit);
							exfat_error(message);
							Pop(LF, message);
						*err = -ERROR_DEVICE_NOT_MOUNTED;
						}
						CloseDevice((struct IORequest *)dev->ed_IO);
					} else {
						char message[100];
						sprintf(message,"Can't open %s:%lu", device, unit);
						exfat_error(message);
						*err = -ERROR_DEVICE_NOT_MOUNTED;
					}
					DeleteIORequest(dev->ed_IO);
				} else {
					char message[100];
					sprintf(message,"ERROR_NO_FREE_STORE 1");
					*err = -ERROR_NO_FREE_STORE;
				}
				DeleteMsgPort(dev->ed_Port);
			} else {
				char message[100];
				sprintf(message,"ERROR_NO_FREE_STORE 2");
				*err = -ERROR_NO_FREE_STORE;
			}
			FreeVec(dev->ed_Cache);
		} else {
			char message[100];
			sprintf(message,"ERROR_NO_FREE_STORE 3");
			*err = -ERROR_NO_FREE_STORE;
		}
		FreeVec(dev);
	} else {
		char message[100];
		sprintf(message,"ERROR_NO_FREE_STORE 4");
		*err = -ERROR_NO_FREE_STORE;
	}
	return NULL;
}

int exfat_close(struct exFAT_Device *dev)
{
	CloseDevice((struct IORequest *)dev->ed_IO);
	DeleteIORequest(dev->ed_IO);
	DeleteMsgPort(dev->ed_Port);
	FreeVec(dev->ed_Cache);
	FreeVec(dev);
	return 0;
}

/*------------------------------------------------------------------------*/

int exfat_sync(struct exFAT_Device *dev)
{
	return 0;
}

UQUAD exfat_seek(struct exFAT_Device *dev, UQUAD offset, int whence)
{
	switch (whence) {
		case SEEK_SET:
			dev->ed_Offset = offset;
			break;
		case SEEK_CUR:
			dev->ed_Offset += offset;
			break;
		case SEEK_END:
			dev->ed_Offset = dev->ed_Size + offset;
			break;
	}
	return dev->ed_Offset;
}

LONG exfat_read(struct exFAT_Device *dev, void *buffer, size_t size)
{
	LONG bytes = exfat_pread(dev, buffer, size, dev->ed_Offset);
	if (bytes != -1)
		dev->ed_Offset += (UQUAD)bytes;
	return bytes;
}

LONG exfat_write(struct exFAT_Device *dev, const void *buffer, size_t size)
{
	LONG bytes = exfat_pwrite(dev, buffer, size, dev->ed_Offset);
	if (bytes != -1)
		dev->ed_Offset += (UQUAD)bytes;
	return bytes;
}

/*------------------------------------------------------------------------*/

static struct exFAT_Cache *cache_find(struct exFAT_Device *ed, UQUAD lba)
{
	int i;

	for (i = 0; i < CACHE_SIZE; i++) {
		if (ed->ed_Cache[i].ec_LBA == lba)
			return &ed->ed_Cache[i];
	}
	return NULL;
}

static struct exFAT_Cache *cache_alloc(struct exFAT_Device *ed)
{
	ULONG m = 0xfffffffful;
	int i, j = 0;

	for (i = 0; i < CACHE_SIZE; i++) {
		if (ed->ed_Cache[i].ec_Usage < m) {
			m = ed->ed_Cache[i].ec_Usage;
			j = i;
		}
	}
	return &ed->ed_Cache[j];
}

static struct exFAT_Cache *cache_obtain(struct exFAT_Device *ed, UQUAD lba)
{
	struct exFAT_Cache *ec;

	if ((ec = cache_find(ed, lba)))
		ec->ec_Usage++;
	else {
		ec = cache_alloc(ed);

		if (SCSI_Read(ed->ed_IO, lba, 1, ec->ec_Data) != 0)
			return NULL;

		ec->ec_LBA = lba;
		ec->ec_Usage = 1;
	}
	return ec;
}

static LONG cache_read(struct exFAT_Device *ed, UQUAD lba, UWORD lbo, APTR data, ULONG bytes)
{
	struct exFAT_Cache *ec;

	if ((ec = cache_obtain(ed, lba))) {
		CopyMem(&ec->ec_Data[lbo], data, bytes);
		return 0;
	}
	return -1;
}

static LONG cache_write(struct exFAT_Device *ed, UQUAD lba, UWORD lbo, APTR data, ULONG bytes)
{
	struct exFAT_Cache *ec;

	if ((ec = cache_obtain(ed, lba))) {
		CopyMem(data, &ec->ec_Data[lbo], bytes);

		if (SCSI_Write(ed->ed_IO, lba, 1, ec->ec_Data) != 0) return -1;
		return 0;
	}
	return -1;
}

/*------------------------------------------------------------------------*/

LONG exfat_pread(struct exFAT_Device *dev, void *buffer, size_t size, UQUAD offset)
{
	UBYTE *ptr = (UBYTE *)buffer;
	ULONG rest = (ULONG)size, bytes;
	UQUAD lba;
	UWORD lbo;

	if (rest == 0)
		return -1;

	offset += (dev->ed_First << BLOCKSIZE_SHIFT);

	lba = offset >> BLOCKSIZE_SHIFT;
	lbo = (UWORD)(offset % BLOCKSIZE);

//	char message[100];
//	sprintf(message,"exfat_pread %p %lld %d %lld\n", buffer, lba, lbo, offset);
//	Pop(LF, message);

	if (lbo) {
		bytes = MIN(rest % BLOCKSIZE, BLOCKSIZE - lbo);
		if (cache_read(dev, lba, lbo, ptr, bytes) != 0)
			return -1;
		
		lba++;
		ptr += bytes;
		rest -= bytes;
	}
	if (rest >= BLOCKSIZE) {
		ULONG blocks = rest >> BLOCKSIZE_SHIFT;
		bytes = blocks << BLOCKSIZE_SHIFT;
		if (SCSI_Read(dev->ed_IO, lba, blocks, ptr) != 0)
			return -1;
		
		lba += blocks;
		ptr += bytes;
		rest -= bytes;
	}
	if (rest > 0) {
		if (cache_read(dev, lba, 0, ptr, rest) != 0)
			return -1;
	}
	return (LONG)size;
}

LONG exfat_pwrite(struct exFAT_Device *dev, const void *buffer, size_t size, UQUAD offset)
{
	UBYTE *ptr = (UBYTE *)buffer;
	ULONG rest = (ULONG)size, bytes;
	UQUAD lba;
	UWORD lbo;

	if (rest == 0)
		return -1;

	offset += (dev->ed_First << BLOCKSIZE_SHIFT);

	lba = offset >> BLOCKSIZE_SHIFT;
	lbo = (UWORD)(offset % BLOCKSIZE);

	if (lbo) {
		bytes = MIN(rest % BLOCKSIZE, BLOCKSIZE - lbo);
		if (cache_write(dev, lba, lbo, ptr, bytes) != 0)
			return -1;
		
		lba++;
		ptr += bytes;
		rest -= bytes;
	}
	if (rest >= BLOCKSIZE) {
		ULONG blocks = rest >> BLOCKSIZE_SHIFT;
		bytes = blocks << BLOCKSIZE_SHIFT;
		if (SCSI_Write(dev->ed_IO, lba, blocks, ptr) != 0)
			return -1;

		lba += blocks;
		ptr += bytes;
		rest -= bytes;
	}
	if (rest > 0) {
		if (cache_write(dev, lba, 0, ptr, rest) != 0)
			return -1;
	}
	return (LONG)size;
}

/*------------------------------------------------------------------------*/

LONG exfat_generic_pread(const struct exFAT *ef, struct exFAT_Node *node, void *buffer, size_t size, UQUAD offset)
{
	ULONG cluster;
	char* bufp = buffer;
	UQUAD lsize, loffset, remainder;

	if (offset >= node->en_Size)
		return 0;
	if (size == 0)
		return 0;

	cluster = exfat_advance_cluster(ef, node, offset / CLUSTER_SIZE(*ef->ef_SB));
	if (CLUSTER_INVALID(cluster))
	{
		exfat_error("invalid cluster 0x%lx while reading", cluster);
		return -1;
	}

	loffset = offset % CLUSTER_SIZE(*ef->ef_SB);
	remainder = MIN(size, node->en_Size - offset);
	while (remainder > 0)
	{
		if (CLUSTER_INVALID(cluster))
		{
			exfat_error("invalid cluster 0x%lx while reading", cluster);
			return -1;
		}
		lsize = MIN(CLUSTER_SIZE(*ef->ef_SB) - loffset, remainder);
		if (exfat_pread(ef->ef_Dev, bufp, lsize, exfat_c2o(ef, cluster) + loffset) < 0)
			return -1;

		bufp += lsize;
		loffset = 0;
		remainder -= lsize;
		cluster = exfat_next_cluster(ef, node, cluster);
	}
	if (!ef->ef_ReadOnly && !ef->ef_NoAtime)
		exfat_update_atime(node);

	return MIN(size, node->en_Size - offset) - remainder;
}

LONG exfat_generic_pwrite(struct exFAT *ef, struct exFAT_Node *node, const void *buffer, size_t size, UQUAD offset)
{
	ULONG cluster;
	const char* bufp = buffer;
	UQUAD lsize, loffset, remainder;

	if (offset + size > node->en_Size)
		if (exfat_truncate(ef, node, offset + size) != 0)
			return -1;
	if (size == 0)
		return 0;

	cluster = exfat_advance_cluster(ef, node, offset / CLUSTER_SIZE(*ef->ef_SB));
	if (CLUSTER_INVALID(cluster))
	{
		exfat_error("invalid cluster 0x%lx while writing", cluster);
		return -1;
	}

	loffset = offset % CLUSTER_SIZE(*ef->ef_SB);
	remainder = size;
	while (remainder > 0)
	{
		if (CLUSTER_INVALID(cluster))
		{
			exfat_error("invalid cluster 0x%lx while writing", cluster);
			return -1;
		}
		lsize = MIN(CLUSTER_SIZE(*ef->ef_SB) - loffset, remainder);
		if (exfat_pwrite(ef->ef_Dev, bufp, lsize, exfat_c2o(ef, cluster) + loffset) < 0)
			return -1;

		bufp += lsize;
		loffset = 0;
		remainder -= lsize;
		cluster = exfat_next_cluster(ef, node, cluster);
	}
	exfat_update_mtime(node);
	return size - remainder;
}

