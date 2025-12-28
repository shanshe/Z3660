/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 */

#ifndef __ALIGNMEM_H
#define __ALIGNMEM_H

#include <xil_types.h>

#define uswap_16(x) \
	((((x) & 0xff00) >> 8) | \
	 (((x) & 0x00ff) << 8))
#define uswap_32(x) \
	((((x) & 0xff000000) >> 24) | \
	 (((x) & 0x00ff0000) >>  8) | \
	 (((x) & 0x0000ff00) <<  8) | \
	 (((x) & 0x000000ff) << 24))

#define cpu_to_le16(x)		(x)
#define cpu_to_le32(x)		(x)
#define cpu_to_le64(x)		(x)
#define le16_to_cpu(x)		(x)
#define le32_to_cpu(x)		(x)
#define le64_to_cpu(x)		(x)
#define cpu_to_be32(x)		uswap_32(x)
#define be32_to_cpu(x)		uswap_32(x)

#define roundup(x, y) (					\
{							\
	const typeof(y) __y = y;			\
	(((x) + (__y - 1)) / __y) * __y;		\
}\
)

#define min(a, b) (((a) < (b)) ? (a) : (b))

#define min3(x, y, z) min((typeof(x))min(x, y), z)

extern void __bad_unaligned_access_size(void);

static inline u16 get_unaligned_le16(const u8 *p)
{
	return p[0] | p[1] << 8;
}
static inline u32 get_unaligned_le32(const u8 *p)
{
	return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
}
static inline u64 get_unaligned_le64(const u8 *p)
{
	return (u64)get_unaligned_le32(p + 4) << 32 |
	       get_unaligned_le32(p);
}
static inline void put_unaligned_le16(u16 val, u8 *p)
{
	*p++ = val;
	*p++ = val >> 8;
}
static inline void put_unaligned_le32(u32 val, u8 *p)
{
	put_unaligned_le16(val >> 16, p + 2);
	put_unaligned_le16(val, p);
}
static inline void put_unaligned_le64(u64 val, u8 *p)
{
	put_unaligned_le32(val >> 32, p + 4);
	put_unaligned_le32(val, p);
}

#define get_unaligned(ptr) ((typeof(*(ptr)))({			\
	__builtin_choose_expr(sizeof(*(ptr)) == 1, *(ptr),			\
	__builtin_choose_expr(sizeof(*(ptr)) == 2, get_unaligned_le16((const u8 *)(ptr)),	\
	__builtin_choose_expr(sizeof(*(ptr)) == 4, get_unaligned_le32((const u8 *)(ptr)),	\
	__builtin_choose_expr(sizeof(*(ptr)) == 8, get_unaligned_le64((const u8 *)(ptr)),	\
	__bad_unaligned_access_size()))));					\
	}))

#define put_unaligned(val, ptr) ({					\
	void *__gu_p = (ptr);						\
	switch (sizeof(*(ptr))) {					\
	case 1:								\
		*(u8 *)__gu_p = (u8)(val);			\
		break;							\
	case 2:								\
		put_unaligned_le16((u16)(val), __gu_p);		\
		break;							\
	case 4:								\
		put_unaligned_le32((u32)(val), __gu_p);		\
		break;							\
	case 8:								\
		put_unaligned_le64((u64)(val), __gu_p);		\
		break;							\
	default:							\
		__bad_unaligned_access_size();				\
		break;							\
	}								\
	(void)0; })

#define le16_to_cpus(x) do { (void)(x); } while (0)

#define ROUND(a,b)		(((a) + (b) - 1) & ~((b) - 1))
#define __ALIGN_MASK(x,mask)	(((x)+(mask))&~(mask))
#define ALIGN(x,a)		__ALIGN_MASK((x),(typeof(x))(a)-1)
#define ALIGN_END_ADDR(type, ptr, size)	 ((unsigned long)(ptr) + roundup((size) * sizeof(type), USB_DMA_MINALIGN))


/*
 * ARCH_DMA_MINALIGN is defined in asm/cache.h for each architecture.  It
 * is used to align DMA buffers.
 */
#ifndef __ASSEMBLY__
//#include <asm/cache.h>
#include <malloc.h>

