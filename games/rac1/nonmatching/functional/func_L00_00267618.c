/* functional: high. corrected from src/overlays/shared/stream_002670E0.c: the definition takes (p, unused), but retail reads only $4 (p); $5 is the table-scan bound, so the second argument is not an input. Body unchanged and checked against asm: if D_0015EE84 >= 0x13 return 0; else scan D_L00_001792B8 over [D_L00_001C47B8[n], D_L00_001C47B8[n+1]) for p and return the index, or -1. */
extern s32 D_0015EE84 MACRO_ADDR;
extern s32 D_L00_001C47B8[];
extern s32 D_L00_001792B8[];

s32 func_L00_00267618(s32 p) {
    s32 n = D_0015EE84;
    s32 lo;
    s32 hi;
    s32 i;
    if ((u32)n >= 0x13) {
        return 0;
    }
    lo = D_L00_001C47B8[n];
    hi = D_L00_001C47B8[n + 1];
    for (i = lo; i < hi; i++) {
        if (D_L00_001792B8[i] == p) {
            return i;
        }
    }
    return -1;
}
