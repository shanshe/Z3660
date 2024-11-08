#ifndef _HUNK_RELOC_H_
#define _HUNK_RELOC_H_

#include <ff.h>
#include "scsi.h"
/*
typedef struct hunk_reloc_ {
    uint32_t src_hunk;
    uint32_t target_hunk;
    uint32_t offset;
} HUNK_RELOC;

typedef struct hunk_info_ {
    uint16_t current_hunk;
    uint16_t num_libs;
    uint8_t *libnames[256];
    uint32_t table_size, byte_size, alloc_size;
    uint32_t base_offset;
    uint32_t first_hunk, last_hunk, num_hunks, header_size;
    uint32_t reloc_hunks;
    uint32_t *hunk_offsets;
    uint32_t *hunk_sizes;
} HUNK_INFO;
*/
enum hunk_types {
    HUNKTYPE_CODE = 0x3E9,
    HUNKTYPE_DATA = 0x3EA,
    HUNKTYPE_BSS = 0x3EB,
    HUNKTYPE_HUNK_RELOC32 = 0x3EC,
    HUNKTYPE_SYMBOL = 0x3F0,
    HUNKTYPE_END = 0x3F2,
    HUNKTYPE_HEADER = 0x3F3,
};


#endif /* _HUNK_RELOC_H_ */
