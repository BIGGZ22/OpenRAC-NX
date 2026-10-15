/* CollType: surface id (bits 0-4) of the last collision hit (CollOutput+0x1c at 0x173F40); -1 when negative or 0x1f. */
extern int D_L00_00173F40[];
int func_L00_001F3958(void) {
    int t = D_L00_00173F40[7];
    int s;
    if (t < 0) {
        return -1;
    }
    s = t & 0x1F;
    if (s == 0x1F) {
        return -1;
    }
    return s;
}
