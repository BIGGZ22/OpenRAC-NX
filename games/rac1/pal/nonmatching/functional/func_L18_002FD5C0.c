/* functional: medium. Update of a pulsing hostile (state 0/8 idle): cycles its glow colour, drops to state 6 when low, goes to state 8 when the level event 0x72 or flag 2 fires, otherwise takes hits (damage drains pvars health; at zero it either dies or enters a knockback with state 5) and follows its path toward the current target. Statically checked only. */
typedef u32 u128_FD5C0 __attribute__((mode(TI), aligned(16)));
extern void f_enq_FD5C0(void *, void *) __asm__("func_001F49B0");
extern float f_addrot_FD5C0(float, float) __asm__("func_001FA748");
extern float f_sin_FD5C0(float) __asm__("func_001F9FA8");
extern int f_tween_FD5C0(int, int, float) __asm__("func_001FA8A8");
extern int f_ticks_FD5C0(int) __asm__("func_001F9850");
extern void f_blend_FD5C0(void *, int, int, int) __asm__("func_00213DE0");
extern char *f_findhit_FD5C0(void *, int, int) __asm__("func_L00_0025B478");
extern void f_hit_FD5C0(void *, void *, void *, int, void *, void *, int, int) __asm__("func_L00_0025B4D0");
extern int f_rand_FD5C0(int) __asm__("func_002140B0");
extern void f_arc_FD5C0(void *, float *, void *, void *) __asm__("func_L00_0025BBA0");
extern void f_knock_FD5C0(void *, void *, float, int, int, int) __asm__("func_L00_0025D5B0");
extern void f_anim_FD5C0(void *, void *) __asm__("func_L00_0025E4B0");
extern void f_fin_FD5C0(void *, void *) __asm__("func_L00_0025E590");
extern int f_path_FD5C0(float, void *, void *, int, int, void *, int) __asm__("func_L00_00260FB0");
extern void f_cb_FD5C0(void *) __asm__("func_L18_002FDF58");
extern unsigned char D_0013E633_FD5C0[] __asm__("D_0013E633");
extern float D_0015EE6C_FD5C0 __asm__("D_0015EE6C") MACRO_ADDR;
extern float D_0015EE70_FD5C0 __asm__("D_0015EE70") MACRO_ADDR;
extern int D_L18_0015F6B0_FD5C0 __asm__("D_L18_0015F6B0") MACRO_ADDR;
extern int D_L18_0015F6A8_FD5C0 __asm__("D_L18_0015F6A8") MACRO_ADDR;
extern int D_L18_00162694_FD5C0 __asm__("D_L18_00162694") MACRO_ADDR;
extern float D_L18_001626A4_FD5C0 __asm__("D_L18_001626A4") MACRO_ADDR;
extern float D_L18_001626A8_FD5C0 __asm__("D_L18_001626A8") MACRO_ADDR;
extern int D_L18_0016269C_FD5C0 __asm__("D_L18_0016269C") MACRO_ADDR;
extern int D_L18_001626A0_FD5C0 __asm__("D_L18_001626A0") MACRO_ADDR;
extern float D_L18_0016268C_FD5C0 __asm__("D_L18_0016268C") MACRO_ADDR;
extern float D_L18_001626AC_FD5C0 __asm__("D_L18_001626AC") MACRO_ADDR;
extern float D_L18_001626B0_FD5C0 __asm__("D_L18_001626B0") MACRO_ADDR;
extern float D_L18_001626B4_FD5C0 __asm__("D_L18_001626B4") MACRO_ADDR;
extern float D_L18_001626B8_FD5C0 __asm__("D_L18_001626B8") MACRO_ADDR;
extern int *D_L18_001B11B0_FD5C0[] __asm__("D_L18_001B11B0");
void func_L18_002FD5C0_impl(unsigned char *m) __asm__("func_L18_002FD5C0");

