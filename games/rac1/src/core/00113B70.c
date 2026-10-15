#include "common.h"
#include "structs.h"

/*
 * core_text object 0x113B70-0x114000. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * newlib's freer.o (mallocr.c built with DEFINE_FREE: _free_r and
 * _malloc_trim_r; the SDK's libc.a). Built with Sony's 2.9-ee
 * (Makefile.sn, EE29_CORE), like libc.a.
 */

/* Declarations in scope here before the split. */
extern long func_00116F68(int arg0, int arg1, int arg2);
extern int D_0015ED10;
extern void *D_0012F86C NOT_SDA;
extern int func_001162B8(void *arg0, void *arg1, void *arg2);
extern int func_00116320(void *arg0, void *arg1, void *arg2);
extern long func_001163A0(void *arg0, void *arg1, void *arg2);
extern void func_00116408(void *arg0);
extern void func_00113968(void);
extern void func_00114438(void *, void *);

/*
 * A version of malloc/free/realloc written by Doug Lea and released to the
 * public domain (dlmalloc 2.6.5), adapted by newlib. See
 * THIRD_PARTY_NOTICES.md.
 */

/* Source: newlib / dlmalloc. */

/* func_00113B70 f1: newlib mallocr.c free with the game's arena/globals. */

struct _reent;

extern void func_001154C0(void *);
extern void func_001154C8(void *);
extern int func_00113E90(void *, unsigned int);

extern unsigned int D_0012F888_u[] __asm__("D_0012F888"); /* __malloc_av_ bins */
extern unsigned long D_0012FC90[]; /* trim_threshold */
extern unsigned long D_0012FC98[]; /* top_pad */

struct malloc_chunk {
    unsigned int prev_size;
    unsigned int size;
    struct malloc_chunk *fd;
    struct malloc_chunk *bk;
};

#define SIZE_SZ 4
#define MINSIZE 16
#define PREV_INUSE 1
#define SIZE_BITS (PREV_INUSE | 0x2)
#define MAX_SMALLBIN_SIZE 512
#define BINBLOCKWIDTH 4
#define chunksize(p) ((p)->size & ~(SIZE_BITS))
#define set_head(p, s) ((p)->size = (s))
#define set_head_size(p, s) ((p)->size = (((p)->size & PREV_INUSE) | (s)))
#define set_foot(p, s) (((struct malloc_chunk *)((char *)(p) + (s)))->prev_size = (s))
#define mem2chunk(mem) ((struct malloc_chunk *)((char *)(mem) - 2 * SIZE_SZ))
#define chunk_at_offset(p, s) ((struct malloc_chunk *)(((char *)(p)) + (s)))
#define inuse_bit_at_offset(p, s) (((struct malloc_chunk *)(((char *)(p)) + (s)))->size & PREV_INUSE)
#define smallbin_index(sz) (((unsigned long)(sz)) >> 3)
#define idx2binblock(ix) ((unsigned long)1 << ((ix) / BINBLOCKWIDTH))
#define bin_index(sz)                                                         \
    (((((unsigned long)(sz)) >> 9) == 0) ? (((unsigned long)(sz)) >> 3) :     \
     ((((unsigned long)(sz)) >> 9) <= 4)                                     \
         ? 56 + (((unsigned long)(sz)) >> 6) :                               \
     ((((unsigned long)(sz)) >> 9) <= 20)                                    \
         ? 91 + (((unsigned long)(sz)) >> 9) :                               \
     ((((unsigned long)(sz)) >> 9) <= 84)                                    \
         ? 110 + (((unsigned long)(sz)) >> 12) :                             \
     ((((unsigned long)(sz)) >> 9) <= 340)                                   \
         ? 119 + (((unsigned long)(sz)) >> 15) :                             \
     ((((unsigned long)(sz)) >> 9) <= 1364)                                  \
         ? 124 + (((unsigned long)(sz)) >> 18) :                             \
         126)

#define av_ D_0012F888_u
#define bin_at(i) ((struct malloc_chunk *)((unsigned char *)&(av_[2 * (i) + 2]) - 2 * SIZE_SZ))
#define top (bin_at(0)->fd)
#define last_remainder (bin_at(1))
#define binblocks (bin_at(0)->size)
#define mark_binblock(ii) (binblocks |= (unsigned int)idx2binblock(ii))

#define trim_threshold (D_0012FC90[0])
#define top_pad (D_0012FC98[0])

#define unlink(P, BK, FD)                                                      \
    {                                                                          \
        BK = P->bk;                                                            \
        FD = P->fd;                                                            \
        FD->bk = BK;                                                           \
        BK->fd = FD;                                                           \
    }

#define link_last_remainder(P)                                                 \
    {                                                                          \
        last_remainder->fd = last_remainder->bk = P;                           \
        P->fd = P->bk = last_remainder;                                        \
    }

#define frontlink(P, S, IDX, BK, FD)                                           \
    {                                                                          \
        if (S < MAX_SMALLBIN_SIZE) {                                           \
            IDX = smallbin_index(S);                                           \
            mark_binblock(IDX);                                                \
            BK = bin_at(IDX);                                                  \
            FD = BK->fd;                                                       \
            P->bk = BK;                                                        \
            P->fd = FD;                                                        \
            FD->bk = BK->fd = P;                                               \
        } else {                                                               \
            IDX = bin_index(S);                                                \
            BK = bin_at(IDX);                                                  \
            FD = BK->fd;                                                       \
            if (FD == BK)                                                      \
                mark_binblock(IDX);                                            \
            else {                                                             \
                while (FD != BK && S < chunksize(FD))                          \
                    FD = FD->fd;                                               \
                BK = FD->bk;                                                   \
            }                                                                  \
            P->bk = BK;                                                        \
            P->fd = FD;                                                        \
            FD->bk = BK->fd = P;                                               \
        }                                                                      \
    }

