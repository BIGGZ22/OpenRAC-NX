/* 120 bytes vs retail 112 (30 instructions vs 28), and not reachable as written: retail loads two words off $gp in ordinary flow (lw v0,0xA28($gp), lw v1,0xA2C($gp)) and cc1 emits lui+lw of their addresses instead (same boundary as func_L00_003B8138, docs/COMPILER-NOTES.md). The loop is index-based here; retail walks three pointers down a counter. */
extern short D_L00_001B28B0[] __attribute__((sda));
extern short D_L00_001B28C0[];
extern short D_L00_001B28C8[];
extern int D_L00_001B28D0;
extern int D_L00_001B28D4;
extern int D_L00_001AFA18 __attribute__((sda));
extern int D_L00_001AFA1C __attribute__((sda));
extern float D_L00_001B22D8 __attribute__((sda));

void func_L00_003B4E88(void)
{
    int i;

    for (i = 3; i >= 0; i--) {
        D_L00_001B28B0[3 - i] = 2;
        D_L00_001B28C0[3 - i] = 2;
        D_L00_001B28C8[3 - i] = 1;
    }
    D_L00_001B28D0 = D_L00_001AFA18;
    D_L00_001B28D4 = D_L00_001AFA1C;
    D_L00_001B22D8 = 1.0f;
}
