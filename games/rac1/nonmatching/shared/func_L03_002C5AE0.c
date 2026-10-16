/* NON_MATCHING func_L03_002C5AE0 -- src/overlays/shared/vendor_00292AC0.c
 * Best so far: SIZE ours 2280 / retail 2304, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Bomb update (class 545), 2304 bytes. Best candidate p5.c: 2280 bytes, 24 short (about 6 instructions), not EXA
 *   Still differing: retail places the state-0 body out of line behind a branch-likely on st==0 (ours falls throug
 */
extern int func_001F9908(int *arg0);
extern void func_L00_001FF4B0(void *, void *, float);
extern int func_L00_001EFFF0(void *, void *, int, void *, void *);
extern void func_001F9BF0(void *, void *, void *);
extern void func_L00_001FF610(void *, void *, void *);
extern float func_001F9CE8(void *);
extern float func_L00_001FF860(float, float);
extern int func_L00_001F3958(void);
extern void func_001F9C30(void *, void *, float);
extern float func_001F9CB8(void *a);
extern int func_L00_001F10E0(float, void *, int, void *);
extern float func_L00_0025F368(float);
extern float func_00214358(void *, int, float);
extern void func_L00_0025B040(unsigned char *, float);
extern float D_0015EE70 MACRO_ADDR;
extern char D_L03_00173FE0[];

