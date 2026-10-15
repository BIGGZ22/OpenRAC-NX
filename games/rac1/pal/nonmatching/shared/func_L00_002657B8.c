/* NON_MATCHING func_L00_002657B8 -- src/overlays/shared/mobyutil_00265558.c
 * Best so far: SIZE ours 256 / retail 260, checked 2026-10-15.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   2026-10-15 cnobis: state read once per round (u8 local) and D_L00_001600A4/D_L00_00160098 MACRO_ADDR match
 *   retail. Only the update call differs: retail "beqz fn; nop; daddu a0; jalr fn; nop", ours fills both slots
 *   (beql + lhu). Tried fn local/cast/unprototyped, for/while/continue, if/else order, separate loop vars: all
 *   256 of 260.
 */

/*
 * Builds the frame's moby update list from the level's moby array, then
 * walks that list calling each moby's per-frame update hooks.
 */

typedef struct MobyRecord {
    char pad0[0x20];
    signed char state; /* +0x20 */
    char pad1[0x28 - 0x21];
    struct MobyRecord *next; /* +0x28 */
    char pad2[0x34 - 0x2C];
    unsigned short flags; /* +0x34 */
    char pad3[0x74 - 0x36];
    void (*updateFn)(struct MobyRecord *); /* +0x74 */
    char pad4[0x100 - 0x78];
} MobyRecord;

extern int D_L00_001ABD80;
extern MobyRecord *D_L00_001600A4 MACRO_ADDR;
extern MobyRecord *D_L00_00160098 MACRO_ADDR;

extern void func_L00_002514B8(MobyRecord *);
extern void func_L00_00251E30(MobyRecord *);

void func_L00_002657B8(void)
{
    MobyRecord *cur;
    MobyRecord *prev;
    unsigned char state;
    MobyRecord *m;
    void (*fn)(MobyRecord *);

    D_L00_001ABD80 = 0;
    D_L00_001600A4 = 0;

    cur = D_L00_00160098;
    prev = 0;
    state = cur->state;
    while (state != 0xFF) {
        if (!(state & 0x80) && !(cur->flags & 0x2)) {
            if (prev != 0)
                prev->next = cur;
            else
                D_L00_001600A4 = cur;
            prev = cur;
        }
        cur++;
        state = cur->state;
    }
    if (prev != 0)
        prev->next = 0;

    m = D_L00_001600A4;
    if (m != 0) {
        do {
            if (m->state >= 0) {
                if (!(m->flags & 0x40)) {
                    func_L00_002514B8(m);
                }
                fn = m->updateFn;
                if (fn != 0) {
                    fn(m);
                }
                if (!(m->flags & 0x4)) {
                    func_L00_00251E30(m);
                }
            }
            m = m->next;
        } while (m != 0);
    }
}
