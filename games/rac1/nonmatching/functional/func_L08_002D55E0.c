/* functional: medium. Batalia enemy moby (two child mobys, class 0x14B) update: state 0 spawns the children and picks state 0xB/0xE/6 by data+0x140/0x144; states 3/6/0xB/0xE/0x63 chase/animate/pop children; ends by computing the position delta and calling func_L00_002617B0. Not runnable in capture; checked by reading only. */
extern unsigned char D_0013E633_55E0[] __asm__("D_0013E633");
extern unsigned char D_0014171B_55E0[] __asm__("D_0014171B");
extern unsigned char D_0013D355_55E0[] __asm__("D_0013D355");
extern unsigned char D_0013D5CA_55E0[] __asm__("D_0013D5CA");
extern float D_0015EE6C_55E0 __asm__("D_0015EE6C");
extern char *D_L08_00160058_55E0 __asm__("D_L08_00160058");
extern char **D_L08_001B0FB0_55E0[] __asm__("D_L08_001B0FB0");
extern float D_L08_00161938_55E0 __asm__("D_L08_00161938");
extern float D_L08_0016195C_55E0 __asm__("D_L08_0016195C");
extern float D_L08_00161960_55E0 __asm__("D_L08_00161960");
extern void destroy_55E0(void *) __asm__("func_0020D678");
extern char *spawn_55E0(int) __asm__("func_0020D348");
extern void animate_55E0(void *, int, int, int) __asm__("func_00213DE0");
extern void init_55E0(void *) __asm__("func_L00_00251E30");
extern void sub5FA8_55E0(void *) __asm__("func_L08_002D5FA8");
extern void setdata_55E0(void *, int, void *) __asm__("func_0020D960");
extern void f7758_55E0(void *) __asm__("func_L08_002D7758");
extern void f7010_55E0(void *) __asm__("func_L08_002D7010");
extern void f68D0_55E0(void *, int) __asm__("func_L08_002D68D0");
extern void f6C58_55E0(void *, int, int) __asm__("func_L08_002D6C58");
extern char *fDC18_55E0(void *, void *, void *) __asm__("func_L08_002DDC18");
extern void zero_55E0(void *) __asm__("func_001F9BC0");
extern int f2i_55E0(float) __asm__("func_001FA898");
extern float dist_55E0(void *, void *) __asm__("func_001F9D48");
extern int scale_55E0(int) __asm__("func_001F9850");
extern void sound_55E0(int, int) __asm__("func_L00_00203F20");
extern int chk_55E0(void *, float) __asm__("func_00215B18");
extern float getf_55E0(void *) __asm__("func_0020D830");
extern void vec_55E0(void *, int, void *) __asm__("func_L00_00250800");
extern float addrot_55E0(float, float) __asm__("func_001FA748");
extern float cos_55E0(float) __asm__("func_001F9F90");
extern float sin_55E0(float) __asm__("func_001F9FA8");
extern void vsub_55E0(void *, void *, void *) __asm__("func_001F9BF0");
extern int voice_55E0(int, int, void *) __asm__("func_0022ED80");
extern void fin_55E0(void *, void *, void *, void *) __asm__("func_L00_002617B0");

