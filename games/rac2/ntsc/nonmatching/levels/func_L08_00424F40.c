/* 0 of 1 places match - last line: 08_tabora: instruction at +0x80 differs
   Candidate is 184 bytes, retail's exact size, and 45 of its 46 instructions are
   byte-identical: the 0x21 guard, the two stack-buffer calls, the `while` shape with its
   `beql`/`bnel` annulled branches and the `lw $a0,0x68($s0)` reload in the loop-back delay
   slot, and the tail decrement of *(*(s1+0x1FC)) followed by func_L08_00424ED0.
   THE ONE WORD LEFT is the loop test's load: retail `lw $v1,0x0($sp)` (the same $v1 both
   times round), this cc1 loads the second one into $v0. Every loop spelling measured gives
   it: `while (buf != 0)`, `while (buf)`, `for (;;) { if (buf == 0) break; ... }`,
   `while (0 != buf)`, `for (; buf != 0; buf = buf)`, a separate `i = buf` copied at the
   top and the bottom, a `s32 *pb = &buf` form (which instead costs 12 bytes), and reading
   the byte through a named local. That is the register-allocation wall, not a source shape. */
#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef int s32;

extern char *func_00328C40(int *a, s32 b, s32 c, s32 d);
extern void func_00328D28(int *a, s32 b, s32 c, s32 d);
extern void func_L08_00424ED0(char *a);

/* Walks the 0x21 slot's entry list: one record per step, each clamps its byte 0x20D down
   against the context's, and the walk ends when the helper reports the list empty. */
void func_L08_00424F40(char *a0)
{
    char *s0 = a0;
    char *s1;
    s32 buf;

    if (*(u8 *)(s0 + 0x21) == 0xFF)
        return;
    s1 = *(char **)(s0 + 0x68);
    buf = 0;
    func_00328C40(&buf, *(u8 *)(s0 + 0x21), 0, 0);
    while (buf != 0) {
        char *q = *(char **)(s0 + 0x68);
        if (*(u8 *)(q + 0x20D) >= *(u8 *)(s1 + 0x20D))
            *(u8 *)(q + 0x20D) = *(u8 *)(q + 0x20D) - 1;
        func_00328D28(&buf, buf, 0, 0);
    }
    *(s32 *)(*(char **)(s1 + 0x1FC)) = *(s32 *)(*(char **)(s1 + 0x1FC)) - 1;
    func_L08_00424ED0(s0);
}
