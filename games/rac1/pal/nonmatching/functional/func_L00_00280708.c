/* functional: medium. Pause menu step, always returns 0. State D_0015EFB0 not 1/0x10: B[2] = B[1]->0x38. Otherwise reads pad flags at 0x13CBE4: bit 0x20 clears B+0xD4, B[2] = B[1]->0x38, B[1]->0x84 = 1; else bit 0x10 does B[2] = B[1]->0x38, B[1]->0x84 = 0. */
extern int D_0015EFB0_80708 __asm__("D_0015EFB0") MACRO_ADDR;
extern char D_L00_001BA070_80708[] __asm__("D_L00_001BA070");
extern int D_0013CBE4_80708 __asm__("D_0013CBE4");
int func_L00_00280708(void) {
    char *b = D_L00_001BA070_80708;
    int s = D_0015EFB0_80708;
    int flags;
    char *p;
    if (s != 0x10 && s != 1) {
        p = *(char **)(b + 4);
        *(int *)(b + 8) = *(int *)(p + 0x38);
        return 0;
    }
    flags = D_0013CBE4_80708;
    if (flags & 0x20) {
        *(int *)(b + 0xD4) = 0;
        p = *(char **)(b + 4);
        *(int *)(b + 8) = *(int *)(p + 0x38);
        *(int *)(p + 0x84) = 1;
    } else if (flags & 0x10) {
        p = *(char **)(b + 4);
        *(int *)(b + 8) = *(int *)(p + 0x38);
        *(int *)(p + 0x84) = 0;
    }
    return 0;
}
