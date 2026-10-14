/* functional: medium. Update of the hoverboard girl NPC (class 918, level 5): runs the dialogue states (0 init, 1 talk with camera events, 3 after the talk, default camera reset), idles between two animations, looks at the hero (or a random point), counts the hero's high-jump / crouch moves into the growth counter (max 20) and drives the head and two chest joints. Not run against captured calls. */
extern char G_hero_17F80[] __asm__("D_0013E633");
extern char G_D605_17F80[] __asm__("D_0013D605");
extern char G_D355_17F80[] __asm__("D_0013D355");
extern char G_D50F_17F80[] __asm__("D_0013D50F");
extern char G_cam_17F80[] __asm__("D_L05_00215F00");
extern char G_cam2_17F80[] __asm__("D_L05_00215F20");
extern char G_saved_17F80[] __asm__("D_0013E633");
extern char *G_entries_17F80[] __asm__("D_L05_001B1E78");
extern char G_name_17F80[] __asm__("D_L05_00161F98");
extern int G_frame_17F80 __asm__("D_L05_0015F6B0");
extern short G_frame_gp_17F80 __asm__("D_L05_0015F6B0");
extern short G_mode_gp_17F80 __asm__("D_L05_0015F6A8");
extern short G_grow_gp_17F80 __asm__("D_L05_00161F90");
extern char G_bighead_17F80[] __asm__("D_0015EEB0");
extern float G_dt_17F80 __asm__("D_0015EE64");
extern short G_dt_gp_17F80 __asm__("D_0015EE64");

extern void func_L05_00317E58_17F80(void *) __asm__("func_L05_00317E58");
extern void func_L05_00317CD8_17F80(void *, void *) __asm__("func_L05_00317CD8");
extern int func_001F9850_17F80(int) __asm__("func_001F9850");
extern void func_00213DE0_17F80(void *, int, int, int) __asm__("func_00213DE0");
extern float func_00214358_17F80(void *, int, float) __asm__("func_00214358");
extern int func_L00_00267618_17F80(void *) __asm__("func_L00_00267618");
extern void func_L00_002676E8_17F80(void *, void *) __asm__("func_L00_002676E8");
extern void func_L02_0025D750_17F80(void *) __asm__("func_L02_0025D750");
extern int func_L00_00267290_17F80(void *, void *) __asm__("func_L00_00267290");
extern void func_L01_00279398_17F80(void *, float) __asm__("func_L01_00279398");
extern void func_L00_00217718_17F80(void *, void *, int, int) __asm__("func_L00_00217718");
extern void func_L00_002664B0_17F80(int, int) __asm__("func_L00_002664B0");
extern int func_001F9908_17F80(void *) __asm__("func_001F9908");
extern float func_002140F8_17F80(float, float) __asm__("func_002140F8");
extern float func_001F9878_17F80(float) __asm__("func_001F9878");
extern int func_001FA898_17F80(float) __asm__("func_001FA898");
extern int func_002140B0_17F80(int) __asm__("func_002140B0");
extern void func_L00_00264DB8_17F80(int, int) __asm__("func_L00_00264DB8");
extern void func_0022EE28_17F80(int, int, int) __asm__("func_0022EE28");
extern void func_0020BFC8_17F80(int, int) __asm__("func_0020BFC8");
extern float func_001F9D48_17F80(void *, void *) __asm__("func_001F9D48");
extern float func_L00_001FF860_17F80(float, float) __asm__("func_L00_001FF860");
extern float func_001FA850_17F80(float, float) __asm__("func_001FA850");
extern float func_001F9CB8_17F80(void *) __asm__("func_001F9CB8");
extern float func_001FA748_17F80(float, float) __asm__("func_001FA748");
extern void func_00215C00_17F80(void *, float, float, float) __asm__("func_00215C00");
extern void func_001F9BD8_17F80(void *, void *, void *) __asm__("func_001F9BD8");
extern void func_001F9BF0_17F80(void *, void *, void *) __asm__("func_001F9BF0");
extern float func_001FA790_17F80(float, float) __asm__("func_001FA790");
extern float func_001F9CE8_17F80(void *) __asm__("func_001F9CE8");
extern void func_L00_00263950_17F80(void *, void *, int, float, float) __asm__("func_L00_00263950");

