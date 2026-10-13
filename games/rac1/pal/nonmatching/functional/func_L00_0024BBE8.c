/* functional: medium. Written as one body with func_L00_0024BC04, func_L00_0024BC2C and func_L00_0024BC38 (the head runs on into them). If D_0013D390[7] is set, D_0015EFB0 = 3. Else with flags = D_0015EFB4: bit 0x20 set -> clear it, D_0015EFB0 = 0x18 if D_L00_0015F688 is nonzero else 12; else bit 0x10 set -> clear it, D_0015EFB0 = 14. */
extern int D_L00_0015F688_4BBE8 __asm__("D_L00_0015F688");
void func_L00_0024BBE8(void) {
    int flags;
    if (D_0013D390[7] != 0) {
        D_0015EFB0 = 3;
        return;
    }
    flags = D_0015EFB4;
    if (flags & 0x20) {
        D_0015EFB4 = flags ^ 0x20;
        if (D_L00_0015F688_4BBE8 != 0) {
            D_0015EFB0 = 0x18;
        } else {
            D_0015EFB0 = 12;
        }
        return;
    }
    if (flags & 0x10) {
        D_0015EFB4 = flags ^ 0x10;
        D_0015EFB0 = 14;
    }
}
