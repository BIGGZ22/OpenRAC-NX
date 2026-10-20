/* 0 of 1 places match - last line: 08_tabora: instruction at +0x20 differs
   Candidate is 188 bytes against retail's 192, and the first 8 words are retail's exactly:
   the frame, the `lui $v0,0x26` base materialisation, both `sd` of the save block, the
   `daddu $t0,$a0` key copy and the `daddu $s1,$a2` source copy. The body is retail's too -
   the three-slot walk with `i` bumped in the `bnel` delay slot, the 1/4 -> 2/3 state rewrite,
   the `jal` with 0x40, the 16-byte `lq`/`sq` copy at offset 0 on both sides (mode(TI) reaches
   it) and the 1/0 answers.
   WHAT IS LEFT is allocation, and it cascades: retail puts the slot counter in `$a3` and the
   0x1B3A30 walk base in `$a2`, this cc1 the other way round (`i` in `$a2`, walk base in
   `$a1`), which pushes the four state constants down one register each (`1/2/4/3` land in
   `$t2/$t3/$a3/$t1` here against retail's `$t3/$t4/$t1/$t2`) and re-interleaves the three
   base materialisations. Everything before and inside the loop then shifts four bytes.
   Six spellings measured with no move: locals declared src-dst-pA0-p30-base-key (this one),
   walk base before the record base, `i` before `key`, `base` declared last, and the two
   bases declared in retail's own register order. Added levers tried: -mno-split-addresses
   on the same body moves the prologue further away (first difference drops to +0x4), and the
   address literal cannot be used for the base (a bare 0x002603E0 cast emits `lui`/`ori`
   where retail has `lui`/`addiu`; the symbol spelling D_002603E0 is what gives addiu). */
#include "include_asm.h"

typedef signed int s32;
typedef int TI128 __attribute__((mode(TI)));

extern s32 D_L08_001B39A0[];
extern s32 D_L08_001B3A30[];
extern char D_L08_001B3A40[];
extern char D_002603E0[];

extern void func_002F9C68(char *a, s32 b);

/* Releases the one of the three slots whose 0x1B3A30 word names the key, rewriting the
   0x1B39A0 state (1 -> 2, 4 -> 3) and copying the caller's 16 bytes into the slot. */
s32 func_L08_004294A0(s32 a0, s32 a1, char *a2)
{
    char *src = a2;
    char *dst = D_L08_001B3A40;
    s32 *pA0 = D_L08_001B39A0;
    s32 *p30 = D_L08_001B3A30;
    char *base = D_002603E0;
    s32 key = a0;
    s32 i;
    s32 st;

    for (i = 0; i < 3; i++) {
        if (*p30 == key) {
            st = *pA0;
            if (st == 1)
                *pA0 = 2;
            else if (st == 4)
                *pA0 = 3;
            func_002F9C68(base, 0x40);
            *(TI128 *)dst = *(TI128 *)src;
            return 1;
        }
        dst += 0x10;
        base += 0x40;
        pA0++;
        p30++;
    }
    return 0;
}