/* _free_r (newlib mallocr.c, dlmalloc 2.6.5). Adapted from Lombyte (MIT)
   for PAL. */
void func_00113B70(struct _reent *ptr, void *mem)
{
    struct malloc_chunk *p;
    unsigned int hd;
    unsigned int sz;
    int idx;
    struct malloc_chunk *next;
    unsigned int nextsz;
    unsigned int prevsz;
    struct malloc_chunk *bck;
    struct malloc_chunk *fwd;
    int islr;

    if (mem == 0)
        return;

    func_001154C0(ptr);

    p = mem2chunk(mem);
    hd = p->size;

    sz = hd & ~PREV_INUSE;
    next = chunk_at_offset(p, sz);
    nextsz = chunksize(next);

    if (next == top) {
        sz += nextsz;

        if (!(hd & PREV_INUSE)) {
            prevsz = p->prev_size;
            p = chunk_at_offset(p, -prevsz);
            sz += prevsz;
            unlink(p, bck, fwd);
        }

        set_head(p, sz | PREV_INUSE);
        top = p;
        if ((unsigned long)(sz) >= (unsigned long)trim_threshold)
            func_00113E90(ptr, top_pad);
        func_001154C8(ptr);
        return;
    }

    set_head(next, nextsz);

    islr = 0;

    if (!(hd & PREV_INUSE)) {
        prevsz = p->prev_size;
        p = chunk_at_offset(p, -prevsz);
        sz += prevsz;

        if (p->fd == last_remainder)
            islr = 1;
        else
            unlink(p, bck, fwd);
    }

    if (!(inuse_bit_at_offset(next, nextsz))) {
        sz += nextsz;

        if (!islr && next->fd == last_remainder) {
            islr = 1;
            link_last_remainder(p);
        } else
            unlink(next, bck, fwd);
    }

    set_head(p, sz | PREV_INUSE);
    set_foot(p, sz);
    if (!islr)
        frontlink(p, sz, idx, bck, fwd);

    func_001154C8(ptr);
}
#undef SIZE_SZ
#undef MINSIZE
#undef PREV_INUSE
#undef SIZE_BITS
#undef MAX_SMALLBIN_SIZE
#undef BINBLOCKWIDTH
#undef chunksize
#undef set_head
#undef set_head_size
#undef set_foot
#undef mem2chunk
#undef chunk_at_offset
#undef inuse_bit_at_offset
#undef smallbin_index
#undef idx2binblock
#undef bin_index
#undef av_
#undef bin_at
#undef top
#undef last_remainder
#undef binblocks
#undef mark_binblock
#undef trim_threshold
#undef top_pad
#undef unlink
#undef link_last_remainder
#undef frontlink

typedef struct MallocChunk {
    unsigned int prev_size;
    unsigned int size;
    struct MallocChunk *fd;
    struct MallocChunk *bk;
} MallocChunk;

typedef struct { char pad[8]; MallocChunk *av2[1]; } MallocState; /* av_[2] = top */

extern MallocState D_0012F888;
extern char *D_0012FCA0; /* malloc_sbrk_base */
extern int D_0012FCB8;   /* current_mallinfo.arena (sbrked_mem) */

extern void func_001154C0(void *); /* MALLOC_LOCK (__malloc_lock, empty) */
extern void func_001154C8(void *); /* MALLOC_UNLOCK (__malloc_unlock, empty) */
extern void *func_001161E8(void *reent_ptr, int size); /* _sbrk_r */

#define TOP (D_0012F888.av2[0])

/* newlib malloc_trim (_malloc_trim_r): gives whole pages of the top chunk
 * back to the system with sbrk when it is more than a page bigger than
 * needed after `pad`, checking first that nothing else moved the break;
 * on failure it resyncs top and sbrked_mem with the real break. The
 * `arena` pointer in the last block is load-bearing: it steers the delay
 * slot of the branch before it. */
int func_00113E90(void *reent_ptr, unsigned int pad) {
    long top_size;
    long extra;
    char *current_brk;
    char *new_brk;
    unsigned long pagesz = 0x1000;

    func_001154C0(reent_ptr);

    top_size = TOP->size & ~3u;
    extra = ((top_size - pad - 0x10 + (pagesz - 1)) / pagesz - 1) * pagesz;

    if (extra < (long)pagesz) {
        func_001154C8(reent_ptr);
        return 0;
    }

    current_brk = (char *)func_001161E8(reent_ptr, 0);
    if (current_brk != (char *)TOP + top_size) {
        func_001154C8(reent_ptr);
        return 0;
    }

    new_brk = (char *)func_001161E8(reent_ptr, -extra);

    if (new_brk == (char *)-1) {
        current_brk = (char *)func_001161E8(reent_ptr, 0);
        top_size = current_brk - (char *)TOP;
        if (top_size >= 0x10) {
            D_0012FCB8 = current_brk - D_0012FCA0;
            TOP->size = top_size | 1;
        }
        func_001154C8(reent_ptr);
        return 0;
    } else {
        int *arena = &D_0012FCB8;
        TOP->size = (top_size - extra) | 1;
        *arena -= extra;
        func_001154C8(reent_ptr);
        return 1;
    }
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_00113FFC);
