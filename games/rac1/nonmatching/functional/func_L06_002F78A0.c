/* functional: medium. Blarg ring-gate moby update: state 0 initialises; state 1 spins it (func_L06_002F8430), probes two collision swept segments for the hero, bumps a hit counter, and when all four child mobys are done switches to state 2 (releases voice, records best time/flags); state 2 spawns 9 x 20 sparks and goes to state 3; state 3 removes it. Not runnable in capture; checked statically/by reading only. */
extern unsigned char D_0013E633_78A0[] __asm__("D_0013E633");
extern unsigned char D_0014171B_78A0[] __asm__("D_0014171B");
extern float D_0015EE6C_78A0 __asm__("D_0015EE6C");
extern int D_0015EE84_78A0 __asm__("D_0015EE84");
extern int D_0015EFA4_78A0 __asm__("D_0015EFA4");
extern char *D_L06_00160058_78A0 __asm__("D_L06_00160058");
extern int D_L06_001746D8_78A0 __asm__("D_L06_001746D8");
extern float D_L06_00167640_78A0[] __asm__("D_L06_00167640");
extern int D_L06_00161ED0_78A0 __asm__("D_L06_00161ED0");
extern int D_L06_00161ED4_78A0 __asm__("D_L06_00161ED4");
extern int D_L06_00161ED8_78A0 __asm__("D_L06_00161ED8");
extern int D_L06_00161EDC_78A0 __asm__("D_L06_00161EDC");
extern int D_L06_00161EE0_78A0 __asm__("D_L06_00161EE0");
extern int D_L06_00161EE4_78A0 __asm__("D_L06_00161EE4");
extern float D_L06_00161E98_78A0 __asm__("D_L06_00161E98");
extern float D_L06_00161E9C_78A0 __asm__("D_L06_00161E9C");
extern float D_L06_00161EA8_78A0 __asm__("D_L06_00161EA8");
extern int D_L06_00161EAC_78A0 __asm__("D_L06_00161EAC");
extern int D_L06_00161EB0_78A0 __asm__("D_L06_00161EB0");
extern float D_L06_00161EB4_78A0 __asm__("D_L06_00161EB4");
extern float D_L06_00161EB8_78A0 __asm__("D_L06_00161EB8");
extern int D_L06_00161EBC_78A0 __asm__("D_L06_00161EBC");
extern int D_L06_00161EC0_78A0 __asm__("D_L06_00161EC0");
extern float D_L06_00161EC4_78A0 __asm__("D_L06_00161EC4");
extern float D_L06_00161EC8_78A0 __asm__("D_L06_00161EC8");
extern float D_L06_001DB320_78A0[] __asm__("D_L06_001DB320");
extern float D_L06_001DB3B0_78A0[] __asm__("D_L06_001DB3B0");
extern void dec_78A0(short *) __asm__("func_001F9938");
extern int chkvoice_78A0(void *, int) __asm__("func_L00_0028EB98");
extern int allocvoice_78A0(int, int, void *) __asm__("func_0022ED80");
extern void relvoice_78A0(int) __asm__("func_L00_0028EBF0");
extern void spin_78A0(void *) __asm__("func_L06_002F8430");
extern float dist_78A0(void *, void *) __asm__("func_001F9D10");
extern void approach_78A0(float *, float, float) __asm__("func_00214D28");
extern float cos_78A0(float) __asm__("func_001F9F90");
extern float sin_78A0(float) __asm__("func_001F9FA8");
extern float addrot_78A0(float, float) __asm__("func_001FA748");
extern void vadd_78A0(void *, void *, void *) __asm__("func_001F9BD8");
extern void vsub_78A0(void *, void *, void *) __asm__("func_001F9BF0");
extern float dot_78A0(void *, void *) __asm__("func_001F9C78");
extern void collsetup_78A0(void *, void *, int, float, void *) __asm__("func_L00_0025A8C0");
extern void sweep_78A0(void *, void *, int, void *, void *) __asm__("func_L00_001EFFF0");
extern int scale_78A0(int) __asm__("func_001F9850");
extern void norm_78A0(void *, void *, float) __asm__("func_L00_001FF4B0");
extern void resp_78A0(void *, void *) __asm__("func_L00_0025AAC0");
extern void setupd_78A0(void *, void *) __asm__("func_001F49B0");
extern void upd_78A0(void *) __asm__("func_L06_002F8200");
extern int collchk_78A0(void *, int, int) __asm__("func_L00_0025B478");
extern void sound_78A0(int, int) __asm__("func_L00_00203F20");
extern void rotm_78A0(void *, void *) __asm__("func_001FA218");
extern void rotmy_78A0(void *, float) __asm__("func_001FA1C0");
extern void mulm_78A0(void *, void *, void *) __asm__("func_001FA540");
extern void xformv_78A0(void *, void *, void *) __asm__("func_001F9EE8");
extern float rndb_78A0(float, float) __asm__("func_L00_00258C80");
extern float rndb2_78A0(float, float) __asm__("func_002140F8");
extern int f2i_78A0(float) __asm__("func_001FA898");
extern unsigned int tween_78A0(unsigned int, unsigned int, float) __asm__("func_001FA8A8");
extern void rndvec_78A0(void *, float, float) __asm__("func_L00_00258DB0");
extern int rndi_78A0(int) __asm__("func_002140B0");
extern void spark_78A0(void *, void *, int, float, int, int, int) __asm__("func_L00_00273F80");
extern void remove_78A0(void *) __asm__("func_0020D678");

