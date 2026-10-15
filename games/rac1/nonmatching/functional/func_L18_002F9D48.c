/* functional: medium. Veldin (return) cutscene driver moby (state byte 0x20: 0 sets the timeline globals, 1 runs): per frame of the sequence state at D_L18_0016D2E0 (state 4 fireworks on objects 3..7 and bursts at frames 706..748 plus fade/ramp callbacks, state 7 the same ramps from unscaled values, state 5 smooths two tracked points, state 1 aims the lasers of object 4, state 3 emits a ring of 32 effects at frame 586). Same function as the NTSC decomp's FUN_L18_002f88e8, re-read against the retail assembly; not checked on real calls. */
typedef int U128_9D48 __attribute__((mode(TI)));
typedef struct { float radius[5]; } Radii_9D48;
extern char D_L18_0016D2E0_9D48[] __asm__("D_L18_0016D2E0");
extern int D_L18_0015F6A8_9D48 __asm__("D_L18_0015F6A8");
extern char *D_L18_00160064_9D48 __asm__("D_L18_00160064");
extern int D_L18_00162528_9D48 __asm__("D_L18_00162528") __attribute__((section(".sdata")));
extern int D_L18_0016252C_9D48 __asm__("D_L18_0016252C") __attribute__((section(".sdata")));
extern int D_L18_00162530_9D48 __asm__("D_L18_00162530") __attribute__((section(".sdata")));
extern int D_L18_00162534_9D48 __asm__("D_L18_00162534") __attribute__((section(".sdata")));
extern int D_L18_00162538_9D48 __asm__("D_L18_00162538") __attribute__((section(".sdata")));
extern int D_L18_0016253C_9D48 __asm__("D_L18_0016253C") __attribute__((section(".sdata")));
extern int D_L18_00162540_9D48 __asm__("D_L18_00162540") __attribute__((section(".sdata")));
extern int D_L18_00162544_9D48 __asm__("D_L18_00162544") __attribute__((section(".sdata")));
extern int D_L18_00162548_9D48 __asm__("D_L18_00162548") __attribute__((section(".sdata")));
extern int D_L18_0016254C_9D48 __asm__("D_L18_0016254C") __attribute__((section(".sdata")));
extern int D_L18_00162550_9D48 __asm__("D_L18_00162550") __attribute__((section(".sdata")));
extern int D_L18_00162554_9D48 __asm__("D_L18_00162554") __attribute__((section(".sdata")));
extern int D_L18_00162558_9D48 __asm__("D_L18_00162558") __attribute__((section(".sdata")));
extern float D_L18_00162520_9D48 __asm__("D_L18_00162520") __attribute__((section(".sdata")));
extern float D_L18_00162524_9D48 __asm__("D_L18_00162524") __attribute__((section(".sdata")));
extern float D_L18_00162560_9D48 __asm__("D_L18_00162560") __attribute__((section(".sdata")));
extern char D_L18_00162570_9D48[] __asm__("D_L18_00162570") __attribute__((section(".sdata")));
extern float D_L18_00162580_9D48 __asm__("D_L18_00162580") __attribute__((section(".sdata")));
extern float D_L18_00162584_9D48 __asm__("D_L18_00162584") __attribute__((section(".sdata")));
extern float D_L18_001625BC_9D48 __asm__("D_L18_001625BC") __attribute__((section(".sdata")));
extern char *D_L18_001625B8_9D48 __asm__("D_L18_001625B8");
extern int D_L18_001625B4_9D48 __asm__("D_L18_001625B4");
extern float D_L18_001625B0_9D48 __asm__("D_L18_001625B0");
extern char D_L18_001625C0_9D48[] __asm__("D_L18_001625C0");
extern char D_L18_001625D0_9D48[] __asm__("D_L18_001625D0");
extern char D_L18_0015F660_9D48[] __asm__("D_L18_0015F660");
extern Radii_9D48 D_L18_001F2F10_9D48 __asm__("D_L18_001F2F10");
extern char D_L18_00167A50_9D48[] __asm__("D_L18_00167A50");
extern float D_0015EE60_9D48 __asm__("D_0015EE60");
extern float D_L18_001617DC_9D48 __asm__("D_L18_001617DC");
extern int D_L18_00161884_9D48 __asm__("D_L18_00161884");
extern int D_L18_00161888_9D48 __asm__("D_L18_00161888");
extern int fr_9D48(int) __asm__("func_001F9850");
extern float i2f_9D48(int) __asm__("func_001FA888");
extern int f2i_9D48(float) __asm__("func_001FA898");
extern void mpos_9D48(void *, int, void *) __asm__("func_L00_00250800");
extern void clr_9D48(void *) __asm__("func_001F9BC0");
extern void spawn_9D48(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float,
                       int, int, int, int) __asm__("func_L00_0025F4A8");