/* bomb update: drifts toward its owner's position, claims a child, and clamps its scale */
void func_L03_002C5AE0(char *moby)
{
    char *d;
    char *owner;
    char *parent;
    char *pos;
    char *g;
    char *o;
    char *p;
    float a[4];
    float t[4];
    float v[4];
    float f0;
    float f1;
    float f2;
    float f3;
    float f4;
    float c;
    int sv[4];
    int st;

    if (moby == 0) return;
    d = *(char **)(moby + 0x78);
    if (d == 0) return;
    func_001F9908((int *)(d + 0x20));
    pos = moby + 0x10;
    qcopy(a, pos);
    st = ((unsigned char *)moby)[0x20];
    if (st != 1) {
        if (st < 2) {
            if (st == 0) {
            parent = *(char **)(moby + 0x24);
            f3 = *(float *)(moby + 0x2C);
            f0 = D_0015EE60 * 0.05f;
            f1 = (*(float *)(parent + 0x24) - f3) * f0;
            f3 = f3 + f1;
            *(float *)(moby + 0x2C) = f3;
            func_001F9BD8(pos, pos, d);
            func_L00_001FF4B0(t, d, 0.2f);
            func_001F9BD8(t, t, pos);
            sv[0] = *(int *)(moby + 0x98);
            sv[2] = *(int *)(moby + 0x94);
            owner = *(char **)(d + 0x10);
            sv[3] = *(int *)(owner + 0x94);
            sv[1] = *(int *)(owner + 0x98);
            *(int *)(moby + 0x98) = -1;
            *(int *)(*(char **)(d + 0x10) + 0x98) = -1;
            *(int *)(moby + 0x94) = 0;
            *(int *)(*(char **)(d + 0x10) + 0x94) = 0;
            if (func_L00_001EFFF0(a, t, 4, moby, 0)) {
                g = D_L03_00173FE0;
                func_001F9BF0(t, g, t);
                func_L00_001FF4B0(t, t, 0.215f);
                func_001F9BD8(pos, pos, t);
                p = *(char **)(g - 0x20 + 0x18);
                if (p == 0) {
                    func_L00_001FF610(d, d, g + 0x20);
                    f0 = func_001F9CE8(g + 0x20);
                    if (func_L00_001FF860(*(float *)(g - 0x20 + 0x48), f0) < 0.6981317f) {
                        if (func_L00_001F3958() == -1) {
                            func_001F9C30(d, d, 0.5f);
                            if (func_001F9CB8(d) < 0.025f) ((unsigned char *)moby)[0x20] = 1;
                        }
                    }
                } else if (*(short *)(p + 0xA6) != 0x221) {
                    func_L00_001FF610(d, d, g + 0x20);
                    f0 = func_001F9CE8(g + 0x20);
                    if (func_L00_001FF860(*(float *)(g - 0x20 + 0x48), f0) < 0.6981317f) {
                        if (func_L00_001F3958() == -1) {
                            func_001F9C30(d, d, 1.2f);
                        }
                    }
                } else {
                    f0 = func_002140F8(0.9f, 1.1f);
                    *(float *)(d + 0x8) = *(float *)(d + 0x8) * f0;
                }
                if (*(int *)(d + 0x20) == 0) {
                    func_0022ED80(0, 0, (int)moby);
                    *(int *)(d + 0x20) = 0xC;
                }
            }
            if (func_L00_001F10E0(0.2f, pos, 4, moby)) {
                g = D_L03_00173FE0;
                func_001F9BF0(t, g, g + 0x10);
                func_L00_001FF4B0(t, t, 0.015f);
                func_001F9BD8(pos, g + 0x10, t);
                p = *(char **)(g - 0x20 + 0x18);
                if (p == 0) {
                    func_L00_001FF610(d, d, g + 0x20);
                    f0 = func_001F9CE8(g + 0x20);
                    if (func_L00_001FF860(*(float *)(g - 0x20 + 0x48), f0) < 0.6981317f) {
                        if (func_L00_001F3958() == -1) {
                            func_001F9C30(d, d, 0.5f);
                            if (func_001F9CB8(d) < 0.025f) ((unsigned char *)moby)[0x20] = 1;
                        }
                    }
                } else if (*(short *)(p + 0xA6) != 0x221) {
                    func_L00_001FF610(d, d, g + 0x20);
                    f0 = func_001F9CE8(g + 0x20);
                    if (func_L00_001FF860(*(float *)(g - 0x20 + 0x48), f0) < 0.6981317f) {
                        if (func_L00_001F3958() == -1) {
                            func_001F9C30(d, d, 1.2f);
                        }
                    }
                } else {
                    f0 = func_002140F8(0.9f, 1.1f);
                    *(float *)(d + 0x8) = *(float *)(d + 0x8) * f0;
                }
                if (*(int *)(d + 0x20) == 0) {
                    func_0022ED80(0, 0, (int)moby);
                    *(int *)(d + 0x20) = 0xC;
                }
            }
            o = D_L03_00160064_p;
            if (o != 0) {
                float k21 = -0.5f;
                float k20 = 1.0f;
                while (o != 0) {
                    if (o != moby && ((unsigned char *)o)[0x20] != 0xFE && ((unsigned char *)o)[0x20] != 0xFD) {
                        func_001F9BF0(v, pos, o + 0x10);
                        v[2] = 0.0f;
                        f0 = func_001F9CB8(v);
                        if (f0 < 1.8f) {
                            f0 = 1.8f - f0;
                            c = D_0015EE60 * k21 + k20;
                            func_001F9C30(v, v, f0 * c);
                            func_001F9BD8(pos, pos, v);
                        }
                    }
                    o = *(char **)(o + 0x28);
                }
            }
            if (func_001F9938(d + 0x14)) ((unsigned char *)moby)[0x20] = 2;
            *(float *)(moby + 0x40) = func_L00_0025F368(*(float *)(d + 0x0));
            *(float *)(moby + 0x44) = func_L00_0025F368(*(float *)(d + 0x4));
            f4 = D_0015EE70 * 9.8f;
            *(int *)(moby + 0x98) = sv[0];
            *(int *)(*(char **)(d + 0x10) + 0x98) = sv[1];
            *(int *)(moby + 0x94) = sv[2];
            *(int *)(*(char **)(d + 0x10) + 0x94) = sv[3];
            *(float *)(d + 0x8) = *(float *)(d + 0x8) - f4;
            f2 = D_0015EE60 * 0.01f;
            f3 = D_0015EE60 * 0.02f;
            *(float *)(moby + 0x40) = *(float *)(moby + 0x40) + f2;
            *(float *)(moby + 0x44) = *(float *)(moby + 0x44) + f3;
            }
        } else if (st == 2) {
            f3 = D_0015EE60 * 0.05f;
            f0 = *(float *)(moby + 0x2C);
            f1 = f0 * f3;
            *(float *)(moby + 0x2C) = f0 - f1;
            p = *(char **)(d + 0x18);
            if (p != 0 && (*(int *)(d + 0x1C) & 4)) {
                f1 = *(float *)(p + 0x2C);
                f0 = *(float *)(*(char **)(p + 0x24) + 0x24);
                f0 = (f0 - f1) * f3;
                *(float *)(p + 0x2C) = f1 + f0;
            }
            f1 = D_0015EE60 * 0.05f;
            f2 = *(float *)(moby + 0x2C);
            f0 = *(float *)(*(char **)(moby + 0x24) + 0x24) * f1;
            if (f2 < f0) {
                *(float *)(moby + 0x2C) = 0.0001f;
                p = *(char **)(d + 0x18);
                if (p == 0 || ((unsigned char *)p)[0x20] == 0xFE || ((unsigned char *)p)[0x20] == 0xFD
                    || (*(int *)(d + 0x1C) & 4) == 0
                    || *(float *)(*(char **)(p + 0x24) + 0x24) - 0.01f <= *(float *)(p + 0x2C)) {
                    func_0020D678(moby);
                    return;
                }
            }
        }
    } else {
        f0 = func_00214358(pos, 0, 0.5f);
        *(int *)(moby + 0x1C) = 0;
        *(float *)(moby + 0x18) = f0 + 0.2f;
        qcopy(t, pos);
        *(int *)(d + 0x18) = (int)func_L03_002C0028(*(char **)(d + 0x10), t, -1);
        ((unsigned char *)moby)[0x20] = 2;
    }
    f1 = *(float *)(moby + 0x2C) * 0.3f;
    parent = *(char **)(moby + 0x24);
    func_L00_0025B040((unsigned char *)moby, f1 / *(float *)(parent + 0x24));
    if (*(float *)(moby + 0x18) < 5.0f || 500.0f < *(float *)(moby + 0x18)) {
        func_0020D678(moby);
        return;
    }
    if (*(float *)(moby + 0x10) < 5.0f) *(float *)(moby + 0x10) = 5.0f;
    if (1018.0f < *(float *)(moby + 0x10)) *(float *)(moby + 0x10) = 1018.0f;
    if (*(float *)(moby + 0x14) < 5.0f) *(float *)(moby + 0x14) = 5.0f;
    if (1018.0f < *(float *)(moby + 0x14)) *(float *)(moby + 0x14) = 1018.0f;
    if (*(float *)(moby + 0x18) < 5.0f) *(float *)(moby + 0x18) = 5.0f;
    if (1018.0f < *(float *)(moby + 0x18)) *(float *)(moby + 0x18) = 1018.0f;
}
