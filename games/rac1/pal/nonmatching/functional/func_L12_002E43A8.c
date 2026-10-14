/* functional: high. Exact logic of retail func_L12_002E43A8 in l12_hoven.
 * Level 12 hoven: per-entry update that places and blends a set of points around the moby.
 */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef short s16;
typedef int s32;
typedef int openrac_ti __attribute__((mode(TI)));

extern short D_L12_001618BC_2E43A8 __asm__("D_L12_001618BC");
extern short D_L12_001618C0_2E43A8 __asm__("D_L12_001618C0");
extern short D_L12_001618C4_2E43A8 __asm__("D_L12_001618C4");
extern short D_L12_001618C8_2E43A8 __asm__("D_L12_001618C8");
extern short D_L12_001618CC_2E43A8 __asm__("D_L12_001618CC");
extern short D_L12_001618D0_2E43A8 __asm__("D_L12_001618D0");
extern short D_L12_001618D4_2E43A8 __asm__("D_L12_001618D4");
extern short D_L12_001618D8_2E43A8 __asm__("D_L12_001618D8");
extern short D_L12_001618DC_2E43A8 __asm__("D_L12_001618DC");
extern short D_L12_001618E0_2E43A8 __asm__("D_L12_001618E0");
extern short D_L12_001618E4_2E43A8 __asm__("D_L12_001618E4");
extern short D_L12_001618E8_2E43A8 __asm__("D_L12_001618E8");
extern short D_L12_001618EC_2E43A8 __asm__("D_L12_001618EC");
extern short D_L12_001618F0_2E43A8 __asm__("D_L12_001618F0");
extern short D_L12_001618F4_2E43A8 __asm__("D_L12_001618F4");
extern short D_L12_001618F8_2E43A8 __asm__("D_L12_001618F8");
extern short D_L12_001618FC_2E43A8 __asm__("D_L12_001618FC");
extern short D_L12_00161900_2E43A8 __asm__("D_L12_00161900");
extern short D_L12_00161904_2E43A8 __asm__("D_L12_00161904");

extern float D_0015EE6C_2E43A8 __asm__("D_0015EE6C");
extern float D_0015EE70_2E43A8 __asm__("D_0015EE70");

extern float func_00214158_2E43A8(void) __asm__("func_00214158");
extern float func_002140F8_2E43A8(float, float) __asm__("func_002140F8");
extern float func_001F9F90_2E43A8(float) __asm__("func_001F9F90");
extern float func_001F9FA8_2E43A8(float) __asm__("func_001F9FA8");
extern void func_001F9C30_2E43A8(void *, void *, float) __asm__("func_001F9C30");
extern void func_001F9BD8_2E43A8(void *, void *, void *) __asm__("func_001F9BD8");
extern int func_001F9850_2E43A8(int) __asm__("func_001F9850");
extern int func_001FA8A8_2E43A8(int, int, float) __asm__("func_001FA8A8");
extern float func_001F9878_2E43A8(float) __asm__("func_001F9878");
extern int func_001FA898_2E43A8(float) __asm__("func_001FA898");
extern char *func_00219780_2E43A8(void *, void *, void *, int, int, int, int, int, int) __asm__("func_00219780");
extern void func_L00_00260958_2E43A8(float *v, float s) __asm__("func_L00_00260958");

static inline void qcopy_2E43A8(void *dst, const void *src) {
    *(openrac_ti *)dst = *(const openrac_ti *)src;
}

