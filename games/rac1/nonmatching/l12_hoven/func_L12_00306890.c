/* NON_MATCHING func_L12_00306890 -- src/overlays/l12_hoven/vendor_002EDAA0.c
 * Best so far: BYTES 22/3740 (99.4% of the bytes match), checked 2026-10-14.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   11 p8 BYTES 640: merged 'off' local.  12 p9 SIZE: qcopy_nc for v1 copy (retail reuses $a0 after the copy).
 *   13 p10 SIZE: color expr 0x7F000000|(c1<<8)|c0, 0025CC58 declared (float*,float,int,float).
 *   14 p11 SIZE: natural qcopy(v2) (hoisted, gives s1 at merge), += in each arm.
 *   15 p12 BYTES 77: hp local for hero pos (retail rematerialises it).  16 p13 BYTES 40: 0.55850536f, ang/s at loo
 *   17 p14 BYTES 22: func_0022ED80 as int-returning, d - -(i*4), own counter k for the 130 loop.
 *   Left: prologue only. Retail computes &v2 into $v0 first and &v1 into $v1 (slots 0x48/0x44); ours &v1 $v0, &v2 
 *   In every arrangement tried (pv1/pv2 vars in any order, natural v1, in-loop pv1) &v1 comes out in $v0. Register
 *   equiv.py p14.c: FUNCTIONAL.
 */
extern float D_0015EE64 MACRO_ADDR;
extern int D_L12_00161F80_m __asm__("D_L12_00161F80") MACRO_ADDR;
extern int D_L12_00161F84_m __asm__("D_L12_00161F84") MACRO_ADDR;
extern int D_L12_00161FA0_s SDATA(D_L12_00161FA0);
extern int D_L12_00161FA4_s SDATA(D_L12_00161FA4);
extern int D_L12_00161FB4 SDATA(D_L12_00161FB4);
extern int D_L12_001FB930[];
extern int D_L12_001FB948[];
extern void func_L00_00263950(char *, char *, int, float, float);
extern float func_L00_0025CC58(float *, float, int, float);
extern float func_L00_0025CCF0(void *, float, void *, int, float, float, float);
extern float func_001FA850(float, float);
extern int func_L00_001FEF78(void *);
extern void *func_L04_002B0068(void *, void *, void *, int);
extern void func_L00_0026A7F8(void *, void *, int, int, int, int, int, int);
extern void func_L00_0026DD70(void *, void *, int, int, float, int);
extern void func_L12_003078C8(void *);
extern int func_0022ED80_09590(int, int, void *) __asm__("func_0022ED80");
void func_L12_00307730(struct Moby *moby, int slot);

typedef struct {
    char pad0[0x60];
    float f60;
    char pad64[0x1C];
    float f80;
    int w84;
    char pad88[4];
    float f8C;
    short h90;
    char b92;
    char b93;
    char pad94[4];
    short h98;
    short h9A;
    short h9C;
    short h9E;
} Gun_306890;

