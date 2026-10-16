/*
 * UpdateParts: walks the particle blocks (64 bytes each) from the base at 0x1601AC through
 * count + 1 blocks, skips a block whose byte 1 is negative, and calls the update routine for the
 * block's type byte (signed, table D_001CE100). The walk keeps its position and end in 0x1601D0
 * and 0x1601D4 so a callee can change them; the end is read again after each call. The return
 * address spill to 0x1601C8 is the compiler's, not part of the work.
 */
extern s32 D_001CE100_218A80[] __asm__("D_001CE100");

/* Named, not literal addresses: in a level these small-data words are the level program's copies
   (the port relocates named globals of executable code, not integer addresses). */
extern s32 D_001601AC_218A80[] __asm__("D_001601AC");
extern s32 D_001601B4_218A80[] __asm__("D_001601B4");
extern s32 D_001601D0_218A80[] __asm__("D_001601D0");
extern s32 D_001601D4_218A80[] __asm__("D_001601D4");
#define PT_BASE_218A80 (D_001601AC_218A80[0])
#define PT_COUNT_218A80 (D_001601B4_218A80[0])
#define PT_ITER_218A80 (D_001601D0_218A80[0])
#define PT_END_218A80 (D_001601D4_218A80[0])

void func_00218A80(void) {
    char *p;
    char *end;
    s32 b0;
    s32 b1;

    end = (char *)(PT_BASE_218A80 + ((PT_COUNT_218A80 + 1) << 6));
    PT_END_218A80 = (s32)end;
    p = (char *)PT_BASE_218A80;
    while (p != end) {
        b1 = (signed char)p[1];
        b0 = (signed char)p[0];
        p = p + 0x40;
        if (b1 < 0) {
            continue;
        }
        PT_ITER_218A80 = (s32)p;
        ((void (*)(char *))D_001CE100_218A80[b0])(p - 0x40);
        p = (char *)PT_ITER_218A80;
        end = (char *)PT_END_218A80;
    }
}
