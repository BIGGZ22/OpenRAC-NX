/* functional: medium. level 12 moby update (class 19): state 0 claims a slot in the 0x1190-byte table and builds it,
 * state 2 steers, then the per-frame tail. Not a match (1380 bytes against 1420). Starts from the near miss in
 * nonmatching/l12_hoven/, with two differences found by tools/equiv.py: the second near-hero test stores the
 * moby's z (+0x18) like the first, not its distance; the 1/sqrt(3) constant is 0x3F13CD36 (0.57735f). */
extern char D_0013E633[];
extern char D_L12_0020BF80[];
extern short D_L12_00161410;
extern char D_L12_001CC080[];
extern char D_L12_001CAF80[];
extern char D_L12_001803C0[];
extern int D_L12_00161350 MACRO_ADDR;
extern int D_L12_00161354 MACRO_ADDR;
extern int D_L12_00161358 MACRO_ADDR;
extern float D_L12_0016128C MACRO_ADDR;
extern float D_L12_00161290 MACRO_ADDR;
extern float D_L12_00161294 MACRO_ADDR;
extern int D_L12_00161288 MACRO_ADDR;
extern float D_0015EE70 MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;
extern char D_L12_001672C0[];
extern char D_L12_001CC1C0[];
extern int func_001E9730();
extern void func_L01_002B8C00(void *, int);
extern void func_L01_002B90A8(float s);
extern float func_L00_001FF860(float, float);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern void func_L01_002B8E20(float *in, int *out);
extern void func_L12_002C0940(int);
extern float func_00214D88(float *, float *, float, float, float, float);
extern void func_L11_0030FB58(char *moby);
extern float func_001F9B88(float);
extern int func_002140B0(int);
extern float func_002140F8(float, float);
extern void func_L00_002A5158(void *, int, int, float, float, float, float);
extern int func_L01_00276680(char *vec, float scale);
extern int func_00215570();
extern void func_L01_002B9198(char *p, int n);
extern void func_001F49B0(void *, void *);
extern void func_L12_002C0918(void);

