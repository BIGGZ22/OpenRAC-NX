/* functional: high. Written from the retail instructions of func_L03_002DDDF8 and the pieces it branches into (002DDE28, 002DDE38, 002DDE9C, 002DDED8: one 252-byte function); not a match. */
/* Level 3. A moby that rises to 4 units above its home: state 0 keeps its position as home and
 * starts (state 1) once its unlock bit is set; state 1 climbs by D_0015EE6C a tick until 4 above
 * home (state 2); state 2 holds it there. */
extern unsigned char D_L03_001BB640_2DDDF8[] __asm__("D_L03_001BB640");
extern int D_0015EE84_2DDDF8 __asm__("D_0015EE84");
extern float D_0015EE6C_2DDDF8 __asm__("D_0015EE6C");

void func_L03_002DDDF8(char *m) {
    char *d = *(char **)(m + 0x78);
    int st = *(unsigned char *)(m + 0x20);

    if (st == 1) {
        *(float *)(m + 0x18) = *(float *)(m + 0x18) + D_0015EE6C_2DDDF8;
        if (*(float *)(d + 8) + 4.0f <= *(float *)(m + 0x18)) {
            *(unsigned char *)(m + 0x20) = 2;
        }
    } else if (st == 2) {
        *(float *)(m + 0x18) = *(float *)(d + 8) + 4.0f;
    } else if (st == 0) {
        unsigned short id = *(unsigned short *)(m + 0xB2);
        int *words = (int *)0x14C290;

        *(float *)(d + 0x0) = *(float *)(m + 0x10);
        *(float *)(d + 0x4) = *(float *)(m + 0x14);
        *(float *)(d + 0x8) = *(float *)(m + 0x18);
        *(float *)(d + 0xC) = *(float *)(m + 0x1C);
        if (D_L03_001BB640_2DDDF8[(short)id + 0x454] != 0 ||
            ((words[((short)id >> 5) + D_0015EE84_2DDDF8 * 64] >> (id & 0x1F)) & 1) != 0) {
            *(unsigned char *)(m + 0x20) = 1;
        }
    }
}