void func_L18_002FD5C0_impl(unsigned char *m) {
    struct { u128_FD5C0 tmp; int st; float damage; float angle[2]; } scratch;
    float zero;
    unsigned char *e = *(unsigned char **)(m + 0x78);
    unsigned char *hero = D_0013E633_FD5C0 + 0xE1D;
    char *hit;
    float speed;
    int *path;
    char *target;
    unsigned char *destination;
    if (m[0x20] == 0 || m[0x20] == 8)
        return;
    if (m[0x20] != 7 && D_L18_0015F6B0_FD5C0 != D_L18_00162694_FD5C0) {
        D_L18_00162694_FD5C0 = D_L18_0015F6B0_FD5C0;
        f_enq_FD5C0((void *)f_cb_FD5C0, m);
    }
    speed = D_L18_001626A4_FD5C0;
    if ((unsigned int)(m[0x20] - 4) < 2)
        speed = D_L18_001626A8_FD5C0;
    *(float *)(e + 0x1F8) = f_addrot_FD5C0(*(float *)(e + 0x1F8), speed * 0.017453292f * D_0015EE6C_FD5C0);
    *(int *)(m + 0x90) = f_tween_FD5C0(D_L18_0016269C_FD5C0, D_L18_001626A0_FD5C0,
                                       f_sin_FD5C0(*(float *)(e + 0x1F8)) * 0.5f + 0.5f);
    if (*(float *)(m + 0x18) < 100.0f && m[0x20] != 7)
        m[0x20] = 6;
    *(float *)(m + 0x2C) = *(float *)(*(char **)(m + 0x24) + 0x24) * D_L18_0016268C_FD5C0;
    if ((*(int *)(hero + 0x2084) == 0x72 || D_L18_0015F6A8_FD5C0 == 2) && m[0x20] != 7) {
        m[0x20] = 8;
        if (m[0x53])
            f_blend_FD5C0(m, 0, 0, f_ticks_FD5C0(10));
        *(int *)(m + 0x94) = 0;
        *(unsigned short *)(m + 0x34) = (*(unsigned short *)(m + 0x34) | 0x41) & 0xEFFF;
    } else {
        zero = 0.0f;
        scratch.damage = zero;
        hit = f_findhit_FD5C0(m, 0x330000, 0);
        f_hit_FD5C0(m, hit, e + 0x20, 0, &scratch.st, &scratch.damage, 0, 4);
        if (scratch.st != 1 && m[0x20] != 6 && scratch.damage != zero) {
            float remaining = *(float *)(e + 0x20) - scratch.damage;
            *(float *)(e + 0x20) = remaining;
            if (remaining <= zero) {
                if (f_rand_FD5C0(2) != 0) {
                    m[0x20] = 6;
                } else {
                    float squared = D_0015EE70_FD5C0;
                    float time = D_0015EE6C_FD5C0;
                    float a = D_L18_001626AC_FD5C0 * squared;
                    float b = D_L18_001626B0_FD5C0 * squared;
                    float c = D_L18_001626B4_FD5C0 * time;
                    float d = D_L18_001626B8_FD5C0 * time;
                    e[0xAD] = 0;
                    *(float *)(e + 0x98) = 0.5f;
                    *(float *)(e + 0x8C) = d;
                    *(float *)(e + 0x80) = a;
                    *(float *)(e + 0x84) = b;
                    *(float *)(e + 0xBC) = D_0015EE6C_FD5C0 + D_0015EE6C_FD5C0;
                    *(float *)(e + 0x88) = c;
                    *(int *)(e + 0x94) = 0x29;
                    *(int *)(e + 0x90) = 0x200;
                    *(unsigned short *)(m + 0x34) &= 0xEFFF;
                    scratch.tmp = *(u128_FD5C0 *)(hit + 0x10);
                    f_arc_FD5C0(&scratch.tmp, scratch.angle, e + 0x88, e + 0x8C);
                    f_knock_FD5C0(m, e + 0x70, scratch.angle[0], 3, 1, 0);
                    *(float *)(e + 0xC0) = 7.5f;
                    *(float *)(e + 0xC4) = 15.0f;
                    m[0x20] = 5;
                    *(int *)(m + 0x94) = 0;
                    e[0x67] = 0x78;
                    f_anim_FD5C0(m, e + 0x60);
                }
            }
        }
        m[0xA4] = 0xFF;
        f_fin_FD5C0(m, e + 0x60);
        destination = e + 0x180;
        path = D_L18_001B11B0_FD5C0[*(int *)(e + 0x1E0)];
        f_path_FD5C0(512.0f, m, destination, 0, 0, path + 4, *path);
        if (*(int *)(e + 0x1C4) == 2) {
            if (*(int *)(hero + 0x208C) == 15) {
                target = *(char **)(hero + 0x560);
                target = target + (*(int *)target << 4);
            } else {
                target = (char *)(hero + 0x80);
            }
            *(u128_FD5C0 *)destination = *(u128_FD5C0 *)target;
        }
        if (*(int *)(e + 0x1C0) == 0)
            *(int *)(e + 0x1C0) = *(int *)(hero + 0x2080);
    }
}
