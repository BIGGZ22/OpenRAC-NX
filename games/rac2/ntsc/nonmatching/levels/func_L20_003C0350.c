/* Near miss from src/levels/20_yeedil/L20_0039C448.c: the build refuses this body at its
   place — instruction at +0x6c differs. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/20_yeedil/L20_0039C448.c. */
extern char *D_L20_001FBB20[];

extern char *D_L20_001B2F9C;

extern float D_L20_0023E670[];

extern float func_00283CE0(int);

extern float func_002A7AA8(float, float, float, float, float);

void func_L20_003C0350(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    short *p = (short *)D_L20_001FBB20[*(int *)state];
    float *out;
    char *tbl;
    char *rec;
    float k;
    float s;
    float v;
    float t;

    if (p != 0) {
        k = func_00283CE0(*(int *)(state + 4)) * *(float *)(state + 8);
        s = func_002A7AA8(-1.0f, 0.0f, 1.0f, 0.0f, 1.0f - k);
        out = D_L20_0023E670;
        tbl = D_L20_001B2F9C;
        do {
            v = *out++;
            t = v - 20.0f;
            rec = tbl + ((*p & 0x7FFF) << 8);
            *(unsigned short *)(rec + 0x34) &= 0xFFBE;
            *(float *)(rec + 0x18) = t + (v - t) * s;
        } while (*p++ >= 0);
    }
}
