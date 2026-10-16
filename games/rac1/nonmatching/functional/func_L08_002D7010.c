/* functional: medium (written from the asm; checked statically only). Hit-effect emitter of a linked moby group: while the level counter D_L08_0016003C is >= 0x32 it spawns up to 0x28 random sparks (class 0x310/0x311) aimed with the hero-side rotation; then for each live moby of the group's list of classes 0x24C..0x256 it spawns a debris effect with a per-class velocity, and for class 0x1D8 starts a particle burst and removes... (func_0020D678 on it). */
extern int D_L08_0016003C_2D7010 __asm__("D_L08_0016003C");
extern char *D_L08_00160058_2D7010 __asm__("D_L08_00160058");
extern unsigned short *D_L08_001AC340_2D7010[] __asm__("D_L08_001AC340");
extern char *D_L08_0016016C_2D7010 __asm__("D_L08_0016016C");
extern float D_0015EE6C_2D7010 __asm__("D_0015EE6C");
extern float D_L08_00167500_2D7010[] __asm__("D_L08_00167500");
extern int func_002140B0_2D7010(int) __asm__("func_002140B0");
extern char *func_0020D348_2D7010(int) __asm__("func_0020D348");
extern float func_00214158_2D7010(void) __asm__("func_00214158");
extern float func_002140F8_2D7010(float, float) __asm__("func_002140F8");
extern void func_001F9EE8_2D7010(void *, void *, void *) __asm__("func_001F9EE8");
extern void func_001F9BC0_2D7010(void *) __asm__("func_001F9BC0");
extern float func_001F9F90_2D7010(float) __asm__("func_001F9F90");
extern float func_001F9FA8_2D7010(float) __asm__("func_001F9FA8");
extern int func_001FA898_2D7010(float) __asm__("func_001FA898");
extern float func_001FA850_2D7010(float, float) __asm__("func_001FA850");
extern float func_L00_001FF860_2D7010(float, float) __asm__("func_L00_001FF860");
extern void func_L05_00319B58_2D7010(void *, void *, void *, void *, int) __asm__("func_L05_00319B58");
extern void func_L00_0025F4A8_2D7010(void *, void *, void *, float, float, int, int, int, float, float, float, int, float, float, int, int, int, int) __asm__("func_L00_0025F4A8");
extern void func_0020D678_2D7010(void *) __asm__("func_0020D678");