/* Hoven turret battery update: aims each manned gun at the hero, fires, takes hits and smokes when damaged. */
void func_L12_00306890(char *moby) {
    char *d = *(char **)(moby + 0x78);
    int i;
    float v0[4];
    float v1[4];
    float v2[4];
    float v3[4];
    float *pv1;
    float *hp;

    pv1 = v1;

    for (i = 0; i < 5; i++) {
        int idx;
        char *ent;
        char *m;
        Gun_306890 *md;
        float yaw;
        float dist;
        float dz;
        int flag;
        float ang;
        float s;

        if (*(int *)(d - -(i * 4) + 0x5D0) != -1) {
        idx = *(int *)(d - -(i * 4) + 0x5D0);
        ent = d + idx * 0x30;
        m = (char *)(D_L12_00160058_m + (*(int *)(ent + 0x20) << 8));
        md = *(Gun_306890 **)(m + 0x78);
        qcopy(v0, D_L12_001672C0_304310);
        v0[2] -= 0.7f;
        qcopy_nc(pv1, m + 0x10);
        v1[2] += 1.5f;
        md->f60 = func_L00_001FF860(func_001F9D48(pv1, v0), v0[2] - v1[2]);
        func_L00_00263950(m, (char *)md, 0, D_0015EE64 * 0.008f, D_0015EE64 * 0.3f);
        yaw = func_001FA790(func_001FA748(func_L00_001FF860(((Hero *)(D_0013E633_e + 0xE1D))->motion.pos.f[0] - *(float *)(m + 0x10),
                                                            ((Hero *)(D_0013E633_e + 0xE1D))->motion.pos.f[1] - *(float *)(m + 0x14)),
                                          -1.5707964f),
                            *(float *)(moby + 0x48));
        if (md->h90 == 0) {
            if (func_L00_0025CC58((float *)(ent + 0x18), yaw, 0,
                                  D_0015EE6C * 0.5235988f + D_0015EE6C * 0.17453292f * (float)i) < 0.08726646f) {
                md->h90 = 1;
                md->f8C = 1.0f;
                if (i & 1) {
                    md->f8C = -1.0f;
                }
            }
        } else {
            float a = func_001FA748(yaw, md->f8C * 0.55850536f);
            if (func_001F9B88_304310(func_L00_0025CCF0(ent + 0x18, a, (char *)md + 0x94, 0, D_0015EE64 * 0.022f, D_0015EE64 * 0.3f,
                                                D_0015EE6C * 0.2617994f + (float)i * (D_0015EE6C * 0.05235988f)))
                < 0.017453292f) {
                md->f8C = -md->f8C;
            }
        }
        flag = 0;
        hp = (float *)(D_0013E633_e + 0xE1D + 0x80);
        dist = func_001F9D48(moby + 0x10, hp);
        dz = func_001F9B88_304310(hp[2] - *(float *)(moby + 0x18));
        if (func_001F9850(0x4B0) < D_L12_00161FB4) {
            if (dist < 80.0f && dz < 4.0f) {
                flag = 1;
            }
        }
        if (dist > 90.0f || dz > 7.0f) {
            D_L12_00161FB4 = 0;
        }
        if ((D_L12_00161F80_m != 0 || flag) && md->f80 > 0.0f) {
            func_001F9938(&md->h98);
            func_001F9938(&md->h9C);
            if (md->h98 == 0) {
                if (func_001FA850(*(float *)(d + idx * 0x30 + 0x18), yaw) < 0.06981317f) {
                    md->h9E = func_001F9850(0x14);
                }
                if (func_001F9938(&md->h9E) == 0) {
                    if (func_L00_001FEF78(&md->b92) == 0) {
                        if (func_L00_001FEF78(&md->b93) != 0) {
                            func_0022ED80_09590(0, 0, m);
                            md->b93 = func_001F9850(9);
                        }
                    }
                    if (func_001F9938(&md->h9C) != 0) {
                        float rz;
                        float pitch;
                        int n;
                        func_L00_00250800(m, md->h9A + 1, v0);
                        md->h9A = md->h9A == 0;
                        rz = func_001FA748(*(float *)(m + 0x48), 1.5707964f);
                        if (D_L12_00161F80_m != 0) {
                            qcopy(v2, D_L12_001672C0_304310);
                            v2[2] += func_002140F8(-1.5f, 0.3f);
                        } else {
                            qcopy(v2, &((Hero *)(D_0013E633_e + 0xE1D))->motion.unkD0);
                            v2[2] += func_L00_00258C80(0.0f, 0.5f);
                        }
                        n = D_L12_00161FA0_s - 3;
                        if (func_001F9850(5) < n) {
                            n = func_001F9850(5);
                        }
                        if (n < 0) {
                            n = 0;
                        }
                        v2[2] += func_002140F8((float)n * -0.3f, (float)n * 0.3f);
                        pitch = func_L00_001FF860(func_001F9D48(v0, v2), v2[2] - v0[2]);
                        func_00215C00(v1, D_0015EE6C * 80.0f, rz, pitch);
                        if (func_L04_002B0068(m, v0, v1, func_001F9850(0x96)) != 0) {
                            int j;
                            md->b92 = func_001F9850(0x14);
                            for (j = 0; j < 9; j++) {
                                int c0;
                                int c1;
                                int life;
                                v3[0] = func_L00_00258C80(0.0f, D_0015EE6C * 1.7f);
                                v3[1] = func_L00_00258C80(0.0f, D_0015EE6C * 1.7f);
                                v3[2] = func_L00_00258C80(0.0f, D_0015EE6C * 1.7f);
                                c0 = func_L00_00258BC8(0xAA, 0xF0);
                                c1 = func_L00_00258BC8(0x6E, 0xC8);
                                c1 = 0x7F000000 | (c1 << 8) | c0;
                                life = func_001F9850(func_L00_00258BC8(0xF, 0x1E));
                                func_L00_0026A7F8(v0, v3, c1, 0xFF, life, 0x37, func_L00_00258BC8(0xB4, 0x10E), 1);
                            }
                        }
                        md->h9C = func_001F9850(7);
                        n = D_L12_00161FA0_s;
                        if (func_001F9850(0xB) < n) {
                            n = func_001F9850(0xB);
                        }
                        if (n < 0) {
                            n = 0;
                        }
                        md->h9C += n;
                    }
                }
            }
        }
        if (D_L12_00161F84_m == 0 && md->f80 > 0.0f) {
            if (func_001F9908_304920(&md->w84) != 0) {
                float dmg = 1.0f;
                char *r = func_L00_0025B478(m, 0x50000, 0);
                if (r == 0) {
                    int k = D_L12_001FB930[i];
                    char *t = 0;
                    if (k == 0x11) {
                        t = moby;
                    } else {
                        int id = *(int *)(d + k * 0x30 + 0x20);
                        if (id != -1) {
                            t = (char *)(D_L12_00160058_m + (id << 8));
                        }
                    }
                    if (t != 0) {
                        dmg = 0.2f;
                        r = func_L00_0025B478(t, 0x50000, 0);
                        ((unsigned char *)t)[0xA4] = 0xFF;
                    }
                    if (r == 0) {
                        goto done;
                    }
                }
                if (D_L12_00161F80_m == 0) {
                    D_L12_00161FB4 = func_001F9850(0xFA0);
                    goto done;
                }
                md->w84 = func_001F9850(0x14);
                md->f80 -= dmg;
                if (md->f80 <= 0.0f) {
                    int j;
                    int k;
                    *(short *)(d + 0x5FC) -= 0x14;
                    if (*(short *)(d + 0x5FC) < 0) {
                        *(short *)(d + 0x5FC) = 0;
                    }
                    func_0022ED80_09590(1, 0, m);
                    if (*(short *)(d + 0x5FC) > 0) {
                        int k = D_L12_001FB930[i];
                        if (k != 0x11) {
                            D_L12_001FB948[k] = *(int *)(d + k * 0x30 + 0x20);
                            *(int *)(d + k * 0x30 + 0x20) = -1;
                        } else {
                            D_L12_001FB948[0x11] = 1;
                        }
                        func_L12_00307730((struct Moby *)moby, k);
                        if (i == 4) {
                            for (j = 0; j < 0x11; j++) {
                                int id = *(int *)(d + j * 0x30 + 0x20);
                                if (id != -1 && *(short *)((id << 8) + D_L12_00160058_m + 0xA6) == 0x4FD) {
                                    D_L12_001FB948[j] = id;
                                    *(int *)(d + j * 0x30 + 0x20) = -1;
                                    func_L12_00307730((struct Moby *)moby, j);
                                }
                            }
                        }
                    }
                    for (k = 0; k < 0x82; k++) {
                        int c;
                        qcopy(v0, m + 0x10);
                        v0[2] += func_002140F8(0.1f, 0.5f);
                        ang = func_00214158_2EDAA0();
                        s = func_002140F8(0.1f, 0.8f);
                        v0[0] += func_001F9F90(ang) * s;
                        v0[1] += func_001F9FA8(ang) * s;
                        c = func_L00_00258BC8(func_001F9850(0x5A), func_001F9850(0xB4));
                        s = func_002140F8(D_0015EE6C * 0.7f, D_0015EE6C * 1.5f);
                        v1[0] = func_001F9F90(ang) * s;
                        v1[1] = func_001F9FA8(ang) * s;
                        v1[2] = 0.0f;
                        v1[2] = func_002140F8(D_0015EE6C, D_0015EE6C * 5.5f);
                        func_L00_0026DD70(v0, v1, 0x500A1414, 0xA0A0A, 420000.0f, c);
                    }
                    *(int *)(m + 0x94) = 0;
                    *(unsigned short *)(m + 0x34) |= 0x41;
                }
            }
        done:
            ((unsigned char *)m)[0xA4] = 0xFF;
        } else {
            ((unsigned char *)m)[0xA4] = 0xFF;
        }
        if (md->f80 > 0.0f && md->f80 < 10.0f) {
            int lvl = (9 - func_001FA898_3055F8(md->f80)) / 2;
            int j;
            int n = lvl + 3;
            for (j = 0; j < n; j++) {
                int c;
                qcopy(v0, m + 0x10);
                v0[2] += 0.7f;
                ang = func_00214158_2EDAA0();
                s = func_002140F8(0.1f, 0.4f);
                v0[0] += func_001F9F90(ang) * s;
                v0[1] += func_001F9FA8(ang) * s;
                c = func_L00_00258BC8(func_001F9850(0x19), func_001F9850(0x37));
                s = func_002140F8(D_0015EE6C * 0.5f, D_0015EE6C * 3.0f);
                v1[0] = func_001F9F90(ang) * s;
                v1[1] = func_001F9FA8(ang) * s;
                v1[2] = 0.0f;
                v1[2] = func_002140F8(D_0015EE6C * 7.0f, D_0015EE6C * 17.0f) * func_001FA888_3055F8(lvl) * 1.5f
                        / func_001FA888_3055F8(5);
                func_L00_0026DD70(v0, v1, (func_L00_00258BC8(0x46, 0xCD) << 8) | 0x460A00AA, 0xA0A46, 420000.0f, c);
            }
            for (j = 0; j < 3; j++) {
                int c;
                qcopy(v0, m + 0x10);
                ang = func_00214158_2EDAA0();
                s = func_002140F8(0.1f, 0.8f);
                v0[0] += func_001F9F90(ang) * s;
                v0[1] += func_001F9FA8(ang) * s;
                c = func_L00_00258BC8(func_001F9850(0x28), func_001F9850(0x46));
                s = func_002140F8(D_0015EE6C * 0.1f, D_0015EE6C * 0.4f);
                v1[0] = func_001F9F90(ang) * s;
                v1[1] = func_001F9FA8(ang) * s;
                v1[2] = 0.0f;
                v1[2] = func_002140F8(D_0015EE6C * 2.4f, D_0015EE6C * 5.5f);
                func_L00_0026DD70(v0, v1, 0x460A1414, 0xA0A0A, 504000.03f, c);
            }
        }
        }
    }
    func_L12_003078C8(moby);
    if (*(short *)(d + 0x5FC) <= 0 && D_L12_00161F84_m == 0) {
        D_L12_00161FA4_s = 1;
        moby[0x20] = 4;
        moby[0xBC] = 0;
    }
}