void func_L12_002E43A8(char *m) {
    openrac_ti vv[5];
    float *fv;
    int cnt;
    int i;
    int k;
    int n;
    int a19;
    int a18;
    int a17;
    int a16;
    int a2;
    int j;
    float f20;
    float f21;
    float f22;
    float t;
    float t2;
    float t3;
    float t4;
    float tmp;
    float r0;
    float k55;
    float k0;

    fv = (float *)vv;
    k55 = 0.55f;
    k0 = 0.0f;
    cnt = *(int *)&D_L12_001618BC_2E43A8;
    if (*(unsigned char *)(m + 0x20) == 15) {
        cnt = func_001FA898_2E43A8((float)cnt * 1.5f);
    }
    for (i = 0; i < cnt; i++) {
        qcopy_2E43A8(vv + 1, m + 0x10);
        func_L00_00260958_2E43A8((float *)(vv + 1), *(float *)&D_L12_001618C4_2E43A8);
        fv[6] = fv[6] + k55;
        f22 = func_00214158_2E43A8();
        f20 = func_002140F8_2E43A8(*(float *)&D_L12_001618D0_2E43A8, *(float *)&D_L12_001618D4_2E43A8);
        f21 = func_002140F8_2E43A8(*(float *)&D_L12_001618D8_2E43A8, *(float *)&D_L12_001618DC_2E43A8);
        if (*(unsigned char *)(m + 0x20) == 15) {
            f20 = f20 * k55;
            f21 = f21 * k55;
            fv[6] = fv[6] - 0.2f;
        }
        tmp = func_001F9F90_2E43A8(f22);
        fv[8] = tmp * (f20 * D_0015EE6C_2E43A8);
        r0 = func_001F9FA8_2E43A8(f22);
        fv[10] = f21 * D_0015EE6C_2E43A8 + k0;
        fv[9] = r0 * (f20 * D_0015EE6C_2E43A8);
        func_001F9C30_2E43A8(vv + 4, *(char **)(m + 0x78) + 0x120, 0.5f);
        fv[18] = k0;
        func_001F9BD8_2E43A8(vv + 2, vv + 2, vv + 4);
        qcopy_2E43A8(vv + 3, vv + 2);
        j = func_001F9850_2E43A8(*(int *)&D_L12_00161900_2E43A8);
        fv[14] = fv[14] - *(float *)&D_L12_001618E0_2E43A8 * D_0015EE70_2E43A8 * (float)j;
        n = *(int *)&D_L12_001618C0_2E43A8;
        if (n > 0) {
            f22 = 0.5f;
            f21 = 0.0f;
            f20 = 1.0f;
            for (k = 0; k < *(int *)&D_L12_001618C0_2E43A8; k++) {
                t = func_002140F8_2E43A8(f22, 1.5f);
                fv[11] = t * *(float *)&D_L12_001618E4_2E43A8;
                fv[15] = t * *(float *)&D_L12_001618E8_2E43A8;
                func_L00_00260958_2E43A8((float *)(vv + 1), *(float *)&D_L12_001618C8_2E43A8);
                func_L00_00260958_2E43A8((float *)(vv + 3), *(float *)&D_L12_001618CC_2E43A8 * D_0015EE6C_2E43A8);
                t2 = func_002140F8_2E43A8(f21, f20);
                a19 = func_001FA8A8_2E43A8(*(int *)&D_L12_001618EC_2E43A8, *(int *)&D_L12_001618F4_2E43A8, t2);
                t3 = func_002140F8_2E43A8(f21, f20);
                a18 = func_001FA8A8_2E43A8(*(int *)&D_L12_001618F0_2E43A8, *(int *)&D_L12_001618F8_2E43A8, t3);
                a17 = func_001F9850_2E43A8(*(int *)&D_L12_001618FC_2E43A8);
                a16 = func_001F9850_2E43A8(*(int *)&D_L12_00161900_2E43A8);
                t4 = func_002140F8_2E43A8((float)*(int *)&D_L12_00161904_2E43A8 * f22, (float)*(int *)&D_L12_00161904_2E43A8 * 2.5f);
                a2 = func_001FA898_2E43A8(func_001F9878_2E43A8(t4));
                func_00219780_2E43A8(vv + 1, vv + 2, vv + 3, a19, a18, a17, a16, a2, -1);
            }
        }
    }
}
