/* 200 bytes (retail's exact size) with 14 words differing, all of them the prologue's
   interleaving: retail sets up `lui $a1,0x5` before the register saves and keeps `a1`
   live across them, this cc1 saves first and materialises `0x7F7F7F7F`/the 0.05f earlier.
   The body is retail's: func_002FC560(0x47, 0x53001); the state at A0+0x68;
   func_L00_0041B1B8(A0, 0x7F7F7F7F, 0x4B, 0.05f); then if (*(float *)(state+0x50) != 0.0f)
   advance the state by 0x10 and issue func_L00_0041AFD8(A0, state, 0x7F10307F, 0.25f) and
   func_L00_0041AFD8(A0, state, 0x3F08183F, 0.5f); then func_L00_0041BCC8(A0) and
   func_002FC560(0x47, 0x5360B).
   TRAP the first reading falls into: the two func_L00_0041AFD8 floats are NOT the same -
   retail materialises 0x3E800000 (0.25f) for the first and 0x3F000000 (0.5f) for the second,
   which is why no callee-saved FPR is used. Writing 0.5f twice gives 208 bytes with an
   f20 save. Three shapes measured on the corrected constants (two plain literals, the
   goto form, and `s0 + 0x10` twice) all land on 200 bytes / 14 words. */
extern void func_002FC560(int, int);
extern void func_L00_0041B1B8(void *, int, int, float);
extern void func_L00_0041AFD8(void *, void *, int, float);
extern void func_L00_0041BCC8(void *);

void func_L00_0041AE28(char *a0) {
    char *s1 = a0;
    char *s0;

    func_002FC560(0x47, 0x53001);
    s0 = *(char **)(s1 + 0x68);
    func_L00_0041B1B8(s1, 0x7F7F7F7F, 0x4B, 0.05f);
    if (*(float *)(s0 + 0x50) != 0.0f) {
        s0 += 0x10;
        func_L00_0041AFD8(s1, s0, 0x7F10307F, 0.25f);
        func_L00_0041AFD8(s1, s0, 0x3F08183F, 0.5f);
    }
    func_L00_0041BCC8(s1);
    func_002FC560(0x47, 0x5360B);
}
