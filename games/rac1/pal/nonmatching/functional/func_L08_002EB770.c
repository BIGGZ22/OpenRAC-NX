/* functional: medium. Swarm gate-part state machine (0: remember yaw, 1: follow driver progress / trigger on hero-class, 2: ease to target); with the flag at D_0013D490+0x2E set and driver unset, wakes every moby listed in the level table. Checked statically/by calls. */
extern unsigned char D_0013D355_2EB770[] __asm__("D_0013D355");
extern unsigned short *D_L08_001AC340_2EB770[] __asm__("D_L08_001AC340");
extern char *D_L08_00160058_b_2EB770 __asm__("D_L08_00160058");
extern float D_0015EE6C_2EB770 __asm__("D_0015EE6C");
extern float func_00214D28_2EB770(float *, float, float) __asm__("func_00214D28");

void func_L08_002EB770(char *m)
{
    char *d = *(char **)(m + 0x78);
    char *mobys;
    char *o;
    int i;
    unsigned char *g = D_0013D355_2EB770 + 0x13B;
    unsigned char st;

    if (g[0x2E] != 0 && *(int *)(d + 8) < 0) {
        unsigned short *p = D_L08_001AC340_2EB770[*(unsigned char *)(m + 0x21)];
        if (p == 0)
            return;
        for (;;) {
            unsigned int k = *p;
            o = D_L08_00160058_b_2EB770 + ((k & 0x7FFF) << 8);
            if (*(unsigned char *)(o + 0x20) < 0x7F && o != m) {
                unsigned int h = *(unsigned short *)(o + 0xA6);
                if (h - 0x24C < 0xB) {
                    func_0020D678_2EAF48(o);
                } else if ((short)h == 0x1D8) {
                    func_0020D678_2EAF48(o);
                }
            }
            if (*(short *)p < -1)
                break;
            p++;
        }
        func_0020D678_2EAF48(m);
        return;
    }
    st = *(unsigned char *)(m + 0x20);
    switch (st) {
    case 0:
        *(float *)(d + 4) = *(float *)(m + 0x18);
        *(unsigned char *)(m + 0x20) = 1;
        break;
    case 1:
        i = *(int *)(d + 8);
        if (i < 0)
            return;
        mobys = (char *)D_L08_00160058_2EAF48;
        o = mobys + (i << 8);
        if (*(short *)(o + 0xA6) == 0x267 || *(short *)(o + 0xA6) == 0x23F) {
            if (*(unsigned char *)(o + 0x20) != 4)
                return;
            g[0x35] = st;
            func_0022ED80_2EAF48(0, 0, (struct Moby *)m);
            *(unsigned char *)(m + 0x20) = 2;
        } else {
            float a = *(float *)(d + 4);
            func_00214D28_2EB770((float *)(m + 0x18),
                (*(float *)d - a) * **(float **)(o + 0x78) + a,
                D_0015EE6C_2EB770 * 10.0f);
        }
        break;
    case 2:
        func_00214D28_2EB770((float *)(m + 0x18), *(float *)d, D_0015EE6C_2EB770 * 10.0f);
        break;
    }
}