extern void enq_9D48(void *, void *) __asm__("func_001F49B0");
extern void blast_9D48(void *, void *, void *, float, float) __asm__("func_L00_00264BE8");
extern void lerp_9D48(void *, void *, void *, float) __asm__("func_001F9C08");
extern void blast2_9D48(void *, void *, void *, float, float) __asm__("func_L18_002FB6B0");
extern float atan2_9D48(float, float) __asm__("func_L00_001FF860");
extern float len2_9D48(void *) __asm__("func_001F9CE8");
extern void laser_9D48(void *) __asm__("func_L00_002BE3A8");
extern void rot_9D48(void *, void *, void *) __asm__("func_001F9EC0");
extern void add_9D48(void *, void *, void *) __asm__("func_001F9BD8");
extern void sub_9D48(void *, void *, void *) __asm__("func_001F9BF0");
extern void norm_9D48(void *, void *, float) __asm__("func_L00_001FF4B0");
extern void axis_9D48(void *, void *, void *, float) __asm__("func_002156E0");
extern int rnd_9D48(int) __asm__("func_002140B0");
extern char *mkfx_9D48(void *, int, void *, float, float, float, float, int) __asm__("func_L00_0026DEA0");
extern void cbA_9D48(void) __asm__("func_L18_002FB080");
extern void cbB_9D48(void) __asm__("func_L18_002FB318");
extern void cbC_9D48(void) __asm__("func_L18_002FABE0");
extern void cbD_9D48(void) __asm__("func_L18_002FAEF8");

