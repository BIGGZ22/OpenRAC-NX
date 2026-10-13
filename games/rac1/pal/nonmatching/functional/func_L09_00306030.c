/* functional: medium. Gaspar level pickup/enemy moby update (states 0, 4, 5): state 0 initialises, state 4 hovers/approaches the hero and drives a light effect record, state 5 is steered by the pad (ship-like) with two spark-burst animations; the first part handles the hit test with the hero. Not runnable in capture; hand-checked against the assembly, no NTSC draft existed. */
extern unsigned char D_0013E633_6030[] __asm__("D_0013E633");
extern unsigned char D_0013A5E0_6030[] __asm__("D_0013A5E0");
extern unsigned char D_0015EED0_6030[] __asm__("D_0015EED0");
extern float D_0015EE6C_6030 __asm__("D_0015EE6C");
extern float D_0015EE70_6030 __asm__("D_0015EE70");
extern float D_0015EE60_6030 __asm__("D_0015EE60");
extern float D_L09_00161D40_6030[] __asm__("D_L09_00161D40");
extern float D_L09_00161D30_6030 __asm__("D_L09_00161D30");
extern float D_L09_00161D34_6030 __asm__("D_L09_00161D34");
extern unsigned char D_L09_00166E80_6030[] __asm__("D_L09_00166E80");
extern unsigned char D_L09_001804C0_6030[] __asm__("D_L09_001804C0");
extern void vecof_6030(void *, int, void *) __asm__("func_L00_00250800");
extern int dec_6030(short *) __asm__("func_001F9938");
extern float dist_6030(void *, void *) __asm__("func_001F9D10");
extern float dist2_6030(void *, void *) __asm__("func_001F9D48");
extern int scale_6030(int) __asm__("func_001F9850");
extern void snd_6030(int, int) __asm__("func_L00_00222B80");
extern float addrot_6030(float, float) __asm__("func_001FA748");
extern float cos_6030(float) __asm__("func_001F9F90");
extern float sin_6030(float) __asm__("func_001F9FA8");
extern void vadd_6030(void *, void *, void *) __asm__("func_001F9BD8");
extern void clear_6030(void *, int) __asm__("func_001F99D8");
extern void cleanup_6030(int) __asm__("func_L00_002EC0C8");
extern void animate_6030(void *, int, int, int) __asm__("func_00213DE0");
extern void f2EBF50_6030(void *, void *, int, int, int) __asm__("func_L00_002EBF50");
extern float getf_6030(void *) __asm__("func_0020D830");
extern float absf_6030(float) __asm__("func_001F9B88");
extern void setdata_6030(void *, int, void *) __asm__("func_0020D960");
extern void f2F10F8_6030(void *, void *) __asm__("func_L09_002F10F8");
extern void voice_6030(int, int, void *) __asm__("func_0022ED80");
extern float atan2_6030(float, float) __asm__("func_L00_001FF860");
extern float approach_6030(float, float) __asm__("func_001FA790");
extern void xform_6030(void *, void *, void *) __asm__("func_001F9EC0");
extern int mk_6030(void *, float, float, float, float, float) __asm__("func_L00_0023F0D0");
extern void rel_6030(int) __asm__("func_L00_0023F1D0");
extern float len2_6030(void *) __asm__("func_001F9CE8");
extern void norm2_6030(void *, void *) __asm__("func_L00_001FF328");
extern void setpos_6030(void *, int, float) __asm__("func_L00_001FFED8");
extern void special_6030(void *, float, float, float) __asm__("func_00215C00");
extern float rnd_6030(float, float) __asm__("func_002140F8");
extern float len3_6030(void *) __asm__("func_001F9CB8");
extern void scalev_6030(void *, void *, float) __asm__("func_L00_001FF4B0");
extern int blend_6030(int, int) __asm__("func_L00_00258BC8");
extern void emit_6030(float, void *, void *, int, int) __asm__("func_L00_0026EBC0");
extern void burst_6030(void *, void *, void *, int) __asm__("func_L09_00308F58");
extern void init_6030(void *) __asm__("func_L00_00251E30");
extern void l8_6030(void) __asm__("func_L08_00315068");
extern void f2EBE88_6030(void *) __asm__("func_L00_002EBE88");
extern void f2EBEE0_6030(void *) __asm__("func_L00_002EBEE0");
extern void fx_6030(void *, int, int, int, int, int, float, float, float) __asm__("func_L00_001EE2E0");