/*
 * The ALLOC_CACHE_ALIGN_BUFFER macro is used to allocate a buffer on the
 * stack that meets the minimum architecture alignment requirements for DMA.
 * Such a buffer is useful for DMA operations where flushing and invalidating
 * the cache before and after a read and/or write operation is required for
 * correct operations.
 *
 * When called the macro creates an array on the stack that is sized such
 * that:
 *
 * 1) The beginning of the array can be advanced enough to be aligned.
 *
 * 2) The size of the aligned portion of the array is a multiple of the minimum
 *    architecture alignment required for DMA.
 *
 * 3) The aligned portion contains enough space for the original number of
 *    elements requested.
 *
 * The macro then creates a pointer to the aligned portion of this array and
 * assigns to the pointer the address of the first element in the aligned
 * portion of the array.
 *
 * Calling the macro as:
 *
 *     ALLOC_CACHE_ALIGN_BUFFER(uint32_t, buffer, 1024);
 *
 * Will result in something similar to saying:
 *
 *     uint32_t    buffer[1024];
 *
 * The following differences exist:
 *
 * 1) The resulting buffer is guaranteed to be aligned to the value of
 *    ARCH_DMA_MINALIGN.
 *
 * 2) The buffer variable created by the macro is a pointer to the specified
 *    type, and NOT an array of the specified type.  This can be very important
 *    if you want the address of the buffer, which you probably do, to pass it
 *    to the DMA hardware.  The value of &buffer is different in the two cases.
 *    In the macro case it will be the address of the pointer, not the address
 *    of the space reserved for the buffer.  However, in the second case it
 *    would be the address of the buffer.  So if you are replacing hard coded
 *    stack buffers with this macro you need to make sure you remove the & from
 *    the locations where you are taking the address of the buffer.
 *
 * Note that the size parameter is the number of array elements to allocate,
 * not the number of bytes.
 *
 * This macro can not be used outside of function scope, or for the creation
 * of a function scoped static buffer.  It can not be used to create a cache
 * line aligned global buffer.
 */
#define PAD_COUNT(s, pad) (((s) - 1) / (pad) + 1)
#define PAD_SIZE(s, pad) (PAD_COUNT(s, pad) * pad)
#define ALLOC_ALIGN_BUFFER_PAD(type, name, size, align, pad)		\
	char __##name[ROUND(PAD_SIZE((size) * sizeof(type), pad), align)  \
		      + (align - 1)];					\
									\
	type *name = (type *)ALIGN((uintptr_t)__##name, align)
#define ALLOC_ALIGN_BUFFER(type, name, size, align)		\
	ALLOC_ALIGN_BUFFER_PAD(type, name, size, align, 1)
#define ALLOC_CACHE_ALIGN_BUFFER_PAD(type, name, size, pad)		\
	ALLOC_ALIGN_BUFFER_PAD(type, name, size, ARCH_DMA_MINALIGN, pad)
#define ALLOC_CACHE_ALIGN_BUFFER(type, name, size)			\
	ALLOC_ALIGN_BUFFER(type, name, size, ARCH_DMA_MINALIGN)

/*
 * DEFINE_CACHE_ALIGN_BUFFER() is similar to ALLOC_CACHE_ALIGN_BUFFER, but it's
 * purpose is to allow allocating aligned buffers outside of function scope.
 * Usage of this macro shall be avoided or used with extreme care!
 */
#define DEFINE_ALIGN_BUFFER(type, name, size, align)			\
	static char __##name[ALIGN(size * sizeof(type), align)]	\
			__aligned(align);				\
									\
	static type *name = (type *)__##name
#define DEFINE_CACHE_ALIGN_BUFFER(type, name, size)			\
	DEFINE_ALIGN_BUFFER(type, name, size, ARCH_DMA_MINALIGN)

/**
 * malloc_cache_aligned() - allocate a memory region aligned to cache line size
 *
 * This allocates memory at a cache-line boundary. The amount allocated may
 * be larger than requested as it is rounded up to the nearest multiple of the
 * cache-line size. This ensured that subsequent cache operations on this
 * memory (flush, invalidate) will not affect subsequently allocated regions.
 *
 * @size:	Minimum number of bytes to allocate
 *
 * @return pointer to new memory region, or NULL if there is no more memory
 * available.
 */
/*static inline void *malloc_cache_aligned(size_t size)
{
	return memalign(ARCH_DMA_MINALIGN, ALIGN(size, ARCH_DMA_MINALIGN));
}*/
#endif

#endif /* __ALIGNMEM_H */
