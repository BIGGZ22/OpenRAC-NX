/* Publishes the third-level object's transform and flags into the level's own globals, then,
 * in the second mode, publishes five more constants and calls the boot helper with three
 * words of the 0x1BD2C0 array. */
void func_L00_00390F28(char *object, int mode)
{
    char *c = *(char **)(object + 0x68);
    float f0;
    float f1;
    float f2;
    float f3;

    D_L00_001A8A44 = 0;
    /* the four transform words are copied as a unit: the loads then the stores */
    f0 = *(float *)(c + 0x30);
    f1 = *(float *)(c + 0x34);
    f2 = *(float *)(c + 0x38);
    f3 = *(float *)(c + 0x3C);
    D_L00_001B1984 = f0;
    D_L00_001B1988 = f1;
    D_L00_001B198C = f2;
    D_L00_001B1990 = f3;
    D_L00_001B1980 = *(unsigned char *)(c + 0x40);
    D_L00_001B1981 = *(unsigned char *)(c + 0x41);
    D_L00_001B1982 = *(unsigned char *)(c + 0x42);
    if (mode != 0) {
        D_L00_001B2500 = 983040.0f;
        D_L00_001B2408 = 500.0f;
        D_L00_001B1EB4 = 960;
        D_L00_001B20C0 = 0x1F4000;
        D_L00_001B255C = 960;
        func_00284E18(D_L00_001BD2C0[0x8F], D_L00_001BD2C0[0x90], D_L00_001BD2C0[0x91]);
    }
}
