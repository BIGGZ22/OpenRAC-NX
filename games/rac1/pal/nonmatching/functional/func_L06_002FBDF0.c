/* functional: medium. Per-frame update of the Blarg boss-type moby (state byte at +0x20): scales damage by the hitter's class, runs the 20/10 hp recovery phases (or death state 14), drives the recovery timer (data+0x12C) and its fade D_L06_0015F4FC, spins the glow phase and follows its path. Written from the asm with the NTSC decomp's draft as a guide. */
extern char D_0013E633[];
extern char D_0013A5E0_2FBDF0[] __asm__("D_0013A5E0");
extern short D_0014172A_2FBDF0 __asm__("D_0014172A") NOT_SDA;
extern int *D_L06_001B0FB0_2FBDF0[] __asm__("D_L06_001B0FB0");
extern float D_L06_0015F4FC_2FBDF0 __asm__("D_L06_0015F4FC") MACRO_ADDR;
extern float D_L06_00161F9C_2FBDF0 SDATA(D_L06_00161F9C);
extern int D_L06_00161FA0_2FBDF0 SDATA(D_L06_00161FA0);
extern int D_L06_00161FA4_2FBDF0 SDATA(D_L06_00161FA4);
extern int func_001FA898_2FBDF0(float) __asm__("func_001FA898");
extern int func_001F9850_2FBDF0(int) __asm__("func_001F9850");
extern float func_001FA888_2FBDF0(int) __asm__("func_001FA888");
extern float func_001F9878_2FBDF0(float) __asm__("func_001F9878");
extern void func_001FFDA0_2FBDF0(int, int) __asm__("func_001FFDA0");
extern char *func_L00_0025B478_2FBDF0(void *, int, int) __asm__("func_L00_0025B478");
extern int func_L00_0025B4D0_2FBDF0(void *, void *, void *, int, int *, float *, int, int) __asm__("func_L00_0025B4D0");
extern void func_L00_002584A8_2FBDF0(void *, int, int) __asm__("func_L00_002584A8");
extern void func_L00_00286128_2FBDF0(void *, void *) __asm__("func_L00_00286128");
extern void func_00213DE0_2FBDF0(void *, int, int, int) __asm__("func_00213DE0");
extern void func_L00_0025E4B0_2FBDF0(void *, void *) __asm__("func_L00_0025E4B0");
extern void func_L00_0025E590_2FBDF0(void *, void *) __asm__("func_L00_0025E590");
extern float func_L00_001FF860_2FBDF0(float, float) __asm__("func_L00_001FF860");
extern void func_001FA1F8_2FBDF0(void *, void *) __asm__("func_001FA1F8");
extern void func_L00_00217718_2FBDF0(void *, void *, int, int) __asm__("func_L00_00217718");
extern void func_L06_002FC578_2FBDF0(void *) __asm__("func_L06_002FC578");
extern float func_001FA748_2FBDF0(float, float) __asm__("func_001FA748");
extern float func_001F9FA8_2FBDF0(float) __asm__("func_001F9FA8");
extern int func_001FA8A8_2FBDF0(int, int, float) __asm__("func_001FA8A8");
extern int func_L00_00260FB0_2FBDF0(float, void *, void *, int, int, void *, int) __asm__("func_L00_00260FB0");

