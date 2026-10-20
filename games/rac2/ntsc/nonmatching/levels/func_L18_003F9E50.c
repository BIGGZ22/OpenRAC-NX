/* func_L18_003F9E50, 160 bytes, 3 placements (18_damosel and two others).  Closest
   candidate is instruction-for-instruction IDENTICAL to retail - same five $gp table
   addresses, same reload of the sub-object's state index at every call, same delay-slot
   fills - except one register: the last store's constant 1 lands in $v0 here and $v1 in
   retail (`addiu $v0,$zero,0x1; sh $v0,0xAC($s0)` against `addiu $v1,$zero,0x1;
   sh $v1,0xAC($s0)`).  Four spellings of that store (literal, named local, u16 store,
   scoped local) give the same $v0, and the named local costs a word.
   SETTLED AT THE CONTAINER (2026-10-16): `python3 tools/check.py func_L18_003F9E50` gives
   `instruction at +0x88 differs` at all three places - 0 of 3 - the same offset as the
   Docker-free ruler, so the $v0/$v1 split is the allocator and not a build difference.
   It needs a -G8 unit: the five table bases are `$gp + 0x1470/0x1490/0x14B0/0x14D0/0x14F0`,
   i.e. 0x001B0460 / 0x001B0480 / 0x001B04A0 / 0x001B04C0 / 0x001B04E0 with $gp pinned at
   0x001AEFF0 in every program (tools/build.py GP), declared `__attribute__((sda))` in a
   sibling `L18_002C4000-g8.c`.  The body below was measured in exactly that unit: */

typedef unsigned char u8;
typedef short s16;
typedef int s32;

extern s32 D_L18_001B0460 __attribute__((sda));
extern s32 D_L18_001B0480 __attribute__((sda));
extern s32 D_L18_001B04A0 __attribute__((sda));
extern s32 D_L18_001B04C0 __attribute__((sda));
extern s32 D_L18_001B04E0 __attribute__((sda));

extern void func_455AA8(char *a0, char *a1, s32 a2, s32 a3, s32 t0);
extern void func_455A20(s32 a0, s32 a1);
extern void func_455960(char *a0);
extern void func_4559B8(char *a0);
extern void func_L18_002E0070(s32 a0, s32 a1);

void func_L18_003F9E50(char *a0) {
    char *s0 = *(char **)(a0 + 0x68);

    func_455AA8((char *)&D_L18_001B0460 + (*(s16 *)(s0 + 0xAE) << 4),
                (char *)&D_L18_001B0480 + (*(s16 *)(s0 + 0xAE) << 4), 1, 0, 0);
    func_455A20(2, *(s32 *)((char *)&D_L18_001B04E0 + (*(s16 *)(s0 + 0xAE) << 2)));
    func_455960((char *)&D_L18_001B04A0 + (*(s16 *)(s0 + 0xAE) << 4));
    func_4559B8((char *)&D_L18_001B04C0 + (*(s16 *)(s0 + 0xAE) << 4));
    func_L18_002E0070(0x6F, 1);
    *(s16 *)(s0 + 0xAC) = 1;
}
