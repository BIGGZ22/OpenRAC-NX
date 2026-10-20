/* func_L15_00381CF0 (208 bytes, 1 place, level 15, default profile -O2 -G0)
   -- measured 0 of 1: candidate is 216 bytes, TWO instructions over.

   Counts the entries of one id list that pass three tests. Shape, matching retail
   instruction for instruction apart from the residue below:

     slot = D_001FB320[arg0[0x21]];
     if (slot == 0) return 0;
     p = (u16 *)slot;                       /* the 0x7FFF list walk of
                                               src/levels/01_oozla/L01_003CAFE0-g8.c */
     count = 0; f = 0.0f;
     do {
         e = D_001B261C + ((*p & 0x7FFF) << 8);
         if ((*(u16 *)(e + 0x34) & 0x20) != 0) {
             o = *(char **)(e + 0x68);
             v = func_002ED688(e);
             if (*(s32 *)o != 0 && *(s32 *)(v + 0x110) != 0 &&
                 !(f < *(float *)(*(s32 *)o)))
                 count++;
         }
     } while (*(s16 *)p++ >= 0);
     return count;

   The two instructions are the frame: retail keeps the base pointer's address out
   of the loop and re-materialises it (`lui $a0,0x1B ; lw $a0,0x261C($a0)`) because
   its three callee-saved registers are all taken ($s0 = the object, $s1 = p,
   $s2 = the count), frame 0x30. This cc1 hoists that `lui` into `$s3` (a fourth
   saved register), so the frame becomes 0x40 and the `sd`/`ld $s3` pair is the
   whole residue. Measured variants: slot inline in the condition or kept in a local
   (both 216), and the float test written `f < x` or `x <= f` (both 216) - the
   `c.olt.s` + `bc1tl` pair and the FPR nop are already exact. */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;
typedef short s16;

extern u32 D_L15_001FB320[];
extern char *D_L15_001B261C;
extern s32 func_002ED688(void *);

s32 func_L15_00381CF0(char *arg0) {
    u16 *p;
    char *base;
    char *e;
    char *o;
    u32 slot;
    s32 v;
    s32 count = 0;
    float f = 0.0f;

    slot = D_L15_001FB320[(u8)arg0[0x21]];
    if (slot == 0)
        return 0;
    p = (u16 *)slot;
    do {
        base = D_L15_001B261C;
        e = base + ((*p & 0x7FFF) << 8);
        if ((*(u16 *)(e + 0x34) & 0x20) != 0) {
            o = *(char **)(e + 0x68);
            v = func_002ED688(e);
            if (*(s32 *)o != 0 && *(s32 *)(v + 0x110) != 0 &&
                !(f < *(float *)(*(s32 *)o)))
                count++;
        }
    } while (*(s16 *)p++ >= 0);
    return count;
}
