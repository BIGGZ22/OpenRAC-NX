/* functional: medium. Pre-update of a level-10 screamer: re-rolls its timer, reacts to a hit (damage to vars+0x20, knock-back state 11 on death, one-shot voice), finishes the animation, picks the follow distance (vars+0x180) from the mode and checks the path / volume, setting vars+0x104 = 2 when the moby strayed or is not visible. Not run against captured calls. */
typedef struct Hit_E0FF8 {
    float x;            /* 0x00 */
    float y;            /* 0x04 */
    char p8[8];
    float dir[4];       /* 0x10 */
    char p20[0xC];
    float damage;       /* 0x2C */
    int flags;          /* 0x30 */
} Hit_E0FF8;

typedef struct PathNode_E0FF8 {
    int count;
    char p4[0xC];
    float pts[4];       /* 0x10 */
} PathNode_E0FF8;

extern short G_gp_E0FF8 __asm__("D_L10_00161FB8");
extern char G_13D50F_E0FF8[] __asm__("D_0013D50F");
extern char G_13E633_E0FF8[] __asm__("D_0013E633");
extern float G_dt70_E0FF8 __asm__("D_0015EE70");
extern float G_dt6C_E0FF8 __asm__("D_0015EE6C");
extern char *G_16016C_E0FF8 __asm__("D_L10_0016016C");
extern PathNode_E0FF8 *G_paths_E0FF8[] __asm__("D_L10_001B0C30");
extern float func_002140F8_E0FF8(float, float) __asm__("func_002140F8");
extern float func_001F9878_E0FF8(float) __asm__("func_001F9878");
extern int func_001FA898_E0FF8(float) __asm__("func_001FA898");
extern void func_001F9908_E0FF8(void *) __asm__("func_001F9908");
extern Hit_E0FF8 *func_L00_0025B478_E0FF8(void *, int, int) __asm__("func_L00_0025B478");
extern void func_L00_0025B4D0_E0FF8(void *, void *, void *, int, void *, int, int, int) __asm__("func_L00_0025B4D0");
extern void func_0022EE28_E0FF8(int, int, int) __asm__("func_0022EE28");
extern void func_L00_00264DB8_E0FF8(int, int) __asm__("func_L00_00264DB8");
extern int func_001F9850_E0FF8(int) __asm__("func_001F9850");
extern void func_00213DE0_E0FF8(void *, int, int, int) __asm__("func_00213DE0");
extern float func_L00_001FF860_E0FF8(float, float) __asm__("func_L00_001FF860");
extern void func_L00_0025D5B0_E0FF8(void *, void *, float, int, int, int) __asm__("func_L00_0025D5B0");
extern void func_L00_0025E4B0_E0FF8(void *, void *) __asm__("func_L00_0025E4B0");
extern void func_L00_00261B00_E0FF8(void *, int, int, int, int) __asm__("func_L00_00261B00");
extern void func_L00_0025E590_E0FF8(void *, void *) __asm__("func_L00_0025E590");
extern int func_L00_00260D30_E0FF8(void *, void *, float) __asm__("func_L00_00260D30");
extern int func_L00_00260FB0_E0FF8(void *, void *, int, int, void *, int, float) __asm__("func_L00_00260FB0");
extern void func_001F9BF0_E0FF8(void *, void *, void *) __asm__("func_001F9BF0");
extern void func_001F9EC0_E0FF8(void *, void *, void *) __asm__("func_001F9EC0");
extern float func_001F9D48_E0FF8(void *, void *) __asm__("func_001F9D48");

