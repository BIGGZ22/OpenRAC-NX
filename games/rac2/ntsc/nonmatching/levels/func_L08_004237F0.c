/* func_L08_004237F0, 172 bytes, 1 placement (level 8).  Closest candidate: every
   instruction is retail's, in retail's order, with retail's delay-slot fills - the
   one-shot refresh, both helpers gated on the kind word 2, the wrap-guarded counter
   (`sltiu` 0x7FFD) and the colour pulse.  The ONLY difference is at +0x7C, the first
   value computed after the final `jal`: this cc1 loads and compares into $v0
   (`lbu $v0,0x20($s1); sltiu $v0,$v0,0x2`) where retail uses $v1.  Four spellings of
   that guard (>= 2, early return, a named local, an empty else) all give $v0.
   SETTLED AT THE CONTAINER (2026-10-16): `python3 tools/check.py func_L08_004237F0` gives
   `08_tabora: instruction at +0x7c differs` - 0 of 1 - i.e. the same offset the Docker-free
   ruler gives.  The October build does not allocate that register differently, so this is a
   real allocator wall and the same one that keeps func_L18_003F9E50 out.  Do not re-open it
   on a compiler-version hypothesis.  The body: */

typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef int s32;
typedef float f32;

typedef struct { u8 pad[0x100]; } Ent;

extern void func_2F9A90();
extern s32 func_327898(s32, s32);
extern void func_329EC0(Ent *, f32);

void func_L08_004237F0(char *a0) {
    char *s1 = a0;
    char *s0 = *(char **)(s1 + 0x68);

    if (*(u8 *)(s1 + 0xBC) != 0) {
        *(u8 *)(s1 + 0xBC) = 0;
        if (*(s16 *)(s0 + 0x1BA) < 0xC8) {
            *(s16 *)(s0 + 0x1BA) = (s16)func_327898(0x12C, 0x168);
        }
    }
    if (*(s32 *)(s0 + 0x144) == 2) {
        func_2F9A90(s0 + 0x1BA);
    }
    if (*(u16 *)(s1 + 0x96) < 0x7FFD) {
        *(u16 *)(s1 + 0x96) += 1;
    }
    func_2F9A90(s0 + 0x1B8);
    func_2F9A90(s0 + 0x1BC);
    if (*(u8 *)(s1 + 0x20) >= 2) {
        func_329EC0((Ent *)s1, *(f32 *)(s0 + 0x1B0));
    }
}
