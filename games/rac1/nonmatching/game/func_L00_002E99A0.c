extern char *D_L00_00166F00;
extern char *D_L00_001690C0;

/* Sets the state's record at 0x40 of its data: the word at 0xDA becomes 1,
   0xB8 gets y, and 0xB4 gets x (flag 0), or the global's 0xF0 float plus x
   (flag set). Does nothing while the state's flag at 0x86 is set. */
void func_L00_002E99A0(int flag, float x, float y) {
    char *moby = D_L00_00166F00;
    char *p;

    if (*(short *)(moby + 0x86) != 0) {
        return;
    }
    p = *(char **)(moby + 0x70) + 0x40;
    *(short *)(p + 0xDA) = 1;
    *(float *)(p + 0xB8) = y;
    if (flag == 0) {
        *(float *)(p + 0xB4) = x;
    } else {
        char *g = D_L00_001690C0;
        *(float *)(p + 0xB4) = *(float *)(g + 0xF0) + x;
    }
}