/* Level 12 moby update (class 19): state 0 claims a slot and builds the table, state 2 steers, then the per-frame tail. */
void func_L12_002C0310(char *moby) {
    char *data;
    char *rec;
    char *x;
    char *p;
    float *P;
    float f20;
    float f21;
    float f;
    float g;
    float r1;
    float r2;
    int st;
    int n;
    int k;
    int idx;
    int r;

    st = *(unsigned char *)(moby + 0x20);
    data = *(char **)(moby + 0x78);
    if (st != 1) {
        if (st < 2) {
        n = *(int *)&D_L12_00161410 + 1;
        *(int *)(data + 0xC) = *(int *)&D_L12_00161410;
        *(int *)&D_L12_00161410 = n;
        if (n >= 0x80) {
            func_001E9730(D_L12_0020BF80);
        }
        f = *(float *)(*(char **)(moby + 0x24) + 0x24);
        *(float *)(moby + 0x2C) = f + f;
        *(int *)data = 3;
        *(unsigned char *)(moby + 0x30) = 0xFF;
        *(unsigned short *)(moby + 0x34) = *(unsigned short *)(moby + 0x34) | 1;
        if (*(int *)(data + 0xC) == 0) {
            p = D_L12_001CC1C0 + 0x1191E;
            k = 0x10;
            do {
                *(unsigned short *)p = 0;
                k = k - 1;
                p = p - 0x1190;
            } while (k >= 0);
            f20 = 1.0f;
            func_L01_002B8C00(D_L12_001CC1C0, 0x11);
            func_L01_002B90A8(f20);
            P = (float *)D_L12_001CAF80;
            P[0] = 16.0f;
            P[3] = -8.0f;
            P[6] = 0.9f;
            P[2] = -8.0f;
            P[8] = 0.1f;
            P[5] = 1.0f;
            P[4] = 1.0f;
            P[1] = 16.0f;
            f20 = func_L00_001FF860(*(float *)(D_L12_001803C0 + 0x10), *(float *)(D_L12_001803C0 + 0x14));
            P[12] = func_001F9F90(f20) * 0.57735f;
            g = func_001F9FA8(f20) * 0.57735f;
            P[14] = -0.57735f;
            P[13] = g;
            *(int *)(P + 10) = *(int *)data + 0x28;
            *(unsigned char *)((char *)P + 0x3D) = 0x40;
            *(unsigned char *)((char *)P + 0x3C) = 0x1C;
            *(int *)(P + 11) = *(int *)(data + 4) + 0x28;
            *(int *)&D_L12_00161350 = (int)D_L12_001CC1C0;
            *(int *)&D_L12_00161354 = (int)D_L12_001CC080;
            *(int *)&D_L12_00161358 = 0x11;
            D_L12_0016128C = 32768.0f;
            D_L12_00161290 = 255.0f;
            D_L12_00161294 = 48.0f;
            D_L12_00161288 = 0;
        }
        idx = *(int *)(data + 0xC);
        *(float *)(D_L12_001CC1C0 + *(int *)(data + 0xC) * 0x1190) = *(float *)(moby + 0x10);
        *(float *)(D_L12_001CC1C0 + *(int *)(data + 0xC) * 0x1190 + 4) = *(float *)(moby + 0x14);
        *(float *)(D_L12_001CC1C0 + *(int *)(data + 0xC) * 0x1190 + 8) = *(float *)(moby + 0x18);
        *(int *)(D_L12_001CC1C0 + *(int *)(data + 0xC) * 0x1190 + 0x14) = *(int *)data + 0x28;
        *(int *)(D_L12_001CC1C0 + *(int *)(data + 0xC) * 0x1190 + 0x18) = *(int *)(data + 4) + 0x28;
        *(unsigned char *)(D_L12_001CC1C0 + *(int *)(data + 0xC) * 0x1190 + 0x1C) = 0x1C;
        *(unsigned char *)(D_L12_001CC1C0 + *(int *)(data + 0xC) * 0x1190 + 0x1D) = 0x44;
        *(unsigned short *)(D_L12_001CC1C0 + *(int *)(data + 0xC) * 0x1190 + 0x1E) = 0xFFFF;
        func_L01_002B8E20((float *)(D_L12_001CC1C0 + *(int *)(data + 0xC) * 0x1190), (int *)(D_L12_001CC080 + *(int *)(data + 0xC) * 16));
        *(unsigned char *)(moby + 0x20) = 1;
        } else if (st == 2) {
        if (*(unsigned char *)(moby + 0xBC) & 4) {
            *(float *)(data + 0x10) = *(float *)(data + 0x10) + 5.0f;
            *(int *)(data + 0x14) = 1;
        }
        *(unsigned char *)(moby + 0xBC) = 1;
        if (*(int *)(data + 0x14) != 0) {
            func_00214D88((float *)(moby + 0x18), (float *)(data + 8), *(float *)(data + 0x10), D_0015EE70 * 4.0f, D_0015EE70 * 4.0f, D_0015EE6C * 4.0f);
            func_L11_0030FB58(moby);
        }
        x = D_0013E633 + 0xE1D;
        f = func_001F9B88(*(float *)(x + 0x80) - *(float *)(moby + 0x10));
        if (f <= 8.0f) {
            *(float *)(x + 0x2F0) = *(float *)(moby + 0x18);
        } else {
            g = func_001F9B88(*(float *)(x + 0x84) - *(float *)(moby + 0x14));
            if (g <= 8.0f) {
                *(float *)(x + 0x2F0) = *(float *)(moby + 0x18);
            }
        }
        if (func_002140B0(200) == 0) {
            r1 = func_002140F8(-4.0f, 4.0f);
            f20 = *(float *)(D_L12_001CC1C0 + *(int *)(data + 0xC) * 0x1190) + r1;
            r2 = func_002140F8(-4.0f, 4.0f);
            func_L00_002A5158(D_L12_001CC1C0 + *(int *)(data + 0xC) * 0x1190, 1, 1, f20, *(float *)(D_L12_001CC1C0 + *(int *)(data + 0xC) * 0x1190 + 4) + r2, 1.0f, -0.05f);
        }
        }
    } else {
        func_L12_002C0940(*(int *)(data + 0xC));
        *(int *)&D_L12_00161410 = 0;
        *(unsigned char *)(moby + 0x20) = 2;
        *(float *)(data + 0x10) = *(float *)(moby + 0x18);
    }

    if (*(int *)(data + 0xC) >= 0) {
        r = func_L01_00276680(moby, 64.0f);
        f20 = 0.0f;
        if (r != -1) {
            f20 = 1.0f;
        }
        if (*(int *)(data + 0x18) != -1) {
            func_00215570(D_L12_001672C0);
            f20 = 0.0f;
        }
        idx = *(int *)(data + 0xC);
        rec = D_L12_001CC1C0 + idx * 0x1190;
        if (f20 == 0.0f) {
            *(unsigned short *)(D_L12_001CC1C0 + *(int *)(data + 0xC) * 0x1190 + 0x1E) = 0;
        } else {
            *(unsigned short *)(D_L12_001CC1C0 + *(int *)(data + 0xC) * 0x1190 + 0x1E) = 0xFFFF;
        }
        if (*(int *)(data + 0xC) == 0) {
            func_L01_002B9198(D_L12_001CC1C0, 0x11);
            func_001F49B0((void *)func_L12_002C0918, moby);
        }
    }
}
