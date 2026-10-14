/* functional: medium. Update callback of the race-timer-style HUD counter element: eases its smoothed value (0x74) toward *value, then runs a 4-state machine (0 idle, 1 counting bonus, 2/3 hold and fade) over the hero's bonus/counter words. Checked statically only. */
extern char D_0013F450_63500[] __asm__("D_0013F450");
#define HI(o) (*(int *)(D_0013F450_63500 + (o)))
extern int D_L05_0015FB8C_63500 __asm__("D_L05_0015FB8C");
extern int D_L05_0015FB90_63500 __asm__("D_L05_0015FB90");
extern int D_L05_0015FB98_63500[] __asm__("D_L05_0015FB98");
extern int D_L05_0015FBA4_63500 __asm__("D_L05_0015FBA4");
extern int D_L05_0015FBA8_63500 __asm__("D_L05_0015FBA8");
extern int D_L05_0015FBAC_63500 __asm__("D_L05_0015FBAC");
extern int D_L05_0015FBB0_63500 __asm__("D_L05_0015FBB0");
extern int D_L05_0015FBB4_63500 __asm__("D_L05_0015FBB4");
extern int D_L05_0015FBB8_63500 __asm__("D_L05_0015FBB8") MACRO_ADDR;
extern unsigned char D_L05_0015F95C_63500 __asm__("D_L05_0015F95C") MACRO_ADDR;
extern float func_00214D28_63500(float *, float, float) __asm__("func_00214D28");
extern int func_001FA898_63500(float) __asm__("func_001FA898");
extern int func_001F9850_63500(int) __asm__("func_001F9850");

void func_L05_00263500(char *e)
{
    
    unsigned char *c = (unsigned char *)(e + 0x70);
    float value;
    int i;
    int *dst;
    int limit;

    value = (float)*(int *)(e + 0x74);
    func_00214D28_63500(&value, (float)**(int **)(e + 0xC), 6.0f);
    *(int *)(e + 0x74) = func_001FA898_63500(value);

    switch (c[2]) {
    case 0: {
        if ((*(short *)(D_0013F450_63500 + 0x30E)) < 6)
            return;
        limit = HI(0x70C);
        if (!(1.0f <= (float)limit))
            return;
        D_L05_0015FB8C_63500 = limit;
        D_L05_0015FB90_63500 = 0;
        dst = &D_L05_0015FBA4_63500;
        for (i = 3; i >= 0; i--)
            *dst-- = 0;
        c[2] = 1;
        c[1] = D_L05_0015F95C_63500;
        c[0] = 0;
        break;
    }
    case 1: {
        if (c[0] < 10)
            c[0]++;
        if (c[1] != 0)
            c[1]--;
        D_L05_0015FB90_63500 = HI(0x6F0);
        for (i = 0; i < 4; i++)
            D_L05_0015FB98_63500[i] = HI(0x6E0 + i * 4);
        limit = HI(0x70C);
        if (D_L05_0015FB8C_63500 < limit) {
            D_L05_0015FB8C_63500 = limit;
            c[1] = D_L05_0015F95C_63500;
        }
        if (HI(0x300) == 0) {
            if ((unsigned int)(HI(0x2084) - 0x6D) > 2)
                return;
            D_L05_0015FBB8_63500 = 0;
        } else {
            D_L05_0015FBB8_63500 = 0;
        }
        HI(0x70C) = 0;
        HI(0x6F0) = 0;
        for (i = 3; i >= 0; i--)
            HI(0x6E0 + i * 4) = 0;
        for (i = 2; i >= 0; i--)
            HI(0x700 + i * 4) = 0;
        if (HI(0x2084) == 0x6B) {
            if (D_L05_0015FB90_63500 > 2) {
                int a = D_L05_0015FBB0_63500;
                int b = D_L05_0015FBAC_63500;
                int cc = D_L05_0015FBA8_63500;
                D_L05_0015FBB4_63500 = a;
                D_L05_0015FBB0_63500 = b;
                D_L05_0015FBAC_63500 = cc;
                D_L05_0015FBA8_63500 = -1;
            }
            c[2] = 3;
            c[1] = D_L05_0015F95C_63500;
            c[0] = func_001F9850_63500(0x78);
        } else {
            c[2] = 2;
            c[1] = D_L05_0015F95C_63500;
            c[0] = func_001F9850_63500(0x3C);
        }
        break;
    }
    case 2:
    case 3: {
        if (c[0] == 0) {
            D_L05_0015FBB8_63500 = 0;
            D_L05_0015FB8C_63500 = 0;
            D_L05_0015FB90_63500 = 0;
            dst = &D_L05_0015FBA4_63500;
            for (i = 3; i >= 0; i--)
                *dst-- = 0;
            c[2] = 0;
            c[0] = 0;
        } else if (c[2] == 3) {
            unsigned char t = c[0];
            if (t == func_001F9850_63500(0x6E)) {
                int add = D_L05_0015FBB8_63500;
                HI(0x8A8) = HI(0x8A8) + add;
                HI(0x864) = HI(0x864) + add * 5;
                if ((int)((float)func_001F9850_63500(0x11) * 60.0f) < HI(0x864))
                    HI(0x864) = (int)((float)func_001F9850_63500(0x11) * 60.0f);
            }
        }
        c[0] = c[0] - 1;
        break;
    }
    }
}