void func_L05_00317F80_impl(char *moby) __asm__("func_L05_00317F80");
void func_L05_00317F80_impl(char *moby) {
    char *data = *(char **)(moby + 0x78);
    char *g = G_hero_17F80 + 0xE1D;
    char *hp = G_hero_17F80 + 0xE9D;
    float v0[4] __attribute__((aligned(16)));
    float v1[4] __attribute__((aligned(16)));
    float v2[4] __attribute__((aligned(16)));
    float rstep, rlim;
    int look;
    int s, n;
    float yaw, pitch, f;

    func_L05_00317E58_17F80(moby);
    s = *(unsigned char *)(moby + 0x20);
    if (s == 1) {
        if (*(int *)(g + 0x2084) != 0x1D) {
            if (func_L00_00267290_17F80(moby, data) != 0) {
                func_L01_00279398_17F80(moby, 2.0f);
                *(unsigned char *)(moby + 0x20) = 3;
            }
            if (*(short *)(data + 0x36) == 3) {
                *(unsigned short *)(data + 4) = *(unsigned short *)(data + 0x36);
                *(int *)(data + 0x38) = G_frame_17F80;
                *(short *)(data + 0x36) = 2;
                *(unsigned char *)(moby + 0xBC) = 0;
                func_L00_00217718_17F80(G_cam_17F80, G_cam_17F80 + 0x10, 0, 1);
                qcopy(g + 0x1D00, G_cam_17F80 + 0x20);
                qcopy(g + 0x1D10, G_cam_17F80 + 0x30);
                *(unsigned char *)(g + 0x20AC) = 1;
                func_L00_002664B0_17F80(2, 6);
            }
        }
        n = *(unsigned char *)(moby + 0xBC);
        if (n == 1) {
            func_L00_00217718_17F80(G_cam2_17F80, G_cam2_17F80 + 0x10, 0, 1);
            func_L00_002664B0_17F80(0, 8);
            if (*(unsigned char *)(G_D355_17F80 + 0x13B) == 0) {
                *(unsigned short *)(data + 4) = *(unsigned short *)(data + 0x36);
                *(short *)(data + 0x36) = 4;
                *(int *)(data + 0x38) = G_frame_17F80;
                *(unsigned char *)(data + 8) = n;
            }
            *(unsigned char *)(moby + 0xBC) = 0;
        }
        if (*(unsigned char *)(moby + 0xBC) == 2) {
            func_L00_002664B0_17F80(0, 8);
            func_L00_00217718_17F80(G_cam2_17F80, G_cam2_17F80 + 0x10, 0, 1);
            *(unsigned char *)(moby + 0xBC) = 0;
        }
        if (*(unsigned char *)(moby + 0xBC) == 3) {
            func_L00_002664B0_17F80(0, 8);
            *(unsigned char *)(moby + 0xBC) = 0;
            func_L00_00217718_17F80(G_cam_17F80, G_cam_17F80 + 0x10, 0, 1);
            qcopy(G_saved_17F80 + 0x2B1D, G_cam_17F80 + 0x20);
            qcopy(G_saved_17F80 + 0x2B2D, G_cam_17F80 + 0x30);
        }
        n = *(unsigned char *)(moby + 0x53);
        if ((n == 0 || n == 2) && func_001F9908_17F80(data + 0x244) != 0) {
            *(int *)(data + 0x244) = func_001FA898_17F80(func_001F9878_17F80(func_002140F8_17F80(1200.0f, 2400.0f)));
            if (*(unsigned char *)(moby + 0x53) != 1) {
                func_00213DE0_17F80(moby, 1, 0, func_001F9850_17F80(10));
            }
        } else if (*(unsigned char *)(moby + 0x70) & 2) {
            n = 0;
            if (func_002140B0_17F80(2) == 0) {
                n = 2;
            }
            if (*(unsigned char *)(moby + 0x53) != n) {
                func_00213DE0_17F80(moby, n, 0, func_001F9850_17F80(10));
            }
        }
    } else if (s < 2) {
        if (s == 0) {
            if (*(unsigned char *)(moby + 0x53) != 0) {
                func_00213DE0_17F80(moby, 0, 0, func_001F9850_17F80(0x14));
            }
            *(float *)(moby + 0x18) = func_00214358_17F80(moby + 0x10, 0, 0.5f);
            if (*(int *)(G_D605_17F80 + 0xB3 + 0x10C) == 0 || *(int *)(G_D605_17F80 + 0xB3 + 0x11C) == 0) {
                char **entries = G_entries_17F80;
                int e = func_L00_00267618_17F80(moby);
                *(short *)(entries[e] + 4) = 0xD;
            }
            *(char **)(data + 0x20) = G_name_17F80;
            *(unsigned char *)(moby + 0x20) = 1;
            func_L00_002676E8_17F80(moby, data);
            func_L02_0025D750_17F80(moby);
        }
    } else if (s == 3) {
        if (*(int *)&G_mode_gp_17F80 != 2) {
            *(unsigned char *)(moby + 0x20) = 1;
            if (*(short *)(data + 4) == 4) {
                *(unsigned char *)(G_D355_17F80 + 0x13B) = 1;
                func_L00_00264DB8_17F80(0x1395, func_001F9850_17F80(300));
                if (*(unsigned char *)(G_D50F_17F80 + 0xA) != 0) {
                    func_0022EE28_17F80(1, 0, 0);
                    func_L00_00264DB8_17F80(0x53DB, -1);
                }
                func_0020BFC8_17F80(0, -1);
            }
        }
    } else {
        if (*(unsigned char *)(moby + 0xBC) != 0) {
            func_L00_00217718_17F80(G_cam2_17F80, G_cam2_17F80 + 0x10, 0, 1);
            *(unsigned char *)(moby + 0xBC) = 0;
        }
    }

    rstep = 0.02f;
    rlim = 0.3f;
    look = 0;
    n = *(unsigned char *)(moby + 0x53);
    if (n == 0 || n == 2) {
        look = 1;
        if (func_001F9D48_17F80(moby + 0x10, hp) < 8.0f &&
            func_001FA850_17F80(*(float *)(moby + 0x48),
                                func_L00_001FF860_17F80(*(float *)(g + 0xD0) - *(float *)(moby + 0x10),
                                                        *(float *)(g + 0xD4) - *(float *)(moby + 0x14))) < 1.5707964f) {
            if (func_001F9CB8_17F80(hp + 0x80) > 0.01f) {
                *(int *)(data + 0x248) = func_001F9850_17F80(120);
            } else {
                func_001F9908_17F80(data + 0x248);
            }
        } else if (*(int *)(data + 0x248) != 0) {
            *(int *)(data + 0x248) = 0;
            qcopy(data + 0x250, g + 0xD0);
        }
        if (func_001F9908_17F80(data + 0x24C) != 0) {
            float k = 0.0174532924f;
            float h, p;
            *(int *)(data + 0x24C) = func_001FA898_17F80(func_001F9878_17F80(func_002140F8_17F80(180.0f, 300.0f)));
            h = func_001FA748_17F80(*(float *)(moby + 0x48), func_002140F8_17F80(-90.0f, 90.0f) * k);
            p = func_002140F8_17F80(0.0f, 30.0f) * k;
            func_00215C00_17F80(data + 0x250, 6.0f, h, p);
            func_001F9BD8_17F80(data + 0x250, data + 0x250, moby + 0x10);
        }
        if (*(int *)(data + 0x248) != 0) {
            qcopy(v0, g + 0xD0);
            rstep = 0.0299999993f;
            rlim = 0.300000012f;
        } else {
            qcopy(v0, data + 0x250);
        }
    }
    if (look) {
        qcopy(v1, moby + 0x10);
        v1[2] = v1[2] + 1.0f;
        func_001F9BF0_17F80(v2, v0, v1);
        yaw = func_001FA790_17F80(func_L00_001FF860_17F80(v2[0], v2[1]), *(float *)(moby + 0x48));
        pitch = -func_L00_001FF860_17F80(func_001F9CE8_17F80(v2), v2[2]);
        if (1.5707964f < yaw) {
            yaw = 1.5707964f;
        } else if (yaw < -1.5707964f) {
            yaw = -1.5707964f;
        }
        if (0.5235988f < pitch) {
            pitch = 0.5235988f;
        } else if (pitch < -0.5235988f) {
            pitch = -0.5235988f;
        }
        *(float *)(data + 0xA4) = pitch;
        *(float *)(data + 0xA8) = yaw * 0.6f;
        *(float *)(data + 0x128) = yaw * 0.4f;
    }

    if (func_001F9D48_17F80(moby + 0x10, hp) < 15.0f &&
        func_001FA850_17F80(*(float *)(moby + 0x48),
                            func_L00_001FF860_17F80(*(float *)(hp) - *(float *)(moby + 0x10),
                                                    *(float *)(hp + 4) - *(float *)(moby + 0x14))) < 1.2217305f) {
        if (*(int *)(g + 0x2084) == 0xB && *(int *)(g + 0x198) == 0xF) {
            *(int *)(data + 0x260) = *(int *)(data + 0x260) + 1;
            if (!(*(int *)(data + 0x260) < 0x15)) {
                *(int *)(data + 0x260) = 0x14;
            }
            *(int *)&G_grow_gp_17F80 = *(int *)(data + 0x260);
        }
        if (*(int *)(g + 0x2084) == 4 && func_001F9850_17F80(0x14) < *(int *)(g + 0x198) &&
            (*(int *)&G_frame_gp_17F80) % 10 == 0) {
            *(int *)(data + 0x260) = *(int *)(data + 0x260) - 1;
            if (*(int *)(data + 0x260) < 0) {
                *(int *)(data + 0x260) = 0;
            }
            *(int *)&G_grow_gp_17F80 = *(int *)(data + 0x260);
        }
    }
    if (*(int *)&G_mode_gp_17F80 != 2) {
        func_L05_00317CD8_17F80(moby, data);
        if (*(unsigned char *)G_bighead_17F80 != 0) {
            *(float *)(data + 0xB0) = 2.75f;
        }
        func_L00_00263950_17F80(moby, data + 0x40, 0, rstep * G_dt_17F80, rlim * G_dt_17F80);
        func_L00_00263950_17F80(moby, data + 0xC0, 1, rstep * G_dt_17F80, rlim * G_dt_17F80);
    }
}
