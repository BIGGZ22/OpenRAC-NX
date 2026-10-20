/* func_L23_0041C578 (312 bytes, level 23 only, uses $gp: belongs in L23_00416388-g8.c) -- NEAR MISS, not integrated.
   state: 308 bytes against retail's 312 (tools/check.py, src/levels/23_damosel_orbit/L23_00416388-g8.c, -O2 -G8).
   Still different:
   - the missing instruction: retail zeroes the 128-bit vector in a register with
     `por $v0,$zero,$zero` (MMI, word 0x700014A9, after the `bne` and its delay slot) and stores that register with
     `sq $v0,0x20($sp)`; this compiler stores the zero register itself (`sq $zero,0x20($sp)`). A TI128 zero
     local and a zeroed union copied in are folded the same way (the union copy costs a frame slot: 324 bytes);
   - with the `por` put back, four words of the then-branch are in another order: retail sets $a0 after the
     4.5f constant (ours before it) and stores the vector before the `addiu $a2,$s0,0xC0` (ours after it).
   Right: the base pointer in $s1 (D_00189E20, its +0x2290 word read, +0x80 passed on), the float stores of
   1.3f, 4.5f and 1.0f over the zero vector, the two vector calls, the 14-argument call with its four stack
   words, the else arm, and the closing test of +0xAA against 0x118F before the func_002E5CF8 call. */

typedef int TI128 __attribute__((mode(TI)));

extern char *D_L23_001B1484 __attribute__((sda));

extern int func_002E5CF8(int index, int a1, char *a2);

extern void func_00283098(char *, char *, char *);

extern void func_00282CC0(char *, char *, char *);

extern void func_0032C700(char *, int, int, int, int, int, int, int, float, float, int, int, int, int);

extern char D_00189E20[];

/* Builds the measured vector when the state's first word names the argument, and lays out the call. */
void func_L23_0041C578(char *arg0) {
    union {
        TI128 q;
        float f[4];
    } vec;
    char *base = D_00189E20;

    if (*(char **)(base + 0x2290) == arg0) {
        vec.q = 0;
        vec.f[0] = 1.3f;
        vec.f[2] = 4.5f;
        vec.f[3] = 1.0f;
        func_00283098((char *)&vec, (char *)&vec, arg0 + 0xC0);
        func_00282CC0((char *)&vec, (char *)&vec, base + 0x80);
        func_0032C700((char *)&vec, 0, 10, 10, 10, 25, 30, 0, 3.0f, 20.0f, 0, 1, 0, 0);
    } else {
        func_0032C700(arg0 + 0x10, 0, 25, 20, 20, 15, 25, 1, 1.5f, 20.0f, 0, 0, 0, 0);
    }
    if (*(short *)(arg0 + 0xAA) != 0x118F)
        func_002E5CF8(3, 0, D_L23_001B1484);
}
