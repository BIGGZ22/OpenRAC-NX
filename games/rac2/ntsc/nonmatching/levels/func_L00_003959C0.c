/* 200 bytes (retail's exact size) with 10 words differing, all register naming and prologue
   interleaving. Everything else is the retail sequence: nine passes over the two halfword
   tables (0x001B2778 tested, 0x001B2790 fed to func_00283CE0), the scaled value
   func_00283CF0(func_00283CE0(state) * 0x001B27DC) handed to func_L00_00395A88 twice with
   the two $gp floats (0x001AF260 / 0x001AF264) and 0.2f, the record advanced by 0x50, then
   func_L00_00396078(A0).
   THE LEVER (measured, kept from worker 10): this row only reaches retail's size in a `-g8`
   sibling compiled with -G8 + -mno-split-addresses; the same source in -G0 is 216 bytes with
   the $gp floats materialised through lui/lwc1. The declarations and the cflags line are
   needed with it.
   What is left: retail gives $s0 to the counter (uses: `i >= 0`, `i--`) and $s3 to the
   record pointer, this cc1 the other way round; the record base pair `lui $v0,0x24` /
   `addiu $s3,$v0,-0x6110` is split around the register saves in retail and adjacent here.
   Sixteen declaration orders measured in two runs (best: p1, p2, rec, s4, i in that order
   with i last -> 10 words), plus fully indexed / strength-reduced spellings (14 words). */
typedef signed short s16;
typedef signed int s32;
typedef float f32;

extern f32 func_00283CE0(int);
extern s32 func_00283CF0(f32);
extern void func_L00_00395A88(void *, int, int, int, f32, f32, f32);
extern void func_L00_00396078(void *);

extern s16 D_L00_001B2778[];
extern s16 D_L00_001B2790[];
extern f32 D_L00_001B27DC;
extern char D_L00_00239EF0[];
extern f32 D_L00_001AF260 __attribute__((sda));
extern f32 D_L00_001AF264 __attribute__((sda));

void func_L00_003959C0(char *a0) {
    int i = 9;
    s16 *p2 = D_L00_001B2790;
    s16 *p1 = D_L00_001B2778;
    char *rec = D_L00_00239EF0;
    char *s4 = a0;

    for (; i >= 0; i--) {
        if (*p1 != 0) {
            int v = func_00283CF0(func_00283CE0(*p2) * D_L00_001B27DC);

            func_L00_00395A88(rec, v, v, 5, D_L00_001AF260, D_L00_001AF264, 0.2f);
        }
        rec += 0x50;
        p2++;
        p1++;
    }
    func_L00_00396078(s4);
}
