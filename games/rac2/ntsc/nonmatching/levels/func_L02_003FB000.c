/* 2026-10-20, Opus retry: no natural loop construct was found. An EMPTY `do { } while (0);`
   anywhere between the main loop and the return also gives 1 of 1 (its loop note brings the
   alignment nop before the shared return label); it is held back, since an empty loop exists
   only to steer the compiler. Wrapping the last (timestamp) store instead also matches, but
   the neighbour func_L02_003FB120 stores the same timestamp with its load moved above earlier
   stores, which a loop note there would block, so that spelling is unlikely to be the
   original's either. */

/* func_L02_003FB000 (288 bytes, level 2, src/levels/02_maktar_nebula/L02_003F5430.c; two $gp
 * accesses, so it belongs in the -g8 sibling).
 *
 * Bytes: 288 of 288 and `tools/check.py func_L02_003FB000` printed "1 of 1 places match" with the
 * body below, compiled in L02_003F5430-g8.c. It is held back here, not placed in src/, because of
 * the `do { } while (0)` around the last store: that wrapper is the only thing found that
 * produces retail's `nop` at +0x114, and a construct whose only purpose is to steer the compiler
 * is excluded by the rules. The lead can decide to place it as it stands. Precedent in this
 * tree: src/core/00124568.c, func_00124818 (carried from rac1-decomp) wraps a store in
 * `do { } while (0)` for the same mechanism (the loop notes it leaves) and is a landed match.
 * To place it: paste the body below into L02_003F5430-g8.c after func_L02_003F56C0, with its
 * four extern lines, and put the INCLUDE_ASM marker's place in L02_003F5430.c as a comment.
 *
 * Without the wrapper the function is 284 bytes and every instruction matches except the missing
 * `nop` between `sh $v0,0x11E($a0)` and the shared return label (`jr $ra` at +0x118). That nop
 * is cc1's loop alignment, not an assembler hazard: final.c (shorten_branches) gives `.p2align 3`
 * to the first code label that follows ANY NOTE_INSN_LOOP_BEG, and unreachable-code deletion
 * keeps LOOP_BEG/LOOP_END notes. So retail's source had a loop construct (a macro-style
 * do/while(0), or a loop with a constant-false test) after the main loop's last label, and the
 * next label, the return label, got padded to 8 bytes. Measured on this body:
 *   - exact (0 differences): the last store inside `do { } while (0)`; an empty
 *     `do { } while (0);`, `while (0) { }` or `for (;0;) { }` after the last store.
 *   - not exact: wrapping the last two stores (the `sh $zero,0x11C` store moves), or all three
 *     (the `lui` of D_00189E20 is hoisted above the main loop).
 *   - the main loop as `for (;;) { ... if (*p < 0) break; p++; }` or `while (1) { ...
 *     if (*p++ < 0) break; }` does not add the pad by itself (and the first costs more).
 * Everything else was settled by measurement: the tail statement order must be f68 store, then
 * h11C, then h11E (the other five orders move the stores: 1 to 12 differing words); `g` as a
 * separate pointer keeps `lui; addiu; lwc1 0x24CC($v0)` (without it cc1 folds the offset into
 * the lwc1 and the function is 4 bytes shorter again); the six type tests as an `||` chain give
 * retail's `beql` ladder with the constants held in $t0-$t7.
 *
 * Meaning: the object's list-state at +0x68 gets a count (+0x60) and a sum (+0x64) of the scale
 * float (+0x20 of each listed entity's own state) over the entities of six kinds in the list
 * D_L02_001FC060[index] (kinds 0x9CA, 0xB22, 0xC3B, 0xC3E, 0x9F8, 0xA44), then +0x68 is set to
 * the float of the word at D_00189E20+0x24CC, +0x11C to 0 and +0x11E to the halfword D_L02_001B1214.
 */
extern char *D_L02_001B359C __attribute__((sda));
extern unsigned short D_L02_001B1214 __attribute__((sda));
extern unsigned int D_L02_001FC060[];
extern char D_00189E20[];

void func_L02_003FB000(char *a0) {
    char *state = *(char **)(a0 + 0x68);
    unsigned int index = *(unsigned int *)(state + 0x40);
    unsigned int entry;
    short *p;
    char *tbl;
    char *rec;
    char *g;

    *(int *)(state + 0x60) = 0;
    *(float *)(state + 0x64) = 0;
    if (index == 0xFF)
        return;
    entry = D_L02_001FC060[index];
    if (entry == 0)
        return;
    p = (short *)entry;
    tbl = D_L02_001B359C;
    do {
        rec = tbl + ((*(unsigned short *)p & 0x7FFF) << 8);
        if (rec == 0)
            continue;
        if (*(unsigned char *)(rec + 0x20) == 0xFE)
            continue;
        if (*(unsigned char *)(rec + 0x20) == 0xFD)
            continue;
        if (*(unsigned char *)(rec + 0x20) == 1)
            continue;
        {
            short type = *(short *)(rec + 0xAA);
            if (type == 0x9CA || type == 0xB22 || type == 0xC3B || type == 0xC3E || type == 0x9F8 || type == 0xA44) {
                *(int *)(state + 0x60) += 1;
                *(float *)(state + 0x64) += *(float *)(*(char **)(rec + 0x68) + 0x20);
            }
        }
    } while (*p++ >= 0);
    g = D_00189E20;
    *(float *)(state + 0x68) = (float)*(int *)(g + 0x24CC);
    *(short *)(state + 0x11C) = 0;
    do {
        *(unsigned short *)(state + 0x11E) = D_L02_001B1214;
    } while (0);
}