void func_L06_002FBDF0(char *moby) {
    char *data;
    char *hit;
    char *base;
    int out;
    float dmg;
    int kind;
    int cls;
    int *path;
    float hp;
    float rec;
    float ang;
    float t;

    if (*(unsigned char *)(moby + 0x20) == 0) return;
    data = *(char **)(moby + 0x78);
    if ((unsigned int)(*(unsigned char *)(moby + 0x20) - 4) < 10 && *(int *)(data + 0x108) != -1) {
        D_0014172A_2FBDF0 = *(short *)(data + 0x108);
    }
    *(int *)(data + 0x110) = func_001FA898_2FBDF0(*(float *)(data + 0x20) * 16.0f);
    dmg = 0.0f;
    hit = func_L00_0025B478_2FBDF0(moby, 0x330000, 0);
    kind = func_L00_0025B4D0_2FBDF0(moby, hit, data + 0x20, 0, &out, &dmg, 0, 4);
    if (out != 1 && *(unsigned char *)(moby + 0x20) != 14 && *(unsigned char *)(moby + 0x20) != 3 &&
        *(unsigned char *)(moby + 0x20) != 5) {
        if (*(char **)(hit + 0x20) != 0) {
            cls = *(short *)(*(char **)(hit + 0x20) + 0xA6);
            switch (cls) {
            case 0x131:
                dmg = dmg * 0.666f;
                break;
            case 0xB0: case 0x79: case 0x33B:
                dmg = dmg * 1.5f;
                break;
            }
        }
        *(float *)(data + 0x20) = *(float *)(data + 0x20) - dmg;
        if (*(int *)(data + 0x170) == 0) {
            if (func_001F9850_2FBDF0(10) <= *(int *)(data + 0x12C)) {
                hp = *(float *)(data + 0x20);
                rec = 20.0f;
                if (hp <= rec && rec < hp + dmg) goto start_recovery;
            }
            if (func_001F9850_2FBDF0(10) <= *(int *)(data + 0x12C)) {
                hp = *(float *)(data + 0x20);
                rec = 10.0f;
                if (hp <= rec && rec < hp + dmg) {
start_recovery:
                    *(float *)(data + 0x174) = rec;
                    *(int *)(data + 0x170) = 1;
                    *(float *)(data + 0x20) = rec;
                    *(int *)(data + 0x12C) = func_001F9850_2FBDF0(30);
                }
            }
        } else if (*(float *)(data + 0x20) < *(float *)(data + 0x174)) {
            *(float *)(data + 0x20) = *(float *)(data + 0x174);
        }
        if (*(float *)(data + 0x20) <= 0.0f) {
            kind = 1;
        } else if (kind == 1) {
            kind = 5;
        }
        switch (kind) {
        case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10:
            *(unsigned char *)(data + 0x67) = 200;
            break;
        case 1:
        case 2:
            if (*(int *)(data + 0x10C) != -1) {
                func_001FFDA0_2FBDF0(*(int *)(data + 0x10C), 0);
                *(int *)(data + 0x10C) = -1;
            }
            *(unsigned char *)(moby + 0x20) = 14;
            func_L00_002584A8_2FBDF0(moby, 0, -1);
            base = D_L06_0016016C + (*(int *)(data + 0xF8) << 7);
            func_L00_00286128_2FBDF0(base + 0x30, base + 0x70);
            *(unsigned short *)(moby + 0x34) = *(unsigned short *)(moby + 0x34) & 0xEFFF;
            if (*(unsigned char *)(moby + 0x53) != 10) {
                func_00213DE0_2FBDF0(moby, 10, 0, func_001F9850_2FBDF0(10));
            }
            *(unsigned char *)(data + 0x67) = 0xFA;
            break;
        case 0: case 11:
        default:
            break;
        }
        func_L00_0025E4B0_2FBDF0(moby, data + 0x60);
    }
    *(unsigned char *)(moby + 0xA4) = 0xFF;
    func_L00_0025E590_2FBDF0(moby, data + 0x60);
    if ((*(unsigned char *)(data + 0x2E) & 2) && *(unsigned char *)(moby + 0x20) != 15) {
        if (*(int *)(data + 0x10C) != -1) {
            func_001FFDA0_2FBDF0(*(int *)(data + 0x10C), 0);
            *(int *)(data + 0x10C) = -1;
        }
        *(unsigned char *)(moby + 0x20) = 14;
        return;
    }
    if (*(unsigned char *)(moby + 0x20) >= 2 && *(int *)(D_0013E633 + 0x1ED5) != 8 &&
        (*(int *)(D_0013A5E0_2FBDF0 + 0x2600) & 0x20)) {
        *(int *)(data + 0x178) = 1;
    }
    if (*(int *)(data + 0x12C) != 0 && *(int *)(data + 0x170) != 0) {
        *(int *)(data + 0x12C) = *(int *)(data + 0x12C) - 1;
        if (*(int *)(data + 0x12C) < func_001F9850_2FBDF0(10)) {
            t = func_001FA888_2FBDF0(*(int *)(data + 0x12C));
            t = t / func_001F9878_2FBDF0(10.0f);
            D_L06_0015F4FC_2FBDF0 = 1.0f - t;
        }
        if (*(int *)(data + 0x12C) == 0 && *(unsigned char *)(moby + 0x20) != 1) {
            if (10.0f < *(float *)(data + 0x20)) {
                *(float *)(data + 0x20) = 19.9f;
                *(unsigned char *)(moby + 0x20) = 3;
                *(int *)(data + 0x170) = 0;
                qcopy(moby + 0x10, data + 0xC0);
                ang = func_L00_001FF860_2FBDF0(*(float *)(D_0013E633 + 0xE9D) - *(float *)(moby + 0x10),
                                               *(float *)(D_0013E633 + 0xEA1) - *(float *)(moby + 0x14));
                *(float *)(moby + 0x48) = ang;
                func_001FA1F8_2FBDF0(moby + 0xC0, moby + 0x40);
                if (*(unsigned char *)(moby + 0x53) != 6) {
                    func_00213DE0_2FBDF0(moby, 6, 0, func_001F9850_2FBDF0(5));
                }
                func_L06_002FC578_2FBDF0(moby);
            } else {
                *(float *)(data + 0x20) = 9.9f;
                *(unsigned char *)(moby + 0x20) = 5;
                *(int *)(data + 0x170) = 0;
                base = D_L06_0016016C + (*(int *)(data + 0xF8) << 7);
                func_L00_00217718_2FBDF0(base + 0x30, base + 0x70, 0x72, 1);
                qcopy(moby + 0x10, data + 0xC0);
                ang = func_L00_001FF860_2FBDF0(*(float *)(D_0013E633 + 0xE9D) - *(float *)(moby + 0x10),
                                               *(float *)(D_0013E633 + 0xEA1) - *(float *)(moby + 0x14));
                *(float *)(moby + 0x48) = ang;
                func_001FA1F8_2FBDF0(moby + 0xC0, moby + 0x40);
                if (*(unsigned char *)(moby + 0x53) != 6) {
                    func_00213DE0_2FBDF0(moby, 6, 0, func_001F9850_2FBDF0(5));
                }
                func_L06_002FC578_2FBDF0(moby);
            }
        }
    } else if (*(int *)(data + 0x170) == 0) {
        if (*(int *)(data + 0x12C) < func_001F9850_2FBDF0(10)) {
            *(int *)(data + 0x12C) = *(int *)(data + 0x12C) + 1;
            t = func_001FA888_2FBDF0(*(int *)(data + 0x12C));
            t = t / 10.0f;
            D_L06_0015F4FC_2FBDF0 = 1.0f - t;
        }
    }
    *(float *)(data + 0x128) =
        func_001FA748_2FBDF0(*(float *)(data + 0x128), D_L06_00161F9C_2FBDF0 * 0.017453292f * D_0015EE6C);
    *(int *)(moby + 0x90) = func_001FA8A8_2FBDF0(D_L06_00161FA0_2FBDF0, D_L06_00161FA4_2FBDF0,
                                                 (func_001F9FA8_2FBDF0(*(float *)(data + 0x128)) + 1.0f) * 0.5f);
    path = D_L06_001B0FB0_2FBDF0[*(int *)(data + 0xEC)];
    func_L00_00260FB0_2FBDF0(255.0f, moby, data + 0x70, 0, 0, path + 4, *path);
    if (*(int *)(data + 0xB0) == 0) {
        *(int *)(data + 0xB0) = *(int *)(D_0013E633 + 0x2E9D);
    }
}
