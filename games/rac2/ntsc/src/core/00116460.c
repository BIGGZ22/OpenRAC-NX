#include "common.h"

/* malloc_extend_top (newlib mallocr.c, DEFINE_MALLOC), the allocator's sbrk
   driver. Its source is on this machine: the SDK build tree's
   build/sdk-compiler-build/source/ee/newlib/libc/stdlib/mallocr.c, whose
   statics map one to one onto the fixture this body uses - &av_[0] 0x1341E8
   (initial_top, and `top` is av_[2] at +8), top_pad 0x1345F8 (unsigned long,
   loaded with `ld`), sbrk_base 0x134600 (compared with 0xFFFFFFFF),
   max_sbrked_mem 0x134608 and max_total_mem 0x134610 (both unsigned long),
   current_mallinfo.arena 0x134618 (sbrked_mem). MINSIZE = sizeof(malloc_chunk)
   = 16, SIZE_SZ = 4, MALLOC_ALIGNMENT = 16, PREV_INUSE = 1, SIZE_BITS = 3.
   The unit saves $ra in a TImode slot (`sq`), so -mastra-quad-saves. */
typedef unsigned int INTERNAL_SIZE_T;

struct malloc_chunk_116460 {
    INTERNAL_SIZE_T prev_size;
    INTERNAL_SIZE_T size;
    struct malloc_chunk_116460 *fd;
    struct malloc_chunk_116460 *bk;
};

typedef struct malloc_chunk_116460 *mchunkptr;

struct mallinfo_116460 {
    int arena;
};

#define SIZE_SZ 4
#define MALLOC_ALIGNMENT 16
#define MALLOC_ALIGN_MASK (MALLOC_ALIGNMENT - 1)
#define MINSIZE 16
#define PREV_INUSE 0x1
#define SIZE_BITS (PREV_INUSE | 0x2)
#define MORECORE_FAILURE 0xFFFFFFFFu

#define bin_at(i) ((mchunkptr)((char *)&av_[2 * (i) + 2] - 2 * SIZE_SZ))
#define top (bin_at(0)->fd)
#define initial_top ((mchunkptr)(bin_at(0)))
#define chunk2mem(p) ((void *)((char *)(p) + 2 * SIZE_SZ))
#define chunksize(p) ((p)->size & ~SIZE_BITS)
#define chunk_at_offset(p, s) ((mchunkptr)(((char *)(p)) + (s)))
#define set_head(p, s) ((p)->size = (s))
#define set_head_size(p, s) ((p)->size = (((p)->size & PREV_INUSE) | (s)))
#define sbrked_mem (current_mallinfo.arena)

extern mchunkptr av_[];

extern unsigned long long top_pad;

extern char *sbrk_base;

extern unsigned long long max_sbrked_mem;

extern unsigned long long max_total_mem;

extern struct mallinfo_116460 current_mallinfo;

extern char *func_00115C90(void *, unsigned int);

extern void func_00116E10(void *, void *);

#define MORECORE(reent, size) func_00115C90(reent, (size))

void func_00116460(void *reent_ptr, INTERNAL_SIZE_T nb) {
    char *brk;
    INTERNAL_SIZE_T front_misalign;
    INTERNAL_SIZE_T correction;
    char *new_brk;
    INTERNAL_SIZE_T top_size;

    mchunkptr old_top = top;
    INTERNAL_SIZE_T old_top_size = chunksize(old_top);
    char *old_end = (char *)(chunk_at_offset(old_top, old_top_size));

    INTERNAL_SIZE_T sbrk_size = nb + top_pad + MINSIZE;
    unsigned long long pagesz = 0x1000;

    if (sbrk_base != (char *)(MORECORE_FAILURE))
        sbrk_size = (sbrk_size + (pagesz - 1)) & ~(pagesz - 1);

    brk = (char *)MORECORE(reent_ptr, sbrk_size);

    if (brk == (char *)(MORECORE_FAILURE) ||
        (brk < old_end && old_top != initial_top))
        return;

    sbrked_mem += sbrk_size;

    if (brk == old_end) {
        top_size = sbrk_size + old_top_size;
        set_head(top, top_size | PREV_INUSE);
    } else {
        if (sbrk_base == (char *)(MORECORE_FAILURE))
            sbrk_base = brk;
        else
            sbrked_mem += brk - (char *)old_end;

        front_misalign = (unsigned int)chunk2mem(brk) & MALLOC_ALIGN_MASK;
        if (front_misalign > 0) {
            correction = (MALLOC_ALIGNMENT)-front_misalign;
            brk += correction;
        } else
            correction = 0;

        correction += pagesz - ((unsigned int)(brk + sbrk_size) & (pagesz - 1));

        new_brk = (char *)MORECORE(reent_ptr, correction);
        if (new_brk == (char *)(MORECORE_FAILURE))
            return;

        sbrked_mem += correction;

        top = (mchunkptr)brk;
        top_size = new_brk - brk + correction;
        set_head(top, top_size | PREV_INUSE);

        if (old_top != initial_top) {
            if (old_top_size < MINSIZE) {
                set_head(top, PREV_INUSE);
                return;
            }

            old_top_size = (old_top_size - 3 * SIZE_SZ) & ~MALLOC_ALIGN_MASK;
            set_head_size(old_top, old_top_size);
            chunk_at_offset(old_top, old_top_size)->size = SIZE_SZ | PREV_INUSE;
            chunk_at_offset(old_top, old_top_size + SIZE_SZ)->size = SIZE_SZ | PREV_INUSE;
            if (old_top_size >= MINSIZE)
                func_00116E10(reent_ptr, chunk2mem(old_top));
        }
    }

    if ((unsigned long long)sbrked_mem > max_sbrked_mem)
        max_sbrked_mem = sbrked_mem;
    if ((unsigned long long)(sbrked_mem) > max_total_mem)
        max_total_mem = sbrked_mem;
}
