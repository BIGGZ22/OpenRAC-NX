/* functional: high. From the near miss, checked against the retail instructions (with its joined
 * tail func_L05_0031AB00, the "found" return). Returns the moby itself instead of recomputing its
 * address from the list entry; the value is the same. */
extern int D_L14_001600B4_2F2778 __asm__("D_L14_001600B4");
extern unsigned short *D_L14_001AC2C0_2F2778[] __asm__("D_L14_001AC2C0");
extern char *D_L14_00160098_2F2778 __asm__("D_L14_00160098");

/* The first moby of group moby[0x21] whose variables' state (+0xB4) is STATE, or 0. The group's
 * list holds moby indexes (low 15 bits); the entry with the top bit set is the last. Moby is
 * vendor_002E0538.c's, which the port reads this into. */
Moby *func_L14_002F2778(char *moby, int state) {
    unsigned char group = (unsigned char)moby[0x21];
    char *base;
    char *m;
    unsigned short *p;
    unsigned short id;

    if (group >= D_L14_001600B4_2F2778) {
        return 0;
    }
    p = D_L14_001AC2C0_2F2778[group];
    if (p == 0) {
        return 0;
    }
    base = D_L14_00160098_2F2778;
    do {
        id = *p;
        m = base + ((id & 0x7FFF) << 8);
        if (*(short *)(*(char **)(m + 0x78) + 0xB4) == state) {
            return (Moby *)m;
        }
        p++;
    } while ((short)id >= 0);
    return 0;
}