void func_L06_002F78A0(char *moby) {
    char *data = *(char **)(moby + 0x78);
    unsigned char *tab = D_0014171B_78A0 + 0x34D;   /* record: +0x158 u16 count, +0x15A u16 best, +0x15C flags */
    char *hero = (char *)D_0013E633_78A0;
    float m[16];
    float loc[16];
    int i;

    switch (*(unsigned char *)(moby + 0x20)) {
    case 0:
        D_L06_00161ED4_78A0 = 11;
        D_L06_00161ED8_78A0 = 23;
        D_L06_00161EDC_78A0 = 34;
        D_L06_00161ED0_78A0 = 0;
        D_L06_00161EE4_78A0 = 22;
        D_L06_00161EE0_78A0 = 45;
        *(int *)(data + 0x10) = -1;
        *(unsigned char *)(moby + 0x20) = 1;
        *(unsigned short *)(moby + 0x34) |= 0x4000;
        return;
    case 1: {
        float *pos = (float *)(moby + 0x10);
        float d, ang;
        int allDone, allSame;

        dec_78A0((short *)(data + 0x16));
        if (chkvoice_78A0(moby, *(int *)(data + 0x10)) == 0) {
            *(int *)(data + 0x10) = allocvoice_78A0(0, 4, moby);
        }
        spin_78A0(moby);
        d = dist_78A0(pos, D_L06_00167640_78A0);
        if (d < 48.0f) {
            int k;
            approach_78A0((float *)(data + 0x18), 1.0f, D_0015EE6C_78A0 + D_0015EE6C_78A0);
            for (k = 0; k < 2; k++) {
                int *s, *t;
                ang = *(float *)(moby + 0x48);
                if (k == 0) {
                    m[8] = cos_78A0(ang) * 0.5f;
                    m[9] = sin_78A0(ang) * 0.5f;
                } else {
                    m[8] = cos_78A0(addrot_78A0(ang, 3.1415927f)) * 0.5f;
                    m[9] = sin_78A0(addrot_78A0(*(float *)(moby + 0x48), 3.1415927f)) * 0.5f;
                }
                m[10] = 0.0f;
                m[12] = cos_78A0(addrot_78A0(*(float *)(moby + 0x48), 1.5707964f)) * 3.0f;
                m[13] = sin_78A0(addrot_78A0(*(float *)(moby + 0x48), 1.5707964f)) * 3.0f;
                m[14] = 0.0f;
                s = (int *)pos; t = (int *)&m[0];
                t[0] = s[0]; t[1] = s[1]; t[2] = s[2]; t[3] = s[3];
                m[2] = m[2] - D_L06_00161EA8_78A0;
                s = (int *)&m[0]; t = (int *)&m[4];
                t[0] = s[0]; t[1] = s[1]; t[2] = s[2]; t[3] = s[3];
                vadd_78A0(&m[0], &m[0], &m[8]);
                vadd_78A0(&m[4], &m[4], &m[8]);
                vadd_78A0(&m[0], &m[0], &m[12]);
                vsub_78A0(&m[4], &m[4], &m[12]);
                collsetup_78A0(loc, moby, 0x10001, 5.0f, &m[8]);
                sweep_78A0(&m[0], &m[4], 0, moby, loc);
                if (*(unsigned short *)(tab + 0x158) == 0 && *(short *)(data + 0x16) == 0 &&
                    D_L06_001746D8_78A0 == *(int *)(hero + 0x2E9D)) {
                    *(short *)(data + 0x14) = *(short *)(data + 0x14) + 1;
                    *(short *)(data + 0x16) = scale_78A0(30);
                }
            }
            if (*(char **)(hero + 0x1059) == moby) {
                vsub_78A0(&loc[12], hero + 0xE9D, pos);
                if (dot_78A0(&loc[12], moby + 0xC0) > 0.0f) {
                    norm_78A0(loc, moby + 0xC0, 1.0f);
                } else {
                    norm_78A0(loc, moby + 0xC0, -1.0f);
                }
                resp_78A0(*(void **)(hero + 0x2E9D), loc);
                if (*(unsigned short *)(tab + 0x158) == 0 && *(short *)(data + 0x16) == 0) {
                    *(short *)(data + 0x14) = *(short *)(data + 0x14) + 1;
                    *(short *)(data + 0x16) = scale_78A0(30);
                }
            }
        } else {
            approach_78A0((float *)(data + 0x18), 0.0f, D_0015EE6C_78A0 + D_0015EE6C_78A0);
        }
        if (*(float *)(data + 0x18) != 0.0f) {
            setupd_78A0(upd_78A0, moby);
        }
        if (*(unsigned short *)(tab + 0x158) == 0) {
            if (*(short *)(data + 0x16) == 0) {
                if (collchk_78A0(moby, 0x330000, 0) != 0) {
                    *(short *)(data + 0x14) = *(short *)(data + 0x14) + 1;
                    *(short *)(data + 0x16) = scale_78A0(30);
                }
            }
            *(unsigned char *)(moby + 0xA4) = 255;
            if (*(short *)(data + 0x14) >= 3) {
                sound_78A0(0x1774, 0x2B);
            }
        }
        allDone = 1;
        allSame = 1;
        for (i = 0; i < 4; i++) {
            int idx = ((int *)data)[i];
            if (idx >= 0) {
                char *child = D_L06_00160058_78A0 + (idx << 8);
                if (*(short *)(child + 0xA6) == 0x516) {
                    if (*(unsigned char *)(child + 0x20) == 2) {
                        allSame = 0;
                    } else {
                        allDone = 0;
                    }
                } else if (*(unsigned char *)(child + 0xBC) == 0) {
                    allDone = 0;
                }
            }
        }
        if (allDone) {
            int voice, frame, color;
            *(int *)(moby + 0x94) = 0;
            *(unsigned char *)(moby + 0x20) = 2;
            voice = *(int *)(data + 0x10);
            if (voice != -1) {
                char *v = (char *)D_0013E633_78A0 + 0x1D + voice * 0x70;
                if (*(char **)(v + 0x88) == moby && *(unsigned char *)(v + 0x74) != 0) {
                    relvoice_78A0(voice);
                }
            }
            *(int *)(data + 0x10) = -1;
            allocvoice_78A0(1, 0, moby);
            if (allSame != 0 && *(unsigned short *)(tab + 0x158) <= 0xFFFE) {
                *(unsigned short *)(tab + 0x158) = *(unsigned short *)(tab + 0x158) + 1;
            }
            frame = scale_78A0(D_0015EFA4_78A0) / 600;
            if ((int)*(unsigned short *)(tab + 0x15A) < frame) {
                *(unsigned short *)(tab + 0x15A) = scale_78A0(D_0015EFA4_78A0) / 600;
            }
            color = D_0015EE84_78A0;
            *(int *)(tab + 0x15C) = *(int *)(tab + 0x15C) | (1 << color) | 0x80000000;
        }
        return;
    }
    case 2: {
        float *pos = (float *)(moby + 0x10);
        float *delta = &loc[12];
        float at[4], off[4], eff[4];
        int idx, j;

        rotm_78A0(m, moby + 0x40);
        rotmy_78A0(loc, D_L06_00161E9C_78A0);
        mulm_78A0(m, m, loc);
        for (idx = 0; idx < 9;) {
            int o = idx * 4;
            rotm_78A0(loc, D_L06_001DB3B0_78A0 + o);
            idx++;
            mulm_78A0(loc, m, loc);
            xformv_78A0(at, D_L06_001DB320_78A0 + o, m);
            vadd_78A0(delta, at, pos);
            delta[3] = 1.0f;
            for (j = 19; j >= 0; j--) {
                float amt, scl, col;
                int color, frame;
                amt = rndb_78A0(0.0f, D_L06_00161E98_78A0 * 0.5f);
                norm_78A0(off, &loc[4], amt);
                vadd_78A0(off, off, delta);
                col = rndb2_78A0(0.0f, 1.0f);
                color = tween_78A0(D_L06_00161EAC_78A0, D_L06_00161EB0_78A0, col);
                scl = rndb2_78A0(D_L06_00161EB4_78A0, D_L06_00161EB8_78A0);
                frame = f2i_78A0(rndb2_78A0((float)D_L06_00161EBC_78A0, (float)D_L06_00161EC0_78A0));
                rndvec_78A0(eff, D_L06_00161EC4_78A0 * D_0015EE6C_78A0, D_L06_00161EC8_78A0 * D_0015EE6C_78A0);
                frame = scale_78A0(frame) & 0xFF;
                spark_78A0(off, eff, color, scl, frame, rndi_78A0(255) & 0xFF, 0);
            }
        }
        *(unsigned char *)(moby + 0x20) = 3;
        return;
    }
    case 3:
        remove_78A0(moby);
        return;
    }
}
