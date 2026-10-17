/* Host test for where an Amiga RDB may live (../src/scsi/rdb_scan.h).
 * Build and run on the host:  c++ -std=c++11 -Wall -O2 rdb_scan_test.cpp -o rdb_scan_test && ./rdb_scan_test
 *
 * It pins the ORDER of the three phases, the empty-sector-0 key, the checksum
 * qualification of a candidate in blocks 1..15, and that a sector-0 RDB short-circuits
 * before anything else is read. It cannot catch a FatFS or SD transport fault, the cache
 * maintenance around each read, or what the caller does with the geometry afterwards --
 * those live on the far side of the callbacks that scsi.c supplies.
 */
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <vector>

extern "C" {
#include "../src/scsi/rdb_scan.h"
}

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, ...) do { \
   if (cond) { g_pass++; } \
   else { g_fail++; printf("  FAIL: " __VA_ARGS__); printf("   [%s:%d]\n", __FILE__, __LINE__); } \
} while (0)

/* ---- a synthetic disk, and the two seams rdb_scan() reads it through ---------------- */

struct FakeDisk {
   std::vector<uint8_t> bytes;
   int sector_reads = 0;
   int chunk_reads = 0;

   explicit FakeDisk(size_t size = 4u * 1024u * 1024u) : bytes(size, 0) {}

   void put32(size_t off, uint32_t v) {
      bytes[off + 0] = (uint8_t)(v >> 24); bytes[off + 1] = (uint8_t)(v >> 16);
      bytes[off + 2] = (uint8_t)(v >> 8);  bytes[off + 3] = (uint8_t)v;
   }
   uint32_t get32(size_t off) const {
      return ((uint32_t)bytes[off] << 24) | ((uint32_t)bytes[off + 1] << 16) |
             ((uint32_t)bytes[off + 2] << 8) | (uint32_t)bytes[off + 3];
   }

   /* A structurally valid RDB at `sector`, with the checksum made to balance
    * (SummedLongs 64, BlockBytes 512). */
   void put_rdb(uint32_t sector, bool valid_checksum = true) {
      size_t off = (size_t)sector * 512;
      memset(&bytes[off], 0, 512);
      memcpy(&bytes[off], "RDSK", 4);
      put32(off + 4, 64);          /* rdb_SummedLongs */
      put32(off + 8, 0);           /* rdb_ChkSum, filled below */
      put32(off + 12, 7);          /* rdb_HostID */
      put32(off + 16, 512);        /* rdb_BlockBytes */
      put32(off + 64, 0x000003F1); /* some geometry so the block is not all-magic */
      uint32_t sum = 0;
      for (int j = 0; j < 64; j++) sum += get32(off + (size_t)j * 4);
      put32(off + 8, valid_checksum ? (uint32_t)(0u - sum) : 0xDEADBEEFu);
   }

   /* An MBR: boot code, a partition table, and the $55AA signature. */
   void put_mbr() {
      memset(&bytes[0], 0, 512);
      bytes[0] = 0xFA; bytes[1] = 0x33; bytes[2] = 0xC0;   /* cli; xor ax,ax */
      bytes[446] = 0x80; bytes[450] = 0x83;                /* active, type Linux */
      bytes[510] = 0x55; bytes[511] = 0xAA;
   }

   /* A VHD footer cookie at sector 0, as a VHD-wrapped hybrid image carries it. */
   void put_conectix() {
      memset(&bytes[0], 0, 512);
      memcpy(&bytes[0], "conectix", 8);
      put32(8, 0x00000002);
      put32(12, 0x00010000);
   }
};

static int fake_read_sector(void *ctx, uint32_t sector, uint8_t *buf)
{
   FakeDisk *d = (FakeDisk *)ctx;
   d->sector_reads++;
   size_t off = (size_t)sector * RDB_SCAN_SECTOR;
   if (off + RDB_SCAN_SECTOR > d->bytes.size()) return 0;
   memcpy(buf, &d->bytes[off], RDB_SCAN_SECTOR);
   return 1;
}

static uint32_t fake_read_chunk(void *ctx, uint32_t index, uint8_t *buf)
{
   FakeDisk *d = (FakeDisk *)ctx;
   d->chunk_reads++;
   size_t off = (size_t)index * RDB_SCAN_CHUNK;
   if (off >= d->bytes.size()) return 0;
   size_t n = d->bytes.size() - off;
   if (n > RDB_SCAN_CHUNK) n = RDB_SCAN_CHUNK;
   memcpy(buf, &d->bytes[off], n);
   return (uint32_t)n;
}

