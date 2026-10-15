/* functional: medium. Spawns `count` debris/effect objects around a moby plus one particle burst (flag selects the variant and sound set); every call, argument and store follows the assembly. */
extern void burst_DF8A0(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int) __asm__("func_L00_0025F4A8");
extern int rnd_flag_DF8A0(int) __asm__("func_002140B0");
extern char *alloc_DF8A0(int) __asm__("func_0020D348");
extern float rnd01_DF8A0(void) __asm__("func_00214158");
extern float rndr_DF8A0(float, float) __asm__("func_002140F8");
extern void xform_DF8A0(void *, void *, char *) __asm__("func_001F9EE8");
extern void zero_DF8A0(void *) __asm__("func_001F9BC0");
extern void sub_DF8A0(void *, void *, void *) __asm__("func_001F9BF0");
extern void scale_DF8A0(void *, void *, float) __asm__("func_L00_001FF500");
extern int ftoi_DF8A0(float) __asm__("func_001FA898");
extern void launch_DF8A0(void *, void *, void *, void *, int) __asm__("func_L05_00319B58");
extern char *D_L08_0016016C_DF8A0 __asm__("D_L08_0016016C");
extern float D_0015EE6C_DF8A0 __asm__("D_0015EE6C");

typedef struct Q_DF8A0 { long a, b; } Q_DF8A0;

void func_L08_002DF8A0(void *m_, int a1, int count, int flag) {
    char *m = (char *)m_;
    float v20[4];
    float v30[4];
    float v40[4];
    char *pos = m + 0x10;
    char *data = *(char **)(m + 0x78);
    int idx;
    char *p;

    if (flag) {
        burst_DF8A0(m, data + 0x20, pos, 0.0f, 0.0f, 30, 20, 20, 6.0f, 4.0f, 9.0f, 1.0f, -1, 15.0f, 1, 1, -1, 0);
    } else {
        burst_DF8A0(m, data + 0x20, pos, 0.0f, 0.0f, 45, 30, 30, 10.0f, 6.0f, 9.0f, 1.0f, -1, 15.0f, 1, 1, -1, 0);
    }
    idx = a1 << 7;
    while (count > 0) {
        int snd;
        float r;
        count--;
        if (flag) {
            if (rnd_flag_DF8A0(255) & 1) snd = 0x30E; else snd = 0x30F;
        } else {
            if (rnd_flag_DF8A0(255) & 1) snd = 0x310; else snd = 0x311;
        }
        p = alloc_DF8A0(snd);
        *(short *)(p + 0x32) = 0x40;
        *(char *)(p + 0x31) = 1;
        *(char *)(p + 0x30) = 0x40;
        *(long *)(p + 0x38) = *(long *)(m + 0x38);
        *(float *)(p + 0x40) = rnd01_DF8A0();
        *(float *)(p + 0x44) = rnd01_DF8A0();
        *(float *)(p + 0x48) = rnd01_DF8A0();
        v20[0] = rndr_DF8A0(-1.0f, 1.0f);
        v20[1] = rndr_DF8A0(-1.0f, 1.0f);
        v20[2] = rndr_DF8A0(-1.0f, 1.0f);
        v20[3] = 1.0f;
        xform_DF8A0(v20, v20, D_L08_0016016C_DF8A0 + idx);
        *(Q_DF8A0 *)(p + 0x10) = *(Q_DF8A0 *)v20;
        zero_DF8A0(v30);
        sub_DF8A0(v40, v20, pos);
        scale_DF8A0(v40, v40, rndr_DF8A0(D_0015EE6C_DF8A0 * 12.0f, D_0015EE6C_DF8A0 * 15.0f));
        r = rndr_DF8A0(D_0015EE6C_DF8A0 * 6.0f, D_0015EE6C_DF8A0 * 18.0f);
        v40[2] = r;
        launch_DF8A0(p, m, v40, v30, ftoi_DF8A0(rndr_DF8A0(60.0f, 90.0f)));
    }
}
