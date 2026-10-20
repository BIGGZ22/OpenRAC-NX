/* from src/core/00124568.c: the row starts 16 bytes late - its first four instructions are
   an orphan frame teardown the body never uses, and the real 160-byte body runs from
   0x1246E0 (candidate is 160 bytes against the row's 176) */
/*
 * libcdvd (0x124xxx, delta +0x3C58): the SIF request path next to WaitGsPathsIdle.
 *
 * The row's first 16 bytes are
 *     0x1246D0  sw $zero,0x30($s2)
 *     0x1246D4  nop
 *     0x1246D8  addiu $sp,$sp,0x60
 *     0x1246DC  nop
 * $s2 is never set anywhere in the row and the frame teardown belongs to a 0x60-frame
 * function that is not this one; no branch in the row reaches 0x1246D0..0x1246DF. The
 * catalogue entry func_00124630 (0x124630, 160 bytes) ends cleanly at 0x1246D0 with its
 * own `jr $ra` at +152 and a 0x20 frame, so the 16 bytes sit between two complete
 * functions. This is the func_0011FB8C shape: a tools/levels.py split unblocks a body
 * that is otherwise plain C.
 *
 * The body from 0x1246E0 takes the pointer argument in $a0, ORs in the uncached alias
 * 0x20000000, and copies two counted byte runs into the destinations the request names -
 * count at +0x0 into *(char **)+0x8 from +0x10, count at +0x4 into *(char **)+0xC from
 * +0x50 - then tail-calls func_124630 with the fixed base 0x1363D4 ($t1 is `lui 0x13`,
 * so `addiu $a0,$t1,0x63D4`). No prologue and no saved registers: both loops index with
 * `$a1` and reload their count each pass, because the destination pointer may alias the
 * request. This part is transcribed below; it was not compiled, so the only measured
 * figure is the row/bookkeeping split above.
 */
typedef struct {
    int count0;          /* +0x00 */
    int count1;          /* +0x04 */
    unsigned char *dst0; /* +0x08 */
    unsigned char *dst1; /* +0x0C */
    /* +0x10: source run 0, +0x50: source run 1 */
} SifRequest_1246D0;

int func_00124630(void *);

int func_001246D0(unsigned int arg0) {
    SifRequest_1246D0 *u = (SifRequest_1246D0 *)(arg0 | 0x20000000);
    unsigned char *src;
    unsigned char *dst;
    int i;

    if (u->count0 > 0) {
        dst = u->dst0;
        src = (unsigned char *)u + 0x10;
        for (i = 0; i < u->count0; i++)
            dst[i] = src[i];
    }
    if (u->count1 > 0) {
        dst = u->dst1;
        src = (unsigned char *)u + 0x50;
        for (i = 0; i < u->count1; i++)
            dst[i] = src[i];
    }
    return func_00124630((void *)0x1363D4);
}
