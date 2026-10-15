/* NON_MATCHING func_L00_00247620 -- src/overlays/shared/map_002465F8.c
 * Best so far: SIZE ours 72 / retail 68, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Fragment: first half of one function that scans 5 slots (stride 4, D_L00_001842F0+0x278.. and +0x28C) for one 
 *   One C function spans 247620+247650 (~68 bytes); neither half matches alone. Needs the symbol split merged.
 *   Scans five records at D_L00_001842F0+0x28C, stepping 4 bytes, and returns the matching index or -1. Two compil
 *   Resumed joined loop: returns the first active matching slot among five, or -1. New p7.c uses a shared goto loo
 *   Resumed through run 8/8: scans five active slots for the requested key. p8.c matches the joined loop body; onl
 *   Stopped at budget. Unblock by finding a plain-C declaration/address expression retaining the separate base loa
 */
extern char D_L00_001842F0[];

/* returns the first active key match, or -1 when none is found */
int func_L00_00247620(int id) {
    char *base = D_L00_001842F0;
    char *p = base + 0x28C;
    int index = 0;
    do {
        int active = *(int *)(p - 0x14);
        int key = *(int *)p;
        if (active && key == id)
            return index;
        index++;
        p = p + 4;
    } while (index != 5);
    return -1;
}
