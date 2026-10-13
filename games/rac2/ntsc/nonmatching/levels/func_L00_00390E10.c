/* Restores the third-level object's transform and flags from the level's own globals, marks
 * the level as initialised, publishes the default geometry constants and calls the boot
 * helper with the three default words. */
void func_L00_00390E10(char *object)
{
    char *c = *(char **)(object + 0x68);
    float f0 = D_L00_001B1984;
    float f1 = D_L00_001B1988;
    float f2 = D_L00_001B198C;
    float f3 = D_L00_001B1990;

    D_L00_001A8A44 = 1;
    *(float *)(c + 0x30) = f0;
    *(float *)(c + 0x34) = f1;
    *(float *)(c + 0x38) = f2;
    *(float *)(c + 0x3C) = f3;
    *(unsigned char *)(c + 0x40) = D_L00_001B1980;
    *(unsigned char *)(c + 0x41) = D_L00_001B1981;
    *(unsigned char *)(c + 0x42) = D_L00_001B1982;
    D_L00_001B1981 = 0x60;
    D_L00_001B1982 = 0x40;
    D_L00_001B1988 = 131072.0f;
    D_L00_001B198C = 255.0f;
    D_L00_001B2500 = 147456.0f;
    D_L00_001B2408 = 144.0f;
    D_L00_001B1EB4 = 144;
    D_L00_001B20C0 = 0x50000;
    D_L00_001B1980 = 0x40;
    D_L00_001B255C = 144;
    D_L00_001B1984 = 0.0f;
    func_00284E18(0x40, 0x60, 0x40);
    D_L00_001B1990 = 0.0f;
}
