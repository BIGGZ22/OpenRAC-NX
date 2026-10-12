/* func_L00_003F8DC8: 72 bytes against 64 (retail uses $gp for the global at 0x1B0A20 via "sw ...,0x1A30($gp)"; this file builds -G0, so the sda global becomes lui/sw). Needs the file-level -G8 that the file does not have; not matched. */
extern char *func_0029EFD0(int size);

extern char *D_L00_001B0A20 __attribute__((sda));

char *func_L00_003F8DC8(void)
{
    char *p = func_0029EFD0(0xD04);

    if (p != 0) {
        D_L00_001B0A20 = p;
        p[0x20] = 1;
        p[0x30] = 0xFF;
        *(short *)(p + 0x32) = 0xFF;
    }
    return p;
}