void func_L08_002D55E0(char *moby) {
    char *data = *(char **)(moby + 0x78);
    int oldpos[4];
    float rot[4];
    float vel[4];     /* sp+0x20 */
    float tmp[4];     /* sp+0x30 */
    int *pos = (int *)(moby + 0x10);
    int *src;
    char *target;
    int state;
    char *c0, *c1;

    oldpos[0] = pos[0]; oldpos[1] = pos[1]; oldpos[2] = pos[2]; oldpos[3] = pos[3];
    ((int *)rot)[0] = ((int *)(moby + 0x40))[0]; ((int *)rot)[1] = ((int *)(moby + 0x40))[1];
    ((int *)rot)[2] = ((int *)(moby + 0x40))[2]; ((int *)rot)[3] = ((int *)(moby + 0x40))[3];

    if (*(unsigned char *)(data + 0x2E) == 2) {
        if (*(char **)(data + 0x160) != 0) destroy_55E0(*(char **)(data + 0x160));
        if (*(char **)(data + 0x164) != 0) destroy_55E0(*(char **)(data + 0x164));
        destroy_55E0(moby);
        return;
    }
    sub5FA8_55E0(moby);
    target = *(char **)(data + 0xF0);
    state = *(unsigned char *)(moby + 0x20);

    if (state == 0) {
        int i;
        for (i = 0; i < 2; i++) {
            int off = (i == 0) ? 0x160 : 0x164;
            if (*(char **)(data + off) == 0) {
                char *ch = spawn_55E0(0x14B);
                int *s2, *d2;
                *(char **)(data + off) = ch;
                *(unsigned char *)(ch + 0x30) = 0x40;
                *(short *)(ch + 0x32) = 0x40;
                *(unsigned char *)(ch + 0x31) = 1;
                s2 = *(int **)(D_0013E633_55E0 + 0x2E9D);
                *(long *)(ch + 0x38) = *(long *)((char *)s2 + 0x38);
                *(unsigned short *)(ch + 0x34) = (i == 0) ? 0x4000 : 0xC000;
                d2 = (int *)(ch + 0x10);
                d2[0] = pos[0]; d2[1] = pos[1]; d2[2] = pos[2]; d2[3] = pos[3];
                *(int *)(ch + 0x94) = 0;
                animate_55E0(ch, 0, 0, 0);
                init_55E0(ch);
            }
        }
        setdata_55E0(moby, 0, data + 0x100);
        *(unsigned short *)(moby + 0x34) |= 0x1000;
        *(float *)(data + 0x16C) = D_0015EE6C_55E0 + D_0015EE6C_55E0;
        if (*(int *)(data + 0x140) > 0 ||
            (*(int *)(data + 0x144) > 0 && *(unsigned char *)(D_0013D355_55E0 + 0x13B + 0x2E) == 0)) {
            *(unsigned char *)(moby + 0x20) = (*(int *)(data + 0x140) > 0) ? 0xB : 0xE;
            if (*(int *)(data + 0x140) > 0) {
                *(unsigned short *)(moby + 0x34) = (*(unsigned short *)(moby + 0x34) & 0xEFFF) | 0x41;
            } else {
                *(unsigned short *)(moby + 0x34) |= 0x41;
            }
            *(unsigned short *)(*(char **)(data + 0x160) + 0x34) |= 0x41;
            *(unsigned short *)(*(char **)(data + 0x164) + 0x34) |= 0x41;
            *(int *)(moby + 0x94) = 0;
            *(int *)(*(char **)(data + 0x160) + 0x94) = 0;
            *(int *)(*(char **)(data + 0x164) + 0x94) = 0;
            if (*(unsigned char *)(moby + 0x53) != 0) animate_55E0(moby, 0, 0, 0);
        } else {
            if (*(int *)(data + 0x144) > 0) f7758_55E0(moby);
            src = (int *)((char *)D_L08_001B0FB0_55E0[*(int *)(data + 0x150)] + 0x10);
            pos[0] = src[0]; pos[1] = src[1]; pos[2] = src[2]; pos[3] = src[3];
            *(unsigned char *)(moby + 0x20) = 6;
            if (*(unsigned char *)(moby + 0x53) != 1) animate_55E0(moby, 1, 0, 0);
        }
        zero_55E0(data + 0x40);
        *(unsigned short *)(data + 0x3E) |= 2;
        *(unsigned char *)(data + 0x29) = 0;
        *(unsigned char *)(data + 0x28) = 3;
        *(float *)(data + 0x30) = 3.0f;
        *(unsigned char *)(data + 0x2A) = 12;
        *(unsigned char *)(data + 0x2E) = 1;
        *(float *)(data + 0x20) = 9.0f;
        zero_55E0(data + 0x40);
        *(int *)(data + 0x178) = 0;
        *(float *)(data + 0x170) = *(float *)(moby + 0x48);
        *(unsigned char *)(data + 0x58) = f2i_55E0(D_L08_0016195C_55E0);
        *(unsigned char *)(data + 0x5A) = f2i_55E0(D_L08_00161960_55E0 * 8.0f);
    } else if (state == 6) {
        f68D0_55E0(moby, *(int *)(data + 0x150));
        if (*(int *)(data + 0xF4) != 2) {
            if (dist_55E0(pos, target + 0x10) < 20.0f) {
                if (*(unsigned short *)(D_0014171B_55E0 + 0x61D) == 0 && *(unsigned char *)(D_0013D5CA_55E0 + 9) == 0) {
                    sound_55E0(0x1F44, 0x5A);
                }
                *(unsigned char *)(moby + 0x20) = 3;
                if (*(unsigned char *)(moby + 0x53) != 2) {
                    animate_55E0(moby, 2, 0, scale_55E0(10));
                }
            }
        }
    } else if (state == 3) {
        f68D0_55E0(moby, *(int *)(data + 0x150));
        if (*(int *)(data + 0xF4) == 2 || 22.0f < dist_55E0(pos, target + 0x10)) {
            *(unsigned char *)(moby + 0x20) = 6;
            if (*(unsigned char *)(moby + 0x53) != 1) {
                animate_55E0(moby, 1, 0, scale_55E0(10));
            }
        }
        if (chk_55E0(moby, 4.0f) != 0 || chk_55E0(moby, 10.0f) != 0) {
            float f21, a, c, s;
            if (getf_55E0(moby) == 4.0f) {
                vec_55E0(moby, 1, tmp);
            } else {
                vec_55E0(moby, 2, tmp);
            }
            f21 = D_L08_00161938_55E0;
            f21 = f21 * D_0015EE6C_55E0;
            a = addrot_55E0(*(float *)(data + 0x170), *(float *)(moby + 0x48));
            c = cos_55E0(a);
            vel[0] = c * (cos_55E0(*(float *)(data + 0x178)) * f21);
            a = addrot_55E0(*(float *)(data + 0x170), *(float *)(moby + 0x48));
            s = sin_55E0(a);
            *(int *)&vel[2] = 0;
            vel[1] = s * (cos_55E0(*(float *)(data + 0x178)) * f21);
            vel[2] = -sin_55E0(*(float *)(data + 0x178)) * f21;
            target = fDC18_55E0(moby, tmp, vel);
            if (target != 0) *(unsigned char *)(target + 0x20) = 1;
        }
    } else if (state == 0xB) {
        char *t = D_L08_00160058_55E0 + (*(int *)(data + 0x140) << 8);
        if (**(float **)(t + 0x78) == 1.0f) {
            int *s3 = (int *)((char *)D_L08_001B0FB0_55E0[*(int *)(data + 0x14C)] + 0x10);
            int *d3;
            pos[0] = s3[0]; pos[1] = s3[1]; pos[2] = s3[2]; pos[3] = s3[3];
            d3 = (int *)(*(char **)(data + 0x160) + 0x10);
            d3[0] = pos[0]; d3[1] = pos[1]; d3[2] = pos[2]; d3[3] = pos[3];
            d3 = (int *)(*(char **)(data + 0x164) + 0x10);
            d3[0] = pos[0]; d3[1] = pos[1]; d3[2] = pos[2]; d3[3] = pos[3];
            *(unsigned short *)(moby + 0x34) &= 0xFFBE;
            *(unsigned short *)(*(char **)(data + 0x160) + 0x34) &= 0xFFBE;
            *(unsigned short *)(*(char **)(data + 0x164) + 0x34) &= 0xFFBE;
            goto after_b;
        }
    } else if (state == 0xE) {
        if (*(int *)(data + 0xF4) != 2) {
            unsigned char *p = D_0013D355_55E0 + 0x13B;
            int *d3;
            f7010_55E0(moby);
            if (p[0x2E] != 0) voice_55E0(6, 0, moby);
            p[0x2E] = 1;
            d3 = (int *)(*(char **)(data + 0x160) + 0x10);
            d3[0] = pos[0]; d3[1] = pos[1]; d3[2] = pos[2]; d3[3] = pos[3];
            d3 = (int *)(*(char **)(data + 0x164) + 0x10);
            d3[0] = pos[0]; d3[1] = pos[1]; d3[2] = pos[2]; d3[3] = pos[3];
            *(unsigned short *)(moby + 0x34) &= 0xFFBE;
            *(unsigned short *)(*(char **)(data + 0x160) + 0x34) &= 0xFFBE;
            *(unsigned short *)(*(char **)(data + 0x164) + 0x34) &= 0xFFBE;
        after_b:
            c0 = *(char **)(data + 0x160);
            c1 = *(char **)(data + 0x164);
            *(int *)(moby + 0x94) = *(int *)(*(char **)(moby + 0x24) + 0x10);
            *(int *)(c0 + 0x94) = *(int *)(*(char **)(c0 + 0x24) + 0x10);
            *(int *)(c1 + 0x94) = *(int *)(*(char **)(c1 + 0x24) + 0x10);
            *(unsigned short *)(moby + 0x34) |= 0x1000;
            if (*(int *)(data + 0xF4) != 2) {
                *(unsigned char *)(moby + 0x20) = 6;
                if (*(unsigned char *)(moby + 0x53) != 1) {
                    animate_55E0(moby, 1, 0, scale_55E0(10));
                }
            }
        }
    } else if (state == 0x63) {
        float f;
        f68D0_55E0(moby, *(int *)(data + 0x150));
        f = getf_55E0(moby);
        if ((59.0f <= f && f <= 60.0f) || (119.0f <= f && f <= 120.0f)) {
            f6C58_55E0(moby, 4, 10);
        } else if ((29.0f <= f && f <= 30.0f) || (89.0f <= f && f <= 90.0f)) {
            f6C58_55E0(moby, 5, 10);
        }
    }
    /* states 0xC/0xD/0xF.. and the rest fall through to the tail */
    vsub_55E0(vel, pos, oldpos);
    fin_55E0(data + 0x60, vel, moby + 0x40, rot);
}
