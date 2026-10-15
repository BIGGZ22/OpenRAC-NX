/* Near miss from src/levels/20_yeedil/L20_0039C448.c: the build refuses this body at its
   place — 192 bytes, expected 204. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/20_yeedil/L20_0039C448.c. */
extern int D_L20_001B2AC8;

extern void func_L20_0039DF18(char *, int);

extern void func_L20_0039D880(char *);

extern void func_L20_0039D680(char *);

extern float func_00283BE0(float, float);

extern void func_002AAA00(char *, char *, int, float, float, float, float);

void func_L20_0039D7B0(char *arg0) {
    char *state;
    float r;

    func_L20_0039DF18(arg0, 0);
    state = *(char **)(arg0 + 0x68);
    func_L20_0039D880(arg0);
    if ((D_L20_001B2AC8 & 7) == (((int)arg0 >> 8) & 7)) {
        func_L20_0039D680(arg0);
    }
    r = func_00283BE0(*(float *)(state + 0x284), *(float *)(arg0 + 0xF8));
    func_002AAA00(state + 0x288, state + 0x28C, 0, r, 0.009696275f, 0.009696275f,
                  0.5817765f);
    *(float *)(state + 0x284) = *(float *)(arg0 + 0xF8);
    if (*(unsigned short *)(arg0 + 0x96) < 0xFFFF) {
        *(unsigned short *)(arg0 + 0x96) += 1;
    }
}
