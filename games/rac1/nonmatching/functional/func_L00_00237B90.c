/* functional: medium. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). HUD race-timer / message banner: draws a one or two line text banner with a 3-piece frame (alpha fades with the byte at e+0x70), and on levels 5 and 0x10 when the camera is near a fixed point also prints the saved race time and score; returns e->0x58. */
typedef struct { float x, y, z, w; } __attribute__((aligned(16))) HudPt_37B90;
extern float func_001FA888_37B90(int) __asm__("func_001FA888");
extern int func_001FA898_37B90(float) __asm__("func_001FA898");
extern float func_001F9878_37B90(float) __asm__("func_001F9878");
extern int func_001F6600_37B90(unsigned char *, int) __asm__("func_001F6600");
extern int func_00200198_37B90(int, int) __asm__("func_00200198");
extern void func_00200468_37B90(int, int, int, int, int, int) __asm__("func_00200468");
extern void func_00200650_37B90(int, int, int, int, int, int) __asm__("func_00200650");
extern void func_001F6EA8_37B90(int, int, int, unsigned char *, int) __asm__("func_001F6EA8");
extern float func_001F9D10_37B90(void *, void *) __asm__("func_001F9D10");
extern char *func_001FE540_37B90(int) __asm__("func_001FE540");
extern int func_00116248_37B90(char *, char *, ...) __asm__("func_00116248");
extern void func_00201A38_37B90(int, int, int, char *) __asm__("func_00201A38");
extern int D_0015EE80_37B90 __asm__("D_0015EE80") MACRO_ADDR;
extern int D_0015EE84_37B90 __asm__("D_0015EE84") MACRO_ADDR;
extern int D_L00_0015F850_37B90 SDATA(D_L00_0015F850);
extern unsigned char D_L00_0017E630_37B90[] __asm__("D_L00_0017E630");
extern float D_0013F4D0_37B90[] __asm__("D_0013F4D0");
extern int D_0015EF58_37B90[] __asm__("D_0015EF58");
extern int D_0015EF68_37B90[] __asm__("D_0015EF68");
extern int D_L00_0015F728_37B90 __asm__("D_L00_0015F728");
extern char D_L00_001E8840_37B90[] __asm__("D_L00_001E8840");
extern char D_L00_0015F858_37B90[] __asm__("D_L00_0015F858");
int func_L00_00237B90(char *e) {
    int y, i, alpha, color, width, left, w2, l2, level, off;
    float fade;
    char text[64];
    unsigned char *opacity = (unsigned char *)e + 0x70;
    unsigned char *buf = (unsigned char *)D_L00_0017E630_37B90;

    if (D_0015EE80_37B90 != 0) {
        y = D_L00_0015F850_37B90 + 0xA;
    } else {
        y = D_L00_0015F850_37B90 + 0x12;
    }
    fade = func_001FA888_37B90(*opacity);
    fade /= func_001F9878_37B90(8.0f);
    if (fade < 0.0f) fade = 0.0f;
    if (1.0f < fade) fade = 1.0f;
    alpha = func_001FA898_37B90(fade * 128.0f);
    color = (alpha << 24) | 0x40F040;
    i = 0;
    if (D_L00_0017E630_37B90[0] > 1) {
        do {
            i++;
        } while (D_L00_0017E630_37B90[i] > 1 && i < 0x80);
    }
    if (buf[i] == 1) {
        buf[i] = 0;
        width = func_001F6600_37B90(buf, -1);
        left = 0xE0 - (width >> 1);
        w2 = func_001F6600_37B90(&buf[1] + i, -1);
        l2 = 0xE0 - (w2 >> 1);
        if (left < l2) {
            l2 = left;
        } else {
            width = w2;
        }
        left = l2;
        func_00200468_37B90(func_00200198_37B90(0x7580, 0), left + 0x20, y, width, 0x36, alpha);
        func_00200468_37B90(func_00200198_37B90(0x7580, 1), left, y, 0x20, 0x36, alpha);
        func_00200650_37B90(func_00200198_37B90(0x7580, 1), left + (width + 0x20), y, 0x20, 0x36, alpha);
        func_001F6EA8_37B90(0x100, y + 8, color, buf, -1);
        func_001F6EA8_37B90(0x100, y + 0x1B, color, &buf[1] + i, -1);
        level = D_0015EE84_37B90;
    } else {
        width = func_001F6600_37B90(buf, -1);
        left = 0xE0 - (width >> 1);
        func_00200468_37B90(func_00200198_37B90(0x7580, 1), left, y, 0x20, 0x20, alpha);
        func_00200468_37B90(func_00200198_37B90(0x7580, 0), left + 0x20, y, width, 0x20, alpha);
        func_00200650_37B90(func_00200198_37B90(0x7580, 1), left + (width + 0x20), y, 0x20, 0x20, alpha);
        func_001F6EA8_37B90(0x100, y + 8, color, buf, -1);
        level = D_0015EE84_37B90;
    }
    if (level == 5 || level == 0x10) {
        HudPt_37B90 pts[2];
        HudPt_37B90 pt;
        int elapsed, score, interval, unit, mins, rem, secs, hund;
        pts[0].x = 0.0f; pts[0].y = 0.0f; pts[0].z = 0.0f; pts[0].w = 0.0f;
        pts[1] = pts[0];
        pts[0].x = 294.5f; pts[0].y = 225.4f; pts[0].z = 100.0f;
        pts[1].x = 115.7f; pts[1].y = 287.0f; pts[1].z = 119.5f;
        if (level == 5) {
            pt = pts[0];
        } else {
            pt = pts[1];
        }
        if (func_001F9D10_37B90(D_0013F4D0_37B90, &pt.x) < 5.0f) {
            D_L00_0015F728_37B90 = y + 0x64;
            off = (level == 0x10) ? 4 : 0;
            elapsed = *(int *)((char *)D_0015EF58_37B90 + off);
            if (elapsed != 0) {
                score = *(int *)((char *)D_0015EF68_37B90 + off);
                interval = (D_0015EE80_37B90 != 0) ? 3000 : 3600;
                unit = interval / 60;
                mins = elapsed / interval;
                rem = elapsed - mins * interval;
                secs = rem / unit;
                rem = rem - secs * unit;
                hund = (rem * 100) / unit;
                func_00116248_37B90(text, D_L00_001E8840_37B90, func_001FE540_37B90(0x5320), mins, secs, hund);
                func_00201A38_37B90(0x100, y + 0x28, color, text);
                if (score != 0) {
                    func_00116248_37B90(text, D_L00_0015F858_37B90, func_001FE540_37B90(0x5321), score);
                    func_00201A38_37B90(0x100, y + 0x48, color, text);
                }
            }
        }
    }
    return *(int *)(e + 0x58);
}
