/* Abandoned attempt - func_L05_00441EA0, 140 bytes, 4 places.
   Retail (05_feltzin_system@0x00441EA0) copies a 4-float record twice, from a
   table indexed by the object's +0x84 halfword (stride 32) into the record
   pointed at by the object's +0x70, then copies the record's +0x20 block into the
   object's +0x30 as a 128-bit move, then fills a 16-byte stack buffer from the
   table entry +0x10 and calls func_306E78 (boot name func_002833F0) with the
   object and that buffer. Frame 0x20, `sd $ra,0x10($sp)` only; the table pointer
   is materialised into $a3 and the record pointer into $v1.
   Best result: 128 bytes, expected 140. Residual: this attempt keeps a copy of
   the parameter in $t0 (retail re-reads 0x84($a0) and 0x70($a0)), gives the table
   pointer $v1 and the record $a3 (retail the other way round), and the 128-bit
   move rematerialises `0x20($a3)` instead of reusing the retail's $a1 = record+0x20.
   Tried: swapping the declaration order of table/record, using the same local
   for the float stores and the 128-bit move. */
#include "include_asm.h"

typedef int TI128 __attribute__((mode(TI)));

extern char *D_L05_001B1EC4;
extern int func_002833F0(void *, void *);

typedef struct {
    float x;
    float y;
    float z;
    unsigned int w;
} Vec4_F1B1EC4;

void func_L05_00441EA0(char *object)
{
    char buf[16];
    char *table = D_L05_001B1EC4;
    char *record = *(char **)(object + 0x70);
    char *entry = table + *(short *)(object + 0x84) * 32;
    Vec4_F1B1EC4 *dst = (Vec4_F1B1EC4 *)(record + 0x20);
    Vec4_F1B1EC4 *src = (Vec4_F1B1EC4 *)entry;
    Vec4_F1B1EC4 *second = (Vec4_F1B1EC4 *)(entry + 0x10);
    Vec4_F1B1EC4 *sp = (Vec4_F1B1EC4 *)buf;

    dst->x = src->x;
    dst->y = src->y;
    dst->z = src->z;
    dst->w = 0;
    *(TI128 *)(object + 0x30) = *(TI128 *)dst;
    sp->x = second->x;
    sp->y = second->y;
    sp->z = second->z;
    sp->w = 0;
    func_002833F0(object, buf);
}
