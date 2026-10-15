/* NON_MATCHING func_L00_002477A0 -- src/overlays/shared/map_002465F8.c
 * Best so far: SIZE ours 80 / retail 84, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Finds a value in a null terminated map list of at most twenty entries; earlier closest candidate p4.c is 80 by
 *   Resume candidate p6.c advances separate pointers and increments the index before checking the next sentinel, m
 *   Infrastructure wall: Docker socket /Users/flavy/.orbstack/run/docker.sock is missing; compilation never starts
 *   Resumed after Docker restart; map list lookup exhausted eight runs. p8.c reaches 84 bytes with 13 byte differe
 *   Remaining differences swap index/current-pointer registers and schedule sentinel load before pointer increment
 *   A source ordering that preserves index allocation while delaying its initialization and moving pointer increme
 */
extern int D_L00_00160360 MACRO_ADDR;

// Finds a value in the active map list.
int func_L00_002477A0(int value) {
    int *entry = (int *)D_L00_00160360;
    int i = 0;
    if (*entry == 0) {
        return -1;
    }
    for (;;) {
        if (*entry == value) return i;
        entry++;
        i++;
        if (i >= 20 || *entry == 0) return -1;
    }
}
