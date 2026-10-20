/* func_L05_0041C708 (216 bytes, 4 places: 05, 10, 15, 25) -- NEAR MISS.
   state: the prologue matches retail word for word through +0x2C (same 0x30 frame, same
   s1/s2/f20/s0/ra save order - note this row does NOT hit the save-order wall func_L00_003BFF68
   does), and the first difference is the null test at +0x30.

   Two measured causes:
   (a) the guard must be an enclosing `if (s0 != 0) { ... } return s0;`, not an early
       `if (s0 == 0) return s0;`. Written as an early return this cc1 inverts the branch and adds
       a copy when it can see the value is zero:
         0041C738  bnel $s0,$zero,+0x10  |  beqz $s0, +0x84
         0041C73C  lhu  $v0,0x34($s0)    |  addiu $v1,$zero,0xFF
         0041C740  b    +0x80            |  lhu  $v0,0x34($s0)
         0041C744  daddu $v0,$zero,$zero |  lw   $a3,0x68($s0)
   (b) the three-iteration copy loop comes out REVERSED: this cc1 counts `i = 2; i >= 0; i--`
       (`addiu $a0,$zero,0x2`, `addiu $a0,$a0,-0x1`, `bgez $a0`) where retail counts up
       (`daddu $a0,$zero,$zero`, `slti $v0,$a0,0x3`, `bnez $v0`), and it walks the two pointers in
       the registers $a2/$v1 against retail's $v1/$a1. The body only walks pointers, so the
       direction is the compiler's when nothing depends on the index.

   What is measured and right (all of it below):
   - the whole signature: `(char *a0, char *a1, char *a2, float f12)`; the third integer argument
     is what the factory call receives (`daddu $a0,$a2,$zero` in its delay slot);
   - the emitted flag-store order 0x30, 0x32, 0x31, 0x34, 0x20 is reached by writing the SOURCE in
     retail's literal order (four source orders measured, all identical), so the permutation is not
     the fault here;
   - the two 16-byte copies take the offset-0 `lq`/`sq` form with the source pointer materialised
     into a register (`addiu $a2,$s0,0x10` / `lq $v0,0x0($v1)`), the case the TI128 destination wall
     does not block;
   - the tail: the rung's rate folded as `*(float *)(*(char **)(s0 + 0x24) + 0x24) * f12`, the
     second slot written in the `func_002A0F48` call's delay slot, and one `return s0`. */

typedef unsigned char u8;
typedef unsigned short u16;
typedef float f32;

typedef int Rac2TI128 __attribute__((mode(TI)));

extern char *func_0029EFD0(char *a0);

extern void func_002A0F48(char *a0);

/* Arms the rung the factory returns: stamps the five flag bytes, binds the owner and the handle,
 * copies the owner's two 16-byte blocks into the rung, folds the rate, then starts it. */
char *func_L05_0041C708(char *a0, char *a1, char *a2, f32 f12)
{
    char *s0 = func_0029EFD0(a2);
    char *sub;
    char *src;
    char *dst;
    int i;

    if (s0 != 0) {
        sub = *(char **)(s0 + 0x68);
        *(u8 *)(s0 + 0x30) = 0xFF;
        *(u16 *)(s0 + 0x32) = 0xFF;
        *(u8 *)(s0 + 0x31) = 1;
        *(u16 *)(s0 + 0x34) |= 0x100;
        *(u8 *)(s0 + 0x20) = 0;
        *(char **)sub = a0;
        *(Rac2TI128 *)(s0 + 0x10) = *(Rac2TI128 *)(a0 + 0x10);
        src = a0 + 0xC0;
        dst = s0 + 0xC0;
        for (i = 0; i < 3; i++) {
            *(Rac2TI128 *)dst = *(Rac2TI128 *)src;
            src += 16;
            dst += 16;
        }
        *(f32 *)(s0 + 0x2C) = *(f32 *)(*(char **)(s0 + 0x24) + 0x24) * f12;
        *(char **)(sub + 4) = a1;
        func_002A0F48(s0);
    }
    return s0;
}
