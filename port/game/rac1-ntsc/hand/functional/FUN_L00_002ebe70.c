/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), nonmatching/functional/func_L00_002ED320.c (games/rac1): func_L00_002ED320, functional C there, not matched.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */
/* functional: medium. Vendor-race follow camera update: eases the camera's distance/pitch/height with the hero's state (0x2C or 0x25), turns the camera angle toward the hero's heading (or the best nearby target moby in levels 14, 7, 9), places the camera behind that heading, collision-traces it and pulls it back to the hit point. Written from the asm and the NTSC decomp draft; the draft's null-pointer read for state 0x2C with no target in levels 14/7/9 is kept as in retail. */
/* carry, not from a `same` function or an `exact` data address (read them):
 *   D_L00_00161EA8 -> 00161DD8: guess
 */

/* The quadword helpers in plain C: hostgen maps only the decompilation's own qcopy, qcopy_nc and
 * qzero to the runtime's, and wrote these renamed copies' inline assembly as nothing (every
 * copy through them was lost). Four words in order, as the lq/sq pair moves them. */
static __inline__ void qcopy_c00_2ebe70(void *dst, void *src) {
    int *d_ = (int *)dst, *s_ = (int *)src;
    d_[0] = s_[0]; d_[1] = s_[1]; d_[2] = s_[2]; d_[3] = s_[3];
}
extern void func_00214D28_ED320_c00_2ebe70(float, float, void *) __asm__("func_00213ED8");
extern void func_001F9BF0_ED320_c00_2ebe70(void *, void *, void *) __asm__("func_001F9A28");
extern void func_L00_001FF4B0_ED320_c00_2ebe70(float, void *, void *) __asm__("func_L00_001FF500");
extern float func_001F9F90_ED320_c00_2ebe70(float) __asm__("func_001F9DC8");
extern float func_001F9FA8_ED320_c00_2ebe70(float) __asm__("func_001F9DE0");
extern float func_001F9C78_ED320_c00_2ebe70(void *, void *) __asm__("func_001F9AB0");
extern float func_001F9CB8_ED320_c00_2ebe70(void *) __asm__("func_001F9AF0");
extern float func_L00_001FF860_ED320_c00_2ebe70(float, float) __asm__("func_L00_001FF8B0");
extern float func_L00_001EB6A8_ED320_c00_2ebe70(void *, float, float, float, float, float) __asm__("func_L00_001EB328");
extern void func_001F9BD8_ED320_c00_2ebe70(void *, void *, void *) __asm__("func_001F9A10");
extern void func_001F9C30_ED320_c00_2ebe70(void *, void *, float) __asm__("func_001F9A68");
extern int func_L00_001EFFF0_ED320_c00_2ebe70(void *, void *, int, int, int) __asm__("func_L00_001EFC70");
extern int func_L00_001F10E0_ED320_c00_2ebe70(float, void *, int, int) __asm__("func_L00_001F0D60");
extern char D_0013F450_ED320_c00_2ebe70[] __asm__("D_0013F350");
extern char D_L00_00166D80_ED320_c00_2ebe70[] __asm__("D_L00_00166C80");
extern char D_L00_00173F60_ED320_c00_2ebe70[] __asm__("D_L00_00173E60");
extern unsigned short *D_L00_001ABBC0_ED320_c00_2ebe70[] __asm__("D_L00_001AB840");
extern char *D_L00_00160098_ED320_c00_2ebe70 __asm__("D_L00_0015FFD8");
extern float D_0015EE6C_ED320_c00_2ebe70 __asm__("D_0015ED6C") ;
extern int D_0015EE84_ED320_c00_2ebe70 __asm__("D_0015ED84") ;
extern float D_L00_00161EA8_ED320_c00_2ebe70 __asm__("D_L00_00161DD8") ;
void FUN_L00_002ebe70(char *o) {
    float next[4] __attribute__((aligned(16)));
    float diff[4] __attribute__((aligned(16)));
    float scaled[4] __attribute__((aligned(16)));
    float aim[4] __attribute__((aligned(16)));
    char *hero = D_0013F450_ED320_c00_2ebe70;
    char *m = *(char **)(o + 0x70);
    char *p = m + 0x40;
    float *angle = (float *)(m + 0xB8);
    float *distance = (float *)(m + 0xA0);
    float target, rate, fraction, score, best;
    char *candidate, *chosen, *cur;
    unsigned short *list;
    int i;
    if (*(int *)(hero + 0x994) != 0 && *(int *)(hero + 0x2084) == 0x2C) {
        fraction = *(float *)(*(char **)(hero + 0x994) + 0x18) - *(float *)(hero + 0x88);
        if (*(float *)(hero + 0x9A0) < fraction) fraction = *(float *)(hero + 0x9A0);
        else if (fraction < 0.0f) fraction = 0.0f;
        fraction = fraction / *(float *)(hero + 0x9A0);
        target = fraction + 3.65f;
        rate = fraction + fraction;
    } else {
        target = 4.64f;
        rate = 2.0f;
    }
    func_00214D28_ED320_c00_2ebe70(target, D_0015EE6C_ED320_c00_2ebe70 * 8.0f, *(char **)(*(char **)(D_L00_00166D80_ED320_c00_2ebe70 + 0x180) + 0x70) + 0xA0);
    func_00214D28_ED320_c00_2ebe70(rate, D_0015EE6C_ED320_c00_2ebe70 * 4.0f, *(char **)(*(char **)(D_L00_00166D80_ED320_c00_2ebe70 + 0x180) + 0x70) + 0xB0);
    if (D_0015EE84_ED320_c00_2ebe70 == 0xE && *(int *)(hero + 0x2084) == 0x2C)
        func_00214D28_ED320_c00_2ebe70(rate + 0.25f, D_0015EE6C_ED320_c00_2ebe70 * 8.0f, *(char **)(*(char **)(D_L00_00166D80_ED320_c00_2ebe70 + 0x180) + 0x70) + 0x8C);
    else
        func_00214D28_ED320_c00_2ebe70(rate - 0.5f, D_0015EE6C_ED320_c00_2ebe70 * 8.0f, *(char **)(*(char **)(D_L00_00166D80_ED320_c00_2ebe70 + 0x180) + 0x70) + 0x8C);
    if (*(int *)(hero + 0x2084) == 0x2C) {
        cur = *(char **)(hero + 0x994);
        if ((D_0015EE84_ED320_c00_2ebe70 == 0xE || D_0015EE84_ED320_c00_2ebe70 == 7 || D_0015EE84_ED320_c00_2ebe70 == 9) &&
            (cur == 0 || *(unsigned char *)(cur + 0x21) != 0xFF)) {
            chosen = 0;
            best = 1000.0f;
            cur = *(char **)(hero + 0x994);
            list = D_L00_001ABBC0_ED320_c00_2ebe70[*(unsigned char *)(cur + 0x21)];
            if (list != 0) {
                do {
                    candidate = D_L00_00160098_ED320_c00_2ebe70 + (*list & 0x7FFF) * 0x100;
                    if (candidate != cur &&
                        (*(short *)(candidate + 0xA6) == 0x323 || *(short *)(candidate + 0xA6) == 0x2F6)) {
                        func_001F9BF0_ED320_c00_2ebe70(diff, candidate + 0x10, cur + 0x10);
                        func_L00_001FF4B0_ED320_c00_2ebe70(1.0f, scaled, diff);
                        aim[0] = func_001F9F90_ED320_c00_2ebe70(*(float *)(hero + 0x98));
                        aim[1] = func_001F9FA8_ED320_c00_2ebe70(*(float *)(hero + 0x98));
                        aim[2] = 0.0f;
                        score = func_001F9C78_ED320_c00_2ebe70(scaled, aim);
                        if (0.0f < score) {
                            score = func_001F9CB8_ED320_c00_2ebe70(diff) - score * 10.0f;
                            if (score < best) {
                                chosen = candidate;
                                best = score;
                            }
                        }
                    }
                } while ((short)*list++ >= 0);
            }
            if (chosen != 0) {
                func_001F9BF0_ED320_c00_2ebe70(diff, chosen + 0x10, cur + 0x10);
                target = func_L00_001FF860_ED320_c00_2ebe70(diff[0], diff[1]);
            } else if (*(int *)(p + 0x40) == 1) {
                target = *(float *)(*(char **)(p + 0x44) + 0x48);
            } else {
                target = *(float *)(hero + 0x98);
            }
        } else {
            target = *(float *)(hero + 0x98);
        }
        *angle = func_L00_001EB6A8_ED320_c00_2ebe70(m + 0xBC, *angle, target, 0.00125f, 0.175f, 0.0f);
    } else if (*(int *)(hero + 0x2084) == 0x25) {
        if (*(int *)(p + 0x40) == 1)
            target = *(float *)(*(char **)(p + 0x44) + 0x48);
        else
            target = *(float *)(hero + 0x98);
        *angle = func_L00_001EB6A8_ED320_c00_2ebe70(m + 0xBC, *angle, target, D_L00_00161EA8_ED320_c00_2ebe70, 0.175f, 0.0f);
    }
    *(float *)(p + 0x30) = func_001F9F90_ED320_c00_2ebe70(*angle);
    *(float *)(p + 0x34) = func_001F9FA8_ED320_c00_2ebe70(*angle);
    *(int *)(p + 0x38) = 0;
    *(float *)(p + 0x3C) = *(float *)(p + 0x38);
    func_L00_001FF4B0_ED320_c00_2ebe70(-*distance, next, p + 0x30);
    next[2] = next[2] + distance[4];
    func_001F9BD8_ED320_c00_2ebe70(next, next, hero + 0x80);
    if (func_L00_001EFFF0_ED320_c00_2ebe70(o + 0x30, next, 0x12, 0, 0) != 0) {
        func_001F9BF0_ED320_c00_2ebe70(diff, D_L00_00173F60_ED320_c00_2ebe70, o + 0x30);
        score = func_001F9CB8_ED320_c00_2ebe70(diff);
        if (0.5f < score) {
            func_001F9C30_ED320_c00_2ebe70(diff, diff, (score - 0.5f) / score);
            func_001F9BD8_ED320_c00_2ebe70(next, o + 0x30, diff);
            for (i = 0; i < 5; i++) {
                if (func_L00_001F10E0_ED320_c00_2ebe70(0.75f, next, 0, 0) == 0) break;
                qcopy_c00_2ebe70(next, D_L00_00173F60_ED320_c00_2ebe70 + 0x10);
            }
        }
        qcopy_c00_2ebe70(o + 0x30, next);
        func_001F9BF0_ED320_c00_2ebe70(diff, hero + 0x80, o + 0x30);
        *angle = func_L00_001FF860_ED320_c00_2ebe70(diff[0], diff[1]);
        *(int *)(m + 0xBC) = 0;
    } else {
        qcopy_c00_2ebe70(o + 0x30, next);
    }
}
