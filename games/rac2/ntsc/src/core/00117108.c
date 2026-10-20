#include "common.h"

/*
 * A version of malloc/free/realloc written by Doug Lea and released to the
 * public domain (dlmalloc 2.6.5), adapted by newlib. See
 * THIRD_PARTY_NOTICES.md.
 */

/* Source: newlib / dlmalloc. */

/* _malloc_trim_r, newlib mallocr.c's malloc_trim, adapted from Lombyte (MIT)
 * src/sdk/library/malloc_trim_r.c and re-pointed at the RAC2 globals. The unit
 * saves $s0..$s4 and $ra in TImode slots (`sq` in 16-byte slots, descending
 * order), so it is built with the -mastra-quad-saves profile like the rest of
 * the allocator family (src/core/00115F28.c, 00115C90.c, 00116360.c).
 * RAC2 addresses: D_001341E8 is __malloc_av_ (its bin 0's fd, the top chunk, at
 * +8), D_00134600 sbrk_base and D_00134618 sbrked_mem. The lock/unlock pair is
 * func_00116300/func_00116360, MORECORE is the _sbrk_r wrapper func_00115C90,
 * and the two 64-bit expressions come out as calls to libgcc's func_00121B20
 * (__divdi3) and func_00121AB8 (__muldi3). */

struct _reent;

struct malloc_chunk {
    u32 prev_size;
    u32 size;
    struct malloc_chunk *fd;
    struct malloc_chunk *bk;
};

extern void func_00116300(struct _reent *);
extern void func_00116360(struct _reent *);
extern void *func_00115C90(struct _reent *ptr, s32 delta);

extern u32 D_001341E8[]; /* __malloc_av_ bins */
extern u32 D_00134600[]; /* sbrk_base */
extern u32 D_00134618[]; /* sbrked_mem */

extern s64 func_00121B20(s64, s64);
extern s64 func_00121AB8(s64, s64);

#define SIZE_SZ               4
#define MINSIZE               16
#define malloc_getpagesize    4096
#define MORECORE(x)           func_00115C90(ptr, (x))
#define MORECORE_FAILURE      ((void *)-1)
#define chunksize(p)          ((p)->size & ~(SIZE_SZ - 1))
#define set_head(p, s)        ((p)->size = (s))

#define av_       D_001341E8
#define bin_at(i) ((struct malloc_chunk *)((u8 *)&(av_[2 * (i) + 2]) - 2 * SIZE_SZ))
#define top       (bin_at(0)->fd)

#define sbrk_base  (D_00134600[0])
#define sbrked_mem (D_00134618[0])

s32 func_00117108(struct _reent *ptr, u32 pad) {
    s64 top_size;
    s64 extra;
    u8 *current_brk;
    u8 *new_brk;
    u64 pagesz = malloc_getpagesize;

    func_00116300(ptr);

    top_size = chunksize(top);
    extra = ((top_size - pad - MINSIZE + (pagesz - 1)) / pagesz - 1) * pagesz;

    if (extra < (s64)pagesz) {
        func_00116360(ptr);
        return 0;
    } else {
        current_brk = (u8 *)MORECORE(0);
        if (current_brk != (u8 *)(top) + top_size) {
            func_00116360(ptr);
            return 0;
        } else {
            new_brk = (u8 *)MORECORE(-extra);
            if (new_brk == (u8 *)MORECORE_FAILURE) {
                current_brk = (u8 *)MORECORE(0);
                top_size = current_brk - (u8 *)(top);
                if (top_size >= (s64)MINSIZE) {
                    sbrked_mem = (u32)(current_brk - (u8 *)sbrk_base);
                    set_head(top, top_size | 1);
                }
                func_00116360(ptr);
                return 0;
            } else {
                set_head(top, (top_size - extra) | 1);
                sbrked_mem -= (u32)extra;
                func_00116360(ptr);
                return 1;
            }
        }
    }
}
