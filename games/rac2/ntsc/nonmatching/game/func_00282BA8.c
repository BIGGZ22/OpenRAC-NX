/* func_00282BA8 -- the engine's draw setup, 56 bytes in retail.

   Retail: three registers go to the engine at 0x1000D080, 0x1000D010 and
   0x1000D020, the command 0x100 goes to 0x1000D000, a `sync` follows, then
   the word at 0x20100000 is read back with its low bit set.

   Measured with tools/check.py in src/game/0027F128.c (the -O2 -G0 profile):
   "func_00282BA8: 0 of 28 places match", 44 bytes against retail's 56.

   What differs: the `sync` (0x0000000F) in the middle of the body.  This cc1
   has no way to emit it from plain C: `__sync_synchronize()` compiles into a
   call to an undefined external (the same wall as
   nonmatching/core/func_0011B0B0.c), and only an inline `asm` produces the
   instruction itself, which is not allowed.  The rest of the body is one
   register short: retail materialises the engine base once in `$at` and
   offsets it (0x80, 0x10, 0x20, 0x0), the candidate keeps the base in `$v0`
   and uses the trapping `add` for the 0x100.
*/
typedef unsigned int u32;

u32 func_00282BA8(u32 a0, u32 a1, u32 a2) {
    volatile u32 *p = (volatile u32 *)0x1000D000;
    p[0x20] = a1;
    p[0x04] = a0;
    p[0x08] = a2;
    p[0x00] = 0x100;
    return *(volatile u32 *)0x20100000 | 1;
}
