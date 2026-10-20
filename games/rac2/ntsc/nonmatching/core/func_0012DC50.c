/* from src/core/0012DA98.c: picture-header setup for the MPEG decoder. It is one of the
   unit's functions saved in 16-byte slots (s1 at +0x10, ra at +0x20 in a 0x30 frame) while
   the stores stay `sd`, which is what config/cflags.txt's `sdk -O2 -g2 -gstabs` gives it -
   the same profile as the neighbouring func_0012D808. With that profile the prologue,
   the func_0012DAC0 call, the 15-bit table index (`dsll 27`/`dsra32`/`andi 0xF`), the
   u32 table entry stored as a quadword and all six lw/sw pairs reproduce retail exactly.
   The candidate is 264 bytes against retail's 272: this cc1 will not make the last
   `return func_0012DA98(s0);` a sibling call, so where retail has
   `ld ra; ld s1; ld s0; j func_12DA98; addiu $sp` it emits `jal` plus a jump to a shared
   epilogue. Nothing else differs. Unblocked by a source shape (or a callee declaration)
   that makes this compiler emit the sibling call for this unit. */
#include "common.h"

extern int func_0012DAC0(int, int, void *, void *, void *);

extern int func_0012D768(int);

extern void func_0012E608(int, int);

extern void func_0012D808(int);

extern int func_0012DA98();

extern unsigned int r1_D_00137F38[];

/* Picture-header setup for the MPEG decoder: hand the three work areas to
   func_0012DAC0, latch the header fields into the decoder object, then run the
   per-picture path and finish in func_0012DA98. */
int func_0012DC50(int s0, int s1) {
    int q = *(int *)(s0 + 0x858);
    int n;

    func_0012DAC0(s0, s1, (void *)(q + 0x10), (void *)(q + 0x18), (void *)(q + 0x20));
    q = *(int *)(s0 + 0x858);
    *(int *)(s0 + 0x80) = *(int *)(q + 0x10);
    n = (int)(*(long long *)(q + 0x20) >> 5);
    *(long long *)(s0 + 0x88) = r1_D_00137F38[n & 0xF];
    *(int *)(s0 + 0xCC) = *(int *)(s1 + 0x5C);
    *(int *)(s0 + 0xD0) = *(int *)(s1 + 0x60);
    *(int *)(s0 + 0xB4) = *(int *)(s1 + 0x44);
    *(int *)(s0 + 0xB8) = *(int *)(s1 + 0x48);
    *(int *)(s0 + 0xBC) = *(int *)(s1 + 0x4C);
    *(int *)(s0 + 0xC0) = *(int *)(s1 + 0x50);
    *(int *)(s0 + 0xC4) = *(int *)(s1 + 0x54);
    *(int *)(s0 + 0xC8) = *(int *)(s1 + 0x58);
    if (func_0012D768(s0) != 0) {
        if (*(int *)(s1 + 0x28) != 1) {
            return 1;
        }
        if (*(int *)(s0 + 0xB0) != 0) {
            func_0012E608(s0, s1);
        } else {
            func_0012D808(s0);
        }
        return func_0012DA98(s0);
    }
    return 0;
}