void func_L08_002D7010(char *m)
{
    char *d = *(char **)(m + 0x78);
    char *mobys = D_L08_00160058_2D7010;
    char *other;
    unsigned short *tbl, *p, *q;
    int cnt;
    float v20[4] __attribute__((aligned(16)));
    float v30[4] __attribute__((aligned(16)));
    float v40[4] __attribute__((aligned(16)));
    float *r16;
    float f20;

    tbl = D_L08_001AC340_2D7010[(unsigned char)(mobys + (*(int *)(d + 0x144) << 8))[0x21]];
    if (tbl == 0)
        return;
    cnt = 0;
    other = mobys + ((tbl[0] & 0x7FFF) << 8);
    q = tbl + 1;
    if (D_L08_0016003C_2D7010 >= 0x32) {
        do {
            int id = (func_002140B0_2D7010(0xFF) & 1) ? 0x310 : 0x311;
            char *s = func_0020D348_2D7010(id);
            if (s != 0) {
                int n;
                s[0x30] = 0xFF;
                *(short *)(s + 0x32) = 0x40;
                s[0x31] = 1;
                *(float *)(s + 0x40) = func_00214158_2D7010();
                *(float *)(s + 0x44) = func_00214158_2D7010();
                *(float *)(s + 0x48) = func_00214158_2D7010();
                v20[0] = func_002140F8_2D7010(-1.0f, 1.0f);
                v20[1] = func_002140F8_2D7010(-1.0f, 1.0f);
                v20[2] = func_002140F8_2D7010(-1.0f, 1.0f);
                v20[3] = 1.0f;
                func_001F9EE8_2D7010(v20, v20, D_L08_0016016C_2D7010 + (*(int *)(d + 0x148) << 7));
                *(float *)(s + 0x10) = v20[0];
                *(float *)(s + 0x14) = v20[1];
                *(float *)(s + 0x18) = v20[2];
                *(float *)(s + 0x1C) = v20[3];
                f20 = func_002140F8_2D7010(-0.261799395f, 0.261799395f);
                func_001F9BC0_2D7010(v30);
                v40[0] = func_001F9F90_2D7010(f20) * (D_0015EE6C_2D7010 * 15.0f);
                v40[1] = func_001F9FA8_2D7010(f20) * (D_0015EE6C_2D7010 * 15.0f);
                v40[2] = 0;
                v40[2] = func_002140F8_2D7010(D_0015EE6C_2D7010 * 8.0f, D_0015EE6C_2D7010 * 12.0f);
                n = func_001FA898_2D7010(func_002140F8_2D7010(60.0f, 90.0f));
                func_L05_00319B58_2D7010(s, m, v40, v30, n);
            }
            cnt++;
            if (cnt >= 0x28)
                break;
        } while (D_L08_0016003C_2D7010 >= 0x32);
    }
    p = tbl;
    for (;;) {
        if ((unsigned char)other[0x20] < 0x7F && other != m) {
            unsigned int cls = *(unsigned short *)(other + 0xA6);
            if (cls - 0x24C < 0xB) {
                float a, b, c;
                unsigned int ux[3];
                f20 = func_002140F8_2D7010(-0.261799395f, 0.261799395f);
                v20[0] = func_001F9F90_2D7010(f20) * (D_0015EE6C_2D7010 * 15.0f);
                v20[1] = func_001F9FA8_2D7010(f20) * (D_0015EE6C_2D7010 * 15.0f);
                v20[2] = 0;
                v20[2] = func_002140F8_2D7010(D_0015EE6C_2D7010 * 8.0f, D_0015EE6C_2D7010 * 12.0f);
                switch (cls - 0x24C) {
                case 0: ux[0] = 0x3FD08312; ux[1] = 0xBFABA5E3; ux[2] = 0x4137AE14; break;
                case 1: ux[0] = 0x3FA645A2; ux[1] = 0xC0B1DB23; ux[2] = 0x4137AE14; break;
                case 2: ux[0] = 0x3F210625; ux[1] = 0xC021BA5E; ux[2] = 0x405A2D0E; break;
                case 3: ux[0] = 0x3C9BA5E3; ux[1] = 0xC0C6147B; ux[2] = 0x405A2D0E; break;
                case 4: ux[0] = 0x404C8B44; ux[1] = 0xC006B852; ux[2] = 0x40F91687; break;
                case 5: ux[0] = 0x40070A3D; ux[1] = 0xC040D4FE; ux[2] = 0x40280000; break;
                case 6: ux[0] = 0x408A8F5C; ux[1] = 0xC0CE5E35; ux[2] = 0x40ECB439; break;
                case 7: ux[0] = 0x40524DD3; ux[1] = 0xC0DD999A; ux[2] = 0x403D70A4; break;
                case 8: ux[0] = 0x3FC4FDF4; ux[1] = 0xBF891687; ux[2] = 0x41524DD3; break;
                case 9: ux[0] = 0x3FCC49BA; ux[1] = 0xC020C49C; ux[2] = 0x4105BE77; break;
                case 10: ux[0] = 0x3F628F5C; ux[1] = 0xC0BA5E35; ux[2] = 0x41039581; break;
                default: break;
                }
                v40[0] = *(float *)&ux[0];
                v40[1] = *(float *)&ux[1];
                v40[2] = *(float *)&ux[2];
                v40[3] = 0;
                v30[0] = v40[0];
                v30[1] = v40[1];
                v30[2] = v40[2];
                v30[3] = v40[3];
                r16 = v30;
                a = func_L00_001FF860_2D7010(v20[0], v20[1]);
                f20 = a;
                b = func_L00_001FF860_2D7010(D_L08_00167500_2D7010[0x140 / 4] - *(float *)(m + 0x10),
                                             D_L08_00167500_2D7010[0x144 / 4] - *(float *)(m + 0x14));
                c = func_001FA850_2D7010(f20, b);
                {
                    float lo, hi;
                    if (c < 0.261799395f) {
                        lo = 10.0f;
                        hi = 20.0f;
                    } else {
                        lo = 30.0f;
                        hi = 45.0f;
                    }
                    c = func_002140F8_2D7010(lo, hi);
                }
                func_L05_00319B58_2D7010(other, m, v20, r16, func_001FA898_2D7010(c));
            } else if ((short)cls == 0x1D8) {
                func_001F9BC0_2D7010(v20);
                func_L00_0025F4A8_2D7010(other, v20, other + 0x10, 0.0f, 0.0f, 0x32, 0x3C, 0x3C,
                                         12.0f, 8.0f, 9.0f, -1, 1.0f, 15.0f, 1, 1, -1, 0);
                func_0020D678_2D7010(other);
            }
        }
        if (*(short *)p < -1)
            break;
        p = q;
        other = D_L08_00160058_2D7010 + ((*q & 0x7FFF) << 8);
        q++;
    }
}