void func_L10_002E0FF8_impl(char *m) __asm__("func_L10_002E0FF8");
void func_L10_002E0FF8_impl(char *m) {
    char *d = *(char **)(m + 0x78);
    float v[4] __attribute__((aligned(16)));
    float w[4] __attribute__((aligned(16)));
    char hitbuf[16] __attribute__((aligned(16)));
    Hit_E0FF8 *r;
    int result;
    char *anim = d + 0x110;
    char *pos_ref;

    if (*(int *)(d + 0x38) != 0) {
        *(int *)(d + 0x184) = func_001FA898_E0FF8(func_001F9878_E0FF8(func_002140F8_E0FF8(180.0f, 240.0f)));
        *(int *)(d + 0x38) = 0;
    }
    func_001F9908_E0FF8(d + 0x184);
    if (*(unsigned char *)(m + 0x20) != 11) {
        r = func_L00_0025B478_E0FF8(m, 0x330000, 0);
        func_L00_0025B4D0_E0FF8(m, r, d + 0x20, 5, hitbuf, 0, 0, 4);
        if (r != 0) {
            if (*(float *)(d + 0x20) <= r->damage) {
                if (*(int *)&G_gp_E0FF8 != 0) {
                    unsigned unsigned char *flag = (unsigned char *)G_13D50F_E0FF8 + 1;
                    if (flag[0x10] == 0) {
                        flag[0x10] = 1;
                        func_0022EE28_E0FF8(1, 0, 0);
                        func_L00_00264DB8_E0FF8(0x53DB, -1);
                    }
                }
                *(int *)(d + 0x20) = 0;
                *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) & 0xEFFF;
                if (*(unsigned char *)(m + 0x53) != 5) {
                    func_00213DE0_E0FF8(m, 5, 0, func_001F9850_E0FF8(10));
                }
                *(unsigned char *)(m + 0x20) = 11;
                *(unsigned char *)(d + 0x9D) = 3;
                {
                    float a = G_dt70_E0FF8 * 26.0f;
                    float t = G_dt6C_E0FF8;
                    float b = t * 7.0f;
                    float c = t * 10.0f;
                    *(float *)(d + 0x70) = a;
                    *(float *)(d + 0x74) = 0.0005f;
                    *(int *)(d + 0x84) = 9;
                    *(float *)(d + 0x78) = b;
                    *(float *)(d + 0x7C) = c;
                }
                if (r->flags & 1) {
                    func_L00_0025D5B0_E0FF8(m, d + 0x60, func_L00_001FF860_E0FF8(r->dir[0], r->dir[1]), 5, 1, 0);
                } else {
                    func_L00_0025D5B0_E0FF8(m, d + 0x60,
                                            func_L00_001FF860_E0FF8(*(float *)(m + 0x10) - r->x, *(float *)(m + 0x14) - r->y), 5, 1, 0);
                }
                *(unsigned char *)(d + 0x117) = 0x78;
                func_L00_0025E4B0_E0FF8(m, anim);
                func_L00_00261B00_E0FF8(m, 1, 3, 0, -1);
            } else {
                *(float *)(d + 0x20) = *(float *)(d + 0x20) - r->damage;
                *(unsigned char *)(d + 0x117) = 0xFA;
                *(short *)(d + 0x26) = func_001F9850_E0FF8(60);
                func_L00_0025E4B0_E0FF8(m, anim);
            }
            *(unsigned char *)(m + 0xA4) = 0xFF;
        }
    }
    func_L00_0025E590_E0FF8(m, anim);

    if (*(int *)(d + 0x174) == 2) {
        *(float *)(d + 0x180) = 42.0f;
    } else if (*(int *)(d + 0x184) != 0) {
        *(float *)(d + 0x180) = *(float *)(d + 0x178) + 6.0f;
    } else if (*(unsigned char *)(m + 0x20) == 1) {
        *(float *)(d + 0x180) = *(float *)(d + 0x188);
    } else {
        *(float *)(d + 0x180) = *(float *)(d + 0x178);
    }

    pos_ref = d + 0xC0;
    if (*(int *)(d + 0x174) == 2) {
        result = func_L00_00260D30_E0FF8(m, pos_ref, *(float *)(d + 0x180));
        if (result != 2) {
            char *bp = G_16016C_E0FF8 + (*(int *)(d + 0x194) << 7);
            func_001F9BF0_E0FF8(v, G_13E633_E0FF8 + 0xE9D, bp + 0x30);
            v[3] = 0.0f;
            func_001F9EC0_E0FF8(w, v, bp + 0x40);
            if (w[0] <= -1.0f || 0.0f <= w[0] || w[1] <= -1.0f || 1.0f <= w[1] || w[2] <= -1.0f || 1.0f <= w[2]) {
                *(int *)(d + 0x104) = 2;
            }
        }
    } else if (*(unsigned char *)(m + 0x20) == 1 && *(int *)(d + 0x18C) >= 0) {
        PathNode_E0FF8 *p = G_paths_E0FF8[*(int *)(d + 0x18C)];
        result = func_L00_00260FB0_E0FF8(m, pos_ref, 0, 0, p->pts, p->count, *(float *)(d + 0x180));
    } else if (*(unsigned char *)(m + 0x20) != 1 && *(int *)(d + 0x17C) >= 0) {
        PathNode_E0FF8 *p = G_paths_E0FF8[*(int *)(d + 0x17C)];
        result = func_L00_00260FB0_E0FF8(m, pos_ref, 0, 0, p->pts, p->count, *(float *)(d + 0x180));
    } else {
        result = func_L00_00260D30_E0FF8(m, pos_ref, *(float *)(d + 0x180));
    }
    if (result != 2) {
        if (*(float *)(d + 0x180) < func_001F9D48_E0FF8(d + 0x130, pos_ref)) {
            *(int *)(d + 0x104) = 2;
        } else if (*(unsigned char *)(m + 0x31) == 0) {
            *(int *)(d + 0x104) = 2;
        }
    }
    if (*(int *)(d + 0x100) == 0) {
        *(int *)(d + 0x100) = *(int *)(G_13E633_E0FF8 + 0x2E9D);
    }
}
