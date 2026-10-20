/* func_L16_00412E70 (212 bytes, 1 place, level 16, default profile -O2 -G0)
   -- measured 0 of 1: SIZE IS RIGHT (212), first difference at +0x1c.

   Level teardown: sets the byte at D_0013963B to 2, then a run of calls, three of
   them guarded and each loading the next handle in its branch's delay slot:

     s0 = *(char **)(arg0 + 0x68);
     D_0013963B = two;                       /* `u8 two = 2;` - see below */
     func_L16_0041ECF8(D_L16_001B2B1C + (*(s32 *)(s0 + 0x190) << 8), 0.0f);
     func_L02_00327350(*(s32 *)(s0 + 0x188), *(s32 *)(s0 + 0x18C));
     func_002DF3F8(D_00189EA0, D_00189EA0 + 0x10);
     func_002AD7B8(0x11);                    /* one argument: a1 keeps 0x189EB0 */
     if (*(s32 *)(s0 + 0x1F0) != 0) func_0029F338((void *)*(s32 *)(s0 + 0x1F0));
     if (*(s32 *)(s0 + 0x238) != 0) func_0029F338((void *)*(s32 *)(s0 + 0x238));
     if (*(s32 *)(s0 + 0x23C) != 0) func_0029F338((void *)*(s32 *)(s0 + 0x23C));
     func_L01_00323310(*(s32 *)(s0 + 0x1A0), -1);
     func_0029B3E0(0, -1);
     func_0029F338(arg0);

   Measured: the store's value MUST come from a local (`u8 two = 2; D_0013963B =
   two;`) - written as a literal the first difference drops from +0x1c to +0x4, and
   putting the store before or after the object load moves it again (+0x8 / +0x4).
   With the local, the prologue, the argument setup of every call and the three
   guarded calls match instruction for instruction; what is left is where the `2`
   is materialised: retail `addiu $v0,$zero,0x2` while the saves are still running
   (0x412E7C) and stores it from `$v0` through a `$v1` base, this cc1 materialises it
   one slot later into `$a0`. Both spellings of the address are relocated, so the
   register is free; only the slot of the `addiu` differs. */

typedef unsigned char u8;
typedef int s32;

extern u8 D_0013963B;
extern char *D_L16_001B2B1C;
extern char D_00189EA0[];
extern void func_L16_0041ECF8(void *, float);
extern void func_L02_00327350(s32, s32);
extern void func_002DF3F8(void *, void *);
extern void func_002AD7B8(s32);
extern void func_0029F338(void *);
extern void func_L01_00323310(s32, s32);
extern void func_0029B3E0(s32, s32);

void func_L16_00412E70(char *arg0) {
    char *s0 = *(char **)(arg0 + 0x68);
    u8 two = 2;

    D_0013963B = two;
    func_L16_0041ECF8(D_L16_001B2B1C + (*(s32 *)(s0 + 0x190) << 8), 0.0f);
    func_L02_00327350(*(s32 *)(s0 + 0x188), *(s32 *)(s0 + 0x18C));
    func_002DF3F8(D_00189EA0, D_00189EA0 + 0x10);
    func_002AD7B8(0x11);
    if (*(s32 *)(s0 + 0x1F0) != 0)
        func_0029F338((void *)*(s32 *)(s0 + 0x1F0));
    if (*(s32 *)(s0 + 0x238) != 0)
        func_0029F338((void *)*(s32 *)(s0 + 0x238));
    if (*(s32 *)(s0 + 0x23C) != 0)
        func_0029F338((void *)*(s32 *)(s0 + 0x23C));
    func_L01_00323310(*(s32 *)(s0 + 0x1A0), -1);
    func_0029B3E0(0, -1);
    func_0029F338(arg0);
}