static rdb_scan_result run(FakeDisk &d, int *at)
{
   static uint8_t block[RDB_SCAN_CHUNK];
   rdb_reader r = { fake_read_sector, fake_read_chunk, &d };
   memset(block, 0xA5, sizeof(block));   /* poison: no result may come from stale buffer */
   return rdb_scan(&r, block, at);
}

int main(void)
{
   printf("=== Z3660 core0 RDB location policy ===\n");
   int at = -1;

   /* 1. Every existing deployment. An AmigaOS disk has RDSK at sector 0, and the old
    *    scan's first iteration tested those same four bytes, so it must still be decided there
    *    -- and decided on the FIRST read, before any new code can run. */
   printf("[test] an RDB at sector 0 is found first, and nothing else is read\n");
   {
      FakeDisk d; d.put_rdb(0);
      CHECK(run(d, &at) == RDB_SCAN_AT_SECTOR0, "sector-0 RDB is found at sector 0\n");
      CHECK(at == 0, "reported at sector 0, got %d\n", at);
      CHECK(d.sector_reads == 1, "exactly one read, got %d\n", d.sector_reads);
      CHECK(d.chunk_reads == 0, "the 64K scan never runs, got %d\n", d.chunk_reads);
   }

   /* 2. A sector-0 RDB is accepted on its magic alone, exactly as it always was. Adding
    *    a checksum test here would REJECT a disk that boots today, so the qualification
    *    must apply only below sector 0. */
   printf("[test] a sector-0 RDB with a broken checksum is still accepted\n");
   {
      FakeDisk d; d.put_rdb(0, /*valid_checksum=*/false);
      CHECK(run(d, &at) == RDB_SCAN_AT_SECTOR0, "no new rejection at sector 0\n");
   }

   /* 3. The case this is for. An Amiga UNIX disk has RDSK at sector 2 with sector 0
    *    all zeroes. */
   printf("[test] an Amiga UNIX layout (empty sector 0, RDSK at sector 2) is found\n");
   {
      FakeDisk d; d.put_rdb(2);
      CHECK(run(d, &at) == RDB_SCAN_SUB_LBA0, "the sector-2 RDB is found\n");
      CHECK(at == 2, "reported at sector 2, got %d\n", at);
   }

   /* 4. A hybrid image must keep getting the synthetic-geometry fallback: its RDB
    *    further down is real, but it has only ever been used without it. */
   printf("[test] a hybrid disk (MBR at sector 0) keeps the old result\n");
   {
      FakeDisk d; d.put_rdb(2); d.put_mbr();
      CHECK(run(d, &at) == RDB_SCAN_NONE, "MBR at sector 0 suppresses the sub-LBA0 scan\n");
   }
   printf("[test] a VHD hybrid (conectix at 0, RDSK at 10) is refused\n");
   {
      FakeDisk d; d.put_rdb(10); d.put_conectix();
      CHECK(run(d, &at) == RDB_SCAN_NONE, "a VHD cookie at sector 0 also claims the disk\n");
   }

   /* 5. The key is emptiness, not MBR-ness: ANY claimant at sector 0 suppresses phase 2.
    *    A single stray byte is enough, because a disk whose front is not blank has some
    *    other owner and the old scan has always given it the fallback. */
   printf("[test] any non-zero byte in sector 0 suppresses the sub-LBA0 scan\n");
   {
      FakeDisk d; d.put_rdb(2); d.bytes[511] = 0x01;
      CHECK(run(d, &at) == RDB_SCAN_NONE, "one byte at the end of sector 0 is a claim\n");
      FakeDisk e; e.put_rdb(2); e.bytes[0] = 0x01;
      CHECK(run(e, &at) == RDB_SCAN_NONE, "one byte at the start of sector 0 is a claim\n");
   }

   /* 6. A sub-LBA0 candidate must pass the standard RDB checksum, so a stray 'RDSK' in
    *    the front of a data disk cannot be promoted into a partition table. */
   printf("[test] a sub-LBA0 candidate is checksum-qualified\n");
   {
      FakeDisk d; d.put_rdb(2, /*valid_checksum=*/false);
      CHECK(run(d, &at) == RDB_SCAN_NONE, "a bad checksum below sector 0 is refused\n");
      FakeDisk e;
      memcpy(&e.bytes[3 * 512], "RDSK", 4);   /* bare magic, nothing else */
      CHECK(run(e, &at) == RDB_SCAN_NONE, "bare 'RDSK' text is not an RDB\n");
   }

   /* 7. The spec's location limit is the loop bound, both ends of it. */
   printf("[test] the sub-LBA0 scan spans exactly sectors 1..%d\n", RDB_SCAN_LIMIT - 1);
   {
      FakeDisk d; d.put_rdb(1);
      CHECK(run(d, &at) == RDB_SCAN_SUB_LBA0 && at == 1, "sector 1 is in range\n");
      FakeDisk e; e.put_rdb(RDB_SCAN_LIMIT - 1);
      CHECK(run(e, &at) == RDB_SCAN_SUB_LBA0 && at == RDB_SCAN_LIMIT - 1,
            "the last in-range sector is scanned\n");
      FakeDisk f; f.put_rdb(RDB_SCAN_LIMIT);
      CHECK(run(f, &at) == RDB_SCAN_NONE, "one sector past the limit is out of range\n");
   }

   /* 8. The old 64K sample points still work, in their old order. Nothing conformant
    *    lives out there, but a disk found there today must still be found there. */
   printf("[test] the old 64K-stride sample points are preserved\n");
   {
      FakeDisk d; d.put_mbr(); d.put_rdb(RDB_SCAN_CHUNK / 512);   /* = byte offset 64K */
      CHECK(run(d, &at) == RDB_SCAN_AT_STRIDE, "the 64K sample point is still tested\n");
      CHECK(at == 1, "reported at chunk 1, got %d\n", at);
   }

   /* 9. Phase order, where two candidates compete. Sector 0 beats everything; a sub-LBA0
    *    RDB beats a 64K one; and on a claimed disk the 64K one still wins by default. */
   printf("[test] the phases are tried in order\n");
   {
      FakeDisk d; d.put_rdb(2); d.put_rdb(0);
      CHECK(run(d, &at) == RDB_SCAN_AT_SECTOR0, "sector 0 beats sector 2\n");
      FakeDisk e; e.put_rdb(RDB_SCAN_CHUNK / 512); e.put_rdb(2);
      CHECK(run(e, &at) == RDB_SCAN_SUB_LBA0 && at == 2, "sector 2 beats the 64K point\n");
   }

   /* 10. A disk with no RDB anywhere still reports none -- the caller then synthesises
    *     CHS exactly as it always has. Includes the all-zero blank, which now takes the
    *     cheap 512-byte path instead of reading a megabyte. */
   printf("[test] a disk with no RDB reports none\n");
   {
      FakeDisk d;
      CHECK(run(d, &at) == RDB_SCAN_NONE, "an all-zero disk has no RDB\n");
      FakeDisk e; e.put_mbr();
      CHECK(run(e, &at) == RDB_SCAN_NONE, "an MBR-only disk has no RDB\n");
   }

   /* 11. A read that fails must not be read past, and must never yield a result out of
    *     the caller's poisoned buffer. */
   printf("[test] a failed sector-0 read yields none\n");
   {
      FakeDisk d(0);   /* smaller than one sector: every read fails */
      CHECK(run(d, &at) == RDB_SCAN_NONE, "an unreadable disk has no RDB\n");
      CHECK(d.chunk_reads == 0, "phase 3 is not attempted after phase 1 fails\n");
   }

   /* 12. The predicates themselves, at their boundaries. */
   printf("[test] the predicates hold at their edges\n");
   {
      uint8_t s[RDB_SCAN_SECTOR];
      memset(s, 0, sizeof(s));
      CHECK(rdb_sector_is_empty(s), "an all-zero sector is empty\n");
      s[RDB_SCAN_SECTOR - 1] = 1;
      CHECK(!rdb_sector_is_empty(s), "the last byte counts\n");
      memcpy(s, "RDSK", 4);
      CHECK(rdb_magic_here(s), "'RDSK' is the magic\n");
      memcpy(s, "RDSk", 4);
      CHECK(!rdb_magic_here(s), "the magic is case-exact\n");
      FakeDisk d; d.put_rdb(0);
      CHECK(rdb_checksum_ok(&d.bytes[0]), "a balanced RDB passes the checksum\n");
      d.bytes[20] ^= 0xFF;
      CHECK(!rdb_checksum_ok(&d.bytes[0]), "a flipped byte fails the checksum\n");
      FakeDisk e; e.put_rdb(0);
      e.put32(4, 1);
      CHECK(!rdb_checksum_ok(&e.bytes[0]), "SummedLongs below 2 is not an RDB\n");
      e.put32(4, 129);
      CHECK(!rdb_checksum_ok(&e.bytes[0]), "SummedLongs past the sector is not an RDB\n");
   }

   printf("\n=== %d passed, %d failed ===\n", g_pass, g_fail);
   return g_fail ? 1 : 0;
}