void func_L09_00306030(char *moby) {
    char *data = *(char **)(moby + 0x78);
    float v0[4];     /* sp+0x00 */
    float a20[4];    /* sp+0x20 */
    float a30[4];    /* sp+0x30 */
    float a40[4];    /* sp+0x40 */
    float a50[4];    /* sp+0x50 */
    float a60[4];
    float t10[4];    /* sp+0x10 */
    char *hero = (char *)D_0013E633_6030 + 0xE1D;   /* 0x13F450 */
    int state;
    short *timer52;
    int k;

    *(unsigned short *)(moby + 0x34) &= 0xFFFB;
    vecof_6030(moby, 3, v0);
    v0[2] = v0[2] + 0.4f;

    if (*(unsigned char *)(moby + 0x20) >= 4) {
        int r = dec_6030((short *)(data + 0x42));
        int main_branch = 0;
        if (r != 0 && *(unsigned char *)(moby + 0xBC) == 1) {
            if (!(*(int *)(hero + 0x208C) == 0x14 || *(int *)(hero + 0x208C) == 7 ||
                  *(int *)(hero + 0x2084) == 0x3D)) {
                main_branch = 1;
            }
        }
        if (main_branch) {
            int *s, *d;
            float *hp = (float *)(hero + 0x80);
            float f20 = -0.7f;
            s = (int *)v0; d = (int *)(hero + 0x80);
            d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3];
            s = (int *)(moby + 0x40); d = (int *)(hero + 0x90);
            d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3];
            *(float *)(hero + 0x98) = addrot_6030(1.5707964f, *(float *)(hero + 0x98));
            t10[0] = cos_6030(*(float *)(hero + 0x98)) * f20;
            t10[1] = sin_6030(*(float *)(hero + 0x98)) * f20;
            *(int *)&t10[2] = 0;
            vadd_6030(hp, hp, t10);
            clear_6030(hero + 0xE0, 0x90);
            if ((*(unsigned int *)(D_0013A5E0_6030 + 0x2604) & 0x10) != 0) {
                cleanup_6030(0);
                snd_6030(0, 1);
                *(unsigned char *)(moby + 0xBC) = 2;
                *(unsigned char *)(moby + 0x20) = 4;
                animate_6030(moby, 0, 0, 1);
            } else {
                if (*(unsigned char *)(moby + 0x20) != 5) {
                    *(short *)(data + 0x52) = 2;
                    f2EBF50_6030(moby + 0x10, moby + 0x40, 0, 0, 0);
                    if (*(unsigned char *)(moby + 0x53) != 5) {
                        animate_6030(moby, 5, 0, scale_6030(5));
                    }
                }
                *(unsigned char *)(moby + 0x20) = 5;
            }
        } else if (*(unsigned char *)(moby + 0xBC) == 2) {
            if (1.2f < dist_6030(v0, *(char **)(hero + 0x2080) + 0x10) ||
                *(char **)(hero + 0x2FC) != moby) {
                *(unsigned char *)(moby + 0xBC) = 0;
                *(short *)(data + 0x42) = scale_6030(60);
            }
        } else {
            if (!(*(int *)(hero + 0x208C) == 0x14 || *(int *)(hero + 0x208C) == 7)) {
                if (*(int *)(hero + 0x2084) != 0x3D && *(char **)(hero + 0x2FC) == moby) {
                    if (dist_6030(v0, *(char **)(hero + 0x2080) + 0x10) < 1.2f) {
                        if (*(short *)(data + 0x42) == 0) {
                            *(unsigned char *)(moby + 0xBC) = 1;
                            snd_6030(0x32, 1);
                        }
                    }
                }
            }
        }
    }

    state = *(unsigned char *)(moby + 0x20);
    if (state == 0) {
        int *s = (int *)(moby + 0x10);
        int *d = (int *)a20;
        setdata_6030(moby, 0, data);
        d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3];
        f2F10F8_6030(moby, a20);
        *(float *)(data + 0x5C) = *(float *)(moby + 0x18);
        *(unsigned char *)(moby + 0x20) = 4;
        *(int *)(data + 0x44) = 0;
        *(int *)(moby + 0x48) = 0;
        *(int *)(data + 0x68) = -1;
        *(short *)(data + 0x40) = 0;
        *(short *)(data + 0x42) = 0;
        *(int *)(data + 0x48) = 0;
        *(int *)(data + 0x4C) = 0;
        *(char **)(D_0013E633_6030 + 0x89) = moby;
        return;
    }
    if (state == 4) {
        float *hpos = (float *)(D_0013E633_6030 + 0xE9D);
        float d = dist2_6030(moby + 0x10, hpos);
        if (d < 4.0f || *(char **)(D_0013E633_6030 + 0x1119) == moby) {
            float f1 = *(float *)(moby + 0x18);
            float f0 = *(float *)(data + 0x5C) - 1.0f;
            float ang;
            float py = *(float *)(moby + 0x14), px = *(float *)(moby + 0x10);
            int idx;
            f0 = f0 - f1;
            f0 = f0 * 0.11f;
            f1 = f1 + f0;
            *(float *)(moby + 0x18) = f1;
            ang = atan2_6030(hpos[0] - px, hpos[1] - py);
            ang = approach_6030(ang, *(float *)(moby + 0x48));
            ang = addrot_6030(ang, 1.5707964f);
            ang = addrot_6030(*(float *)(data + 0x60), ang * 0.01f);
            *(float *)(data + 0x60) = ang * 0.9f;
            *(float *)(moby + 0x48) = addrot_6030(*(float *)(moby + 0x48), *(float *)(data + 0x60));
            xform_6030(a20, D_L09_00161D40_6030, moby + 0xC0);
            vadd_6030(a20, a20, v0);
            idx = *(int *)(data + 0x68);
            if (idx == -1) {
                *(int *)(data + 0x6C) = 0;
                *(int *)(data + 0x68) = mk_6030(a20, 0.4f, 0.0f, 4.0f, 4.0f, 4.0f);
                voice_6030(1, 0, moby);
                return;
            } else {
                char *rec = (char *)D_L09_001804C0_6030 + (idx << 5);
                int n;
                *(float *)(rec + 0x10) = a20[0];
                *(float *)(rec + 0x14) = a20[1];
                *(float *)(rec + 0x18) = a20[2];
                n = *(int *)(data + 0x6C) + 1;
                *(int *)(data + 0x6C) = n;
                if (n < 0x29 && (n & 4) != 0) {
                    if (n < 0x19) {
                        *(int *)(rec + 0x1C) = 0;
                        return;
                    }
                    if ((n & 0xA) != 0xA) {
                        *(int *)(rec + 0x1C) = 0;
                        return;
                    }
                }
                *(float *)(rec + 0x0) = 4.0f;
                *(float *)(rec + 0x1C) = 0.4f;
                return;
            }
        } else {
            float f1 = *(float *)(moby + 0x18);
            float f0 = *(float *)(data + 0x5C);
            if (f1 < f0) {
                *(float *)(moby + 0x18) = f1 + D_0015EE6C_6030;
            } else {
                *(float *)(moby + 0x18) = f0;
            }
            if (*(char **)(D_0013E633_6030 + 0x1119) != moby && *(int *)(data + 0x68) != -1) {
                voice_6030(2, 0, moby);
                rel_6030(*(int *)(data + 0x68));
                *(int *)(data + 0x68) = -1;
            }
            return;
        }
    }
    if (state != 5) {
        return;
    }

    /* state 5 */
    *(char **)(D_0013E633_6030 + 0x89) = moby;
    {
        float f1 = *(float *)(moby + 0x18);
        float f0 = *(float *)(data + 0x5C);
        float lx, ly, len, sx;
        float ft2, nv;
        if (f1 < f0) {
            *(float *)(moby + 0x18) = f1 + D_0015EE6C_6030;
        } else {
            *(float *)(moby + 0x18) = f0;
        }
        a20[0] = *(float *)(hero + 0x1D20);
        ly = *(float *)(hero + 0x1D24);
        if (*(int *)(D_0015EED0_6030 + 0xC) != 0) {
            a20[1] = -ly;
        } else {
            a20[1] = ly;
        }
        len = len2_6030(a20);
        if (1.0f < len) {
            norm2_6030(a20, a20);
        }
        ft2 = D_0015EE70_6030;
        sx = a20[0];
        *(float *)(data + 0x60) = *(float *)(data + 0x60) + (ft2 * 3.1415927f) * (-sx);
        *(float *)(data + 0x64) = *(float *)(data + 0x64) + (ft2 * 0.7853982f) * (-a20[1]);
        *(float *)(moby + 0x48) = addrot_6030(*(float *)(moby + 0x48), *(float *)(data + 0x60));
        nv = addrot_6030(*(float *)(data + 0x44), *(float *)(data + 0x64));
        *(float *)(data + 0x44) = nv;
        if (nv < -0.5235988f) {
            *(float *)(data + 0x44) = -0.5235988f;
            *(int *)(data + 0x64) = 0;
        } else if (0.5235988f < nv) {
            *(float *)(data + 0x44) = 0.5235988f;
            *(int *)(data + 0x64) = 0;
        }
        {
            float c = *(float *)(data + 0x64) * 0.97f;
            float d2 = *(float *)(data + 0x60) * 0.97f;
            *(float *)(data + 0x64) = c;
            *(float *)(data + 0x60) = d2;
            setpos_6030(data + 0x10, 1, -*(float *)(data + 0x44));
        }
    }
    {
        unsigned int w0 = *(unsigned int *)(D_0013A5E0_6030 + 0x2600);
        unsigned int w1 = *(unsigned int *)(D_0013A5E0_6030 + 0x2604);
        timer52 = (short *)(data + 0x52);
        if ((w0 & 0x20) == 0) {
            if (*(unsigned char *)(moby + 0x53) != 5) {
                if (*(unsigned char *)(moby + 0x70) & 2) {
                    animate_6030(moby, 5, 0, 1);
                }
            }
        } else if (w1 & 0x20) {
            animate_6030(moby, *(short *)(data + 0x54) != 0 ? 3 : 4, 0, 1);
        } else {
            int a = *(unsigned char *)(moby + 0x52);
            if (a == 3 || a == 4) {
                float f22 = 1.0f;
                float g = absf_6030(getf_6030(moby) - f22);
                if (g <= 0.25f) {
                    float f20 = D_0015EE6C_6030 * 60.0f;
                    float ang;
                    char *p;
                    int i;
                    ang = addrot_6030(1.5707964f, *(float *)(moby + 0x48));
                    special_6030(a30, f20, ang, *(float *)(data + 0x44));
                    vecof_6030(moby, a == 3 ? 1 : 2, a40);
                    burst_6030(moby, a40, a30, scale_6030(90));
                    voice_6030(0, 0, moby);
                    p = (char *)D_L09_00166E80_6030;
                    *(float *)(p + 0x160) = 0.03f;
                    *(int *)(p + 0x168) = scale_6030(10);
                    *(float *)(p + 0x170) = 0.03f;
                    *(int *)(p + 0x178) = scale_6030(7);
                    *(short *)(data + 0x54) = (a == 3) ? 0 : 1;
                    for (i = 4; i >= 0; i--) {
                        int c1, c2;
                        float rr, mulf;
                        a60[0] = a60[1] = a60[2] = a60[3] = 0.0f;
                        a60[0] = rnd_6030(-1.0f, 1.0f);
                        a60[1] = rnd_6030(-1.0f, 1.0f);
                        a60[2] = rnd_6030(-1.0f, 1.0f);
                        ((int *)a50)[0] = ((int *)a60)[0]; ((int *)a50)[1] = ((int *)a60)[1];
                        ((int *)a50)[2] = ((int *)a60)[2]; ((int *)a50)[3] = ((int *)a60)[3];
                        rr = len3_6030(a30);
                        mulf = (D_0015EE60_6030 * -0.35f + f22);
                        scalev_6030(a50, a50, rr * mulf);
                        vadd_6030(a50, a50, a30);
                        rr = rnd_6030(D_0015EE6C_6030 * 3.0f, D_0015EE6C_6030 * 6.0f);
                        scalev_6030(a50, a50, rr);
                        c1 = scale_6030(5);
                        c2 = scale_6030(10);
                        emit_6030(50000.0f, a40, a50, 0x5F2F4F6F, blend_6030(c1, c2));
                    }
                    {
                        int c1, c2;
                        scalev_6030(a30, a30, D_0015EE6C_6030 * 0.5f);
                        c1 = scale_6030(4);
                        c2 = scale_6030(7);
                        emit_6030(500000.0f, a40, a30, 0x2F4F7F7F, blend_6030(c1, c2));
                    }
                    (void)p;
                } else if (*(unsigned char *)(moby + 0x70) & 2) {
                    animate_6030(moby, a == 3 ? 4 : 3, 0, 1);
                }
            }
        }
        if (dec_6030(timer52) != 0) {
            int *s;
            init_6030(moby);
            *(unsigned short *)(moby + 0x34) |= 4;
            l8_6030();
            vecof_6030(moby, 0, a20);
            special_6030(a30, D_L09_00161D30_6030, addrot_6030(1.5707964f, *(float *)(moby + 0x48)),
                         *(float *)(data + 0x44));
            a30[2] = a30[2] + D_L09_00161D34_6030;
            vadd_6030(a40, a20, a30);
            f2EBE88_6030(a40);
            *(int *)&a50[0] = 0;
            a50[1] = -*(float *)(data + 0x44);
            a50[2] = addrot_6030(1.5707964f, *(float *)(moby + 0x48));
            f2EBEE0_6030(a50);
            (void)s;
        }
        fx_6030(moby, 0xFF0F0FFF, 0, 0xD, -1, 4, 1.0f, 0.0f, 90.0f);
    }
}
