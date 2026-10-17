// SPDX-License-Identifier: MIT
#ifndef RDB_SCAN_H
#define RDB_SCAN_H

/* Where an Amiga RigidDiskBlock may live -- as a pure policy, with the transport
 * behind two callbacks so a host test can drive it (test/rdb_scan_test.cpp).
 * scsi.c's piscsi_parse_rdb() has no scan loop of its own for image files: there is
 * one implementation of this policy and the test exercises that one.
 *
 * The RDB may sit in any of the first RDB_LOCATION_LIMIT (16) blocks of a disk
 * (NDK devices/hardblocks.h), and Commodore's own mounters scan all of them. The old
 * image-file scan read 64 KB per step and tested only the first four bytes of each
 * read, so it sampled offsets 0, 64K, 128K, ... and never saw an RDB in blocks 1..15.
 * A disk partitioned that way -- e.g. Amiga UNIX, whose installer leaves block 0 empty
 * and puts the RDB at block 2 -- came up with made-up CHS and no partitions.
 *
 * THE THREE PHASES, AND WHAT EACH ONE PROTECTS
 *
 *  1. Sector 0.  The old first iteration tested these same four bytes, so every disk
 *     with an RDB at sector 0 -- every AmigaOS disk -- is decided on the first read,
 *     with the result it has always had.  No checksum is applied: a sector-0 RDB is
 *     accepted on its magic alone, exactly as before, so no disk that boots today
 *     can be rejected.
 *
 *  2. Sectors 1..RDB_SCAN_LIMIT-1 -- but only on a disk whose sector 0 is EMPTY.  A
 *     hybrid image carries a foreign container or an MBR at sector 0 plus an Amiga RDB
 *     further down (a VHD with its 'conectix' cookie at sector 0 is one).  The old scan
 *     missed that RDB, so such an image has only ever been used with synthetic
 *     geometry, and handing it real RDB geometry would leave Kickstart unable to read
 *     it.  A non-empty sector 0 means something already claims the front of the disk,
 *     so phase 2 is skipped and the disk keeps the old result exactly.  A candidate
 *     here must also pass the standard RDB checksum, so a stray 'RDSK' in the front of
 *     a data disk cannot be mistaken for a partition table.
 *
 *  3. The old 64K sample points (64K, 128K, ...).  Nothing spec-conformant can live out
 *     there, but a disk that is found there today must still be found there, so they
 *     are kept in their old order.
 */

#include <stdint.h>

#ifndef RDB_SCAN_SECTOR
#define RDB_SCAN_SECTOR 512
#endif
/* The old scan's read granularity (= PISCSI_MAX_BLOCK_SIZE). */
#ifndef RDB_SCAN_CHUNK
#define RDB_SCAN_CHUNK 65536
#endif
/* RDB_LOCATION_LIMIT (devices/hardblocks.h), and the old loop bound, are both 16. */
#ifndef RDB_SCAN_LIMIT
#define RDB_SCAN_LIMIT 16
#endif
/* 'RDSK' */
#ifndef RDB_SCAN_IDENTIFIER
#define RDB_SCAN_IDENTIFIER 0x5244534BUL
#endif

typedef enum {
   RDB_SCAN_NONE = 0,   /* no RDB -- the caller synthesises CHS, as it always did */
   RDB_SCAN_AT_SECTOR0, /* sector 0: every AmigaOS disk */
   RDB_SCAN_SUB_LBA0,   /* sectors 1..15 of an empty-sector-0 disk */
   RDB_SCAN_AT_STRIDE   /* one of the old 64K sample points */
} rdb_scan_result;

typedef struct {
   /* Read one RDB_SCAN_SECTOR-byte sector. Return 1 on success, 0 on short/failure. */
   int (*read_sector)(void *ctx, uint32_t sector, uint8_t *buf);
   /* Read one RDB_SCAN_CHUNK-byte chunk at chunk index. Return bytes delivered. */
   uint32_t (*read_chunk)(void *ctx, uint32_t index, uint8_t *buf);
   void *ctx;
} rdb_reader;

/* Big-endian long load, so the policy needs no endian header of its own. */
static inline uint32_t rdb_be32(const uint8_t *p)
{
   return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
          ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

/* Does this sector start with 'RDSK'? */
static inline int rdb_magic_here(const uint8_t *block)
{
   return rdb_be32(block) == (uint32_t)RDB_SCAN_IDENTIFIER;
}

/* Is this sector entirely zero -- i.e. does nothing claim it? */
static inline int rdb_sector_is_empty(const uint8_t *block)
{
   for (int j = 0; j < RDB_SCAN_SECTOR; j++)
      if (block[j])
         return 0;
   return 1;
}

/* The standard RDB checksum: the first rdb_SummedLongs big-endian longs sum to zero.
 * Applied only to a phase-2 candidate; see the header comment. */
static inline int rdb_checksum_ok(const uint8_t *block)
{
   uint32_t summed = rdb_be32(&block[4]);
   uint32_t sum = 0;
   if (summed < 2 || summed > 128)
      return 0;
   for (uint32_t j = 0; j < summed; j++)
      sum += rdb_be32(&block[j * 4]);
   return sum == 0;
}

/* Run the policy.  On a hit, `block` holds the RDB sector/chunk and *at receives the
 * sector index (phases 1-2) or the chunk index (phase 3).  `block` must be at least
 * RDB_SCAN_CHUNK bytes, because phase 3 reads into it at that size. */
static inline rdb_scan_result rdb_scan(const rdb_reader *r, uint8_t *block, int *at)
{
   int i;

   *at = 0;

   /* Phase 1 -- sector 0. */
   if (!r->read_sector(r->ctx, 0, block))
      return RDB_SCAN_NONE;
   if (rdb_magic_here(block))
      return RDB_SCAN_AT_SECTOR0;

   /* Phase 2 -- sectors 1..RDB_SCAN_LIMIT-1, only if sector 0 is empty. */
   if (rdb_sector_is_empty(block))
   {
      for (i = 1; i < RDB_SCAN_LIMIT; i++)
      {
         if (!r->read_sector(r->ctx, (uint32_t)i, block))
            break;
         if (rdb_magic_here(block) && rdb_checksum_ok(block))
         {
            *at = i;
            return RDB_SCAN_SUB_LBA0;
         }
      }
   }

   /* Phase 3 -- the old 64K sample points, in their old order. */
   for (i = 1; i < RDB_SCAN_LIMIT; i++)
   {
      if (r->read_chunk(r->ctx, (uint32_t)i, block) != RDB_SCAN_CHUNK)
         break;
      if (rdb_magic_here(block))
      {
         *at = i;
         return RDB_SCAN_AT_STRIDE;
      }
   }

   return RDB_SCAN_NONE;
}

#endif /* RDB_SCAN_H */
