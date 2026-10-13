/* functional: high. Written from the retail instructions of func_L01_0026EFB8 and its tail func_L01_0026EFDC (joined in config/overlays/joined.tsv, 132 bytes); not a match. */
/* Level code. Counts the live mobys of group `group` (a list of moby indices, ended by an entry with
 * bit 15 set, which is still counted) that are not in state `state`, or all of them for state -1. */
extern unsigned short *D_L01_001ABFC0_26EFB8[] __asm__("D_L01_001ABFC0");
extern char *D_L01_00160058_26EFB8 __asm__("D_L01_00160058");

int func_L01_0026EFB8(int group, int state) {
    unsigned short *list = D_L01_001ABFC0_26EFB8[group];
    int count = 0;
    unsigned int v;

    if (list == 0) {
        return 0;
    }
    do {
        char *m;

        v = *list;
        m = D_L01_00160058_26EFB8 + ((v & 0x7FFF) << 8);
        if (*(signed char *)(m + 0x20) >= 0 && (state == -1 || *(unsigned char *)(m + 0x20) != state)) {
            count++;
        }
        list++;
    } while ((short)v >= 0);
    return count;
}