void func_L18_002F9D48_impl(char *moby) __asm__("func_L18_002F9D48");
void func_L18_002F9D48_impl(char *moby) {
    float v_pos[4] __attribute__((aligned(16)));
    float v_vel[4] __attribute__((aligned(16)));
    float v_dir[4] __attribute__((aligned(16)));
    float v_a[4] __attribute__((aligned(16)));
    float v_b[4] __attribute__((aligned(16)));
    Radii_9D48 radii;
    char *S = D_L18_0016D2E0_9D48;
    char **obj = (char **)(S + 0x178);
    int i;
    float num, den, fraction, radius;
    char *target;
    char *effect;

    if (*(unsigned char *)(moby + 0x20) == 0) {
        D_L18_00162528_9D48 = 355;
        D_L18_0016254C_9D48 = 60;
        D_L18_00162550_9D48 = 200;
        D_L18_00162554_9D48 = 190;
        D_L18_00162558_9D48 = 365;
        D_L18_0016252C_9D48 = 365;
        *(unsigned char *)(moby + 0x20) = 1;
        *(unsigned char *)(moby + 0x30) = 255;
        return;
    }
    if (*(unsigned char *)(moby + 0x20) != 1)
        return;
    if (D_L18_0015F6A8_9D48 != 2)
        return;
    if (*(int *)(S + 0x30) == 4) {
        if (*(int *)(S + 0x34) == 1) {
            for (target = D_L18_00160064_9D48; target; target = *(char **)(target + 0x28)) {
                if (*(short *)(target + 0xA6) == 0x59B) {
                    D_L18_001625B8_9D48 = target;
                    break;
                }
            }
        }
        if (fr_9D48(500) <= *(int *)(S + 0x34) && *(int *)(S + 0x34) <= fr_9D48(860)) {
            radii = D_L18_001F2F10_9D48;
            for (i = 3; i < 8; i++) {
                if (obj[i]) {
                    mpos_9D48(obj[i], 0, v_dir);
                    radius = radii.radius[i - 3];
                    if (i != 3 || *(int *)(S + 0x34) < fr_9D48(645) || fr_9D48(736) < *(int *)(S + 0x34)) {
                        if (i == 5 && fr_9D48(645) <= *(int *)(S + 0x34) && *(int *)(S + 0x34) <= fr_9D48(730))
                            radius = 300000.0f;
                        blast_9D48(v_dir, v_dir, D_L18_0015F660_9D48, radius, radius * 0.4f);
                    }
                }
            }
        }
        if (*(int *)(S + 0x34) == fr_9D48(706)) {
            target = obj[7];
            if (target) {
                mpos_9D48(target, 0, v_pos);
                clr_9D48(v_vel);
                v_vel[2] = 1.0f;
                spawn_9D48(target, v_vel, v_pos, 0.0f, 0.0f, 50, 25, 25, 8.0f, 4.0f, 9.0f, 1.0f, -1, 30.0f, 0, 15, -1, 0);
            }
        }
        if (*(int *)(S + 0x34) == fr_9D48(710)) {
            target = obj[4];
            if (target) {
                mpos_9D48(target, 0, v_pos);
                clr_9D48(v_vel);
                v_vel[2] = 1.0f;
                spawn_9D48(target, v_vel, v_pos, 0.0f, 0.0f, 50, 25, 25, 8.0f, 4.0f, 9.0f, 1.0f, -1, 30.0f, 0, 15, -1, 0);
            }
        }
        if (*(int *)(S + 0x34) == fr_9D48(720)) {
            target = obj[6];
            if (target) {
                mpos_9D48(target, 0, v_pos);
                clr_9D48(v_vel);
                v_vel[2] = 1.0f;
                spawn_9D48(target, v_vel, v_pos, 0.0f, 0.0f, 50, 25, 25, 8.0f, 4.0f, 9.0f, 1.0f, -1, 30.0f, 0, 15, -1, 0);
            }
        }
        if (*(int *)(S + 0x34) == fr_9D48(730)) {
            target = obj[5];
            if (target) {
                mpos_9D48(target, 0, v_pos);
                clr_9D48(v_vel);
                v_vel[2] = 1.0f;
                spawn_9D48(target, v_vel, v_pos, 0.0f, 0.0f, 50, 25, 25, 8.0f, 4.0f, 9.0f, 1.0f, -1, 30.0f, 0, 15, -1, 0);
            }
        }
        if (*(int *)(S + 0x34) == fr_9D48(748)) {
            *(U128_9D48 *)v_pos = 0;
            v_pos[0] = 634.556030273f;
            v_pos[1] = 454.162994385f;
            v_pos[2] = 102.999000549f;
            v_pos[3] = 1.0f;
            clr_9D48(v_vel);
            v_vel[2] = 1.0f;
            spawn_9D48(moby, v_vel, v_pos, 0.0f, 0.0f, 300, 200, 200, 12.0f, 6.0f, 9.0f, 1.0f, -1, 60.0f, 0, 60, -1, 0);
        }
        if (fr_9D48(D_L18_0016254C_9D48) <= *(int *)(S + 0x34) && *(int *)(S + 0x34) < fr_9D48(D_L18_00162550_9D48)) {
            num = i2f_9D48(*(int *)(S + 0x34) - fr_9D48(D_L18_0016254C_9D48));
            den = i2f_9D48(fr_9D48(D_L18_00162550_9D48 - D_L18_0016254C_9D48));
            num = num / den;
            D_L18_001625B4_9D48 = f2i_9D48(num * 255.0f);
            enq_9D48(cbA_9D48, D_L18_001625B8_9D48);
        }
        if (fr_9D48(D_L18_00162554_9D48) <= *(int *)(S + 0x34) && *(int *)(S + 0x34) < fr_9D48(D_L18_00162558_9D48)) {
            num = i2f_9D48(*(int *)(S + 0x34) - fr_9D48(D_L18_00162554_9D48));
            den = i2f_9D48(fr_9D48(D_L18_00162558_9D48 - D_L18_00162554_9D48));
            num = num / den;
            num *= 200.0f;
            num += 0.25f;
            D_L18_001625BC_9D48 = num;
            enq_9D48(cbB_9D48, D_L18_001625B8_9D48);
        }
        if (fr_9D48(D_L18_00162528_9D48) <= *(int *)(S + 0x34) && *(int *)(S + 0x34) < fr_9D48(D_L18_00162530_9D48)) {
            enq_9D48(cbC_9D48, obj[0]);
            if (*(int *)(S + 0x34) >= fr_9D48(D_L18_0016252C_9D48)) {
                D_L18_001625B0_9D48 = 10.0f;
            } else {
                num = i2f_9D48(*(int *)(S + 0x34) - fr_9D48(D_L18_00162528_9D48));
                den = i2f_9D48(fr_9D48(D_L18_0016252C_9D48 - D_L18_00162528_9D48));
                num = num / den;
                D_L18_001625B0_9D48 = D_L18_00162520_9D48 + (D_L18_00162524_9D48 - D_L18_00162520_9D48) * num;
            }
        }
    }
    if (*(int *)(S + 0x30) == 7) {
        if (*(int *)(S + 0x34) == 1) {
            D_L18_0016254C_9D48 = D_L18_00162534_9D48;
            D_L18_00162550_9D48 = D_L18_00162538_9D48;
            D_L18_00162554_9D48 = D_L18_0016253C_9D48;
            D_L18_00162558_9D48 = D_L18_00162540_9D48;
            D_L18_00162528_9D48 = D_L18_00162544_9D48;
            D_L18_0016252C_9D48 = D_L18_00162548_9D48;
            for (target = D_L18_00160064_9D48; target; target = *(char **)(target + 0x28)) {
                if (*(short *)(target + 0xA6) == 0x59B) {
                    D_L18_001625B8_9D48 = target;
                    break;
                }
            }
        }
        if (fr_9D48(D_L18_0016254C_9D48) <= *(int *)(S + 0x34) && *(int *)(S + 0x34) < fr_9D48(D_L18_00162550_9D48)) {
            num = i2f_9D48(*(int *)(S + 0x34) - D_L18_0016254C_9D48);
            den = i2f_9D48(D_L18_00162550_9D48 - D_L18_0016254C_9D48);
            num = num / den;
            D_L18_001625B4_9D48 = f2i_9D48(num * 255.0f);
            enq_9D48(cbA_9D48, D_L18_001625B8_9D48);
        }
        if (fr_9D48(D_L18_00162554_9D48) <= *(int *)(S + 0x34) && *(int *)(S + 0x34) < fr_9D48(D_L18_00162558_9D48)) {
            num = i2f_9D48(*(int *)(S + 0x34) - D_L18_00162554_9D48);
            den = i2f_9D48(D_L18_00162558_9D48 - D_L18_00162554_9D48);
            num = num / den;
            num *= 200.0f;
            num += 0.25f;
            D_L18_001625BC_9D48 = num;
            enq_9D48(cbB_9D48, D_L18_001625B8_9D48);
        }
        if (fr_9D48(D_L18_00162528_9D48) <= *(int *)(S + 0x34) && *(int *)(S + 0x34) < fr_9D48(D_L18_00162530_9D48)) {
            enq_9D48(cbC_9D48, obj[0]);
            if (*(int *)(S + 0x34) >= D_L18_0016252C_9D48) {
                D_L18_001625B0_9D48 = 10.0f;
            } else {
                num = i2f_9D48(*(int *)(S + 0x34) - D_L18_00162528_9D48);
                den = i2f_9D48(D_L18_0016252C_9D48 - D_L18_00162528_9D48);
                num = num / den;
                D_L18_001625B0_9D48 = D_L18_00162520_9D48 + (D_L18_00162524_9D48 - D_L18_00162520_9D48) * num;
            }
        }
    }
    if (*(int *)(S + 0x30) == 1 && obj[3])
        enq_9D48(cbD_9D48, obj[3]);
    if (*(int *)(S + 0x30) == 5 && fr_9D48(138) <= *(int *)(S + 0x34) && *(int *)(S + 0x34) <= fr_9D48(264)) {
        target = obj[2];
        if (target) {
            if (fr_9D48(140) < *(int *)(S + 0x34)) {
                mpos_9D48(target, 0, v_pos);
                mpos_9D48(target, 2, v_vel);
                for (i = 0; i < 12; i++) {
                    fraction = i2f_9D48(i) / 12.0f;
                    lerp_9D48(v_a, v_pos, D_L18_001625C0_9D48, fraction);
                    lerp_9D48(v_b, v_vel, D_L18_001625D0_9D48, fraction);
                    blast2_9D48(v_a, v_a, D_L18_0015F660_9D48, D_L18_00162560_9D48, D_L18_00162560_9D48 * 0.4f);
                    blast2_9D48(v_b, v_b, D_L18_0015F660_9D48, D_L18_00162560_9D48, D_L18_00162560_9D48 * 0.4f);
                }
                *(U128_9D48 *)D_L18_001625C0_9D48 = *(U128_9D48 *)v_pos;
                *(U128_9D48 *)D_L18_001625D0_9D48 = *(U128_9D48 *)v_vel;
            } else {
                mpos_9D48(target, 0, D_L18_001625C0_9D48);
                mpos_9D48(target, 2, D_L18_001625D0_9D48);
            }
        }
    }
    if (*(int *)(S + 0x30) == 1 && fr_9D48(446) <= *(int *)(S + 0x34) && *(int *)(S + 0x34) < fr_9D48(502)) {
        target = obj[4];
        if (target) {
            float saved;
            int color = 0x207F7F;
            int color2 = 0x7F7F20;
            char *data = *(char **)(moby + 0x78);
            *(float *)(data + 0x10) = 0.0f;
            mpos_9D48(target, 3, v_pos);
            mpos_9D48(target, 10, v_vel);
            sub_9D48(v_dir, v_vel, v_pos);
            norm_9D48(moby + 0xD0, v_dir, 1.0f);
            *(U128_9D48 *)data = *(U128_9D48 *)v_vel;
            *(float *)(data + 0x48) = atan2_9D48(*(float *)(moby + 0xD0), *(float *)(moby + 0xD4));
            *(float *)(data + 0x4C) = atan2_9D48(len2_9D48(moby + 0xD0), *(float *)(moby + 0xD8));
            saved = D_L18_001617DC_9D48;
            D_L18_001617DC_9D48 = 24.0f;
            laser_9D48(moby);
            D_L18_001617DC_9D48 = saved;
            D_L18_00161884_9D48 = color;
            D_L18_00161888_9D48 = color2;
        }
    }
    if (*(int *)(S + 0x30) != 2 && *(int *)(S + 0x30) != 0) {
        if (*(int *)(S + 0x30) != 3)
            return;
        if (fr_9D48(586) <= *(int *)(S + 0x34))
            goto burst;
    }
    if (obj[2])
        enq_9D48(cbD_9D48, obj[2]);
burst:
    if (*(int *)(S + 0x30) == 3 && *(int *)(S + 0x34) == fr_9D48(586)) {
        char *matrix = D_L18_00167A50_9D48;
        rot_9D48(v_pos, D_L18_00162570_9D48, matrix);
        add_9D48(v_pos, v_pos, matrix - 0x210);
        *(U128_9D48 *)v_vel = *(U128_9D48 *)(matrix + 0x10);
        for (i = 0; i < 32; i++) {
            int n;
            axis_9D48(v_dir, v_vel, matrix, i2f_9D48(i) * 0.19634955f);
            norm_9D48(v_dir, v_dir, D_L18_00162580_9D48 * D_0015EE60_9D48);
            n = rnd_9D48(16);
            if (rnd_9D48(2))
                n = -n;
            effect = mkfx_9D48(v_pos, n, v_dir, 0.0f, 1.0f, 1.01f, D_L18_00162584_9D48, 0x7F7F7F7F);
            if (effect) {
                char *fd = effect + 0x20;
                *(short *)(effect + 0xA) = fr_9D48(60);
                *(unsigned char *)(effect + 9) = f2i_9D48(4.0f) + 64;
                *(int *)(fd + 4) = 2;
                *(unsigned char *)(fd + 0xA) = 127;
                *(unsigned char *)(fd + 0xB) = *(unsigned char *)(effect + 0xA);
            }
        }
    }
}
