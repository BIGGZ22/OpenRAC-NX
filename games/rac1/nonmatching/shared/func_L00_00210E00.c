/* NON_MATCHING func_L00_00210E00 -- src/overlays/shared/help_0020CDF0.c
 * Best so far: BYTES 117/480 (75.6% of the bytes match), checked 2026-10-15.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   2026-10-15 cnobis: right size with hero.items[i] slots (slot pointer), switch on state, index loop over
 *   unk18F0[3]. State 3 as one call "if ((obj[0x70] & 2) || i == 1 || i == 0)" gives retail's single call site
 *   and size. Left: retail recomputes the hero base each round and builds the slot base with mtlo/madd, ours
 *   hoists it (mult+addu). madd appears in matched C only for "global_array + i * 0x50" passed on
 *   (func_L00_002BC860); byte and typed forms here stay mult.
 */
extern int D_L00_0015F4F8_10E00 __asm__("D_L00_0015F4F8") MACRO_ADDR;
extern void func_L00_00210558(int);
extern void func_L00_00205780(void *);
extern void func_00213DE0(void *, int, int, int);
extern void func_L00_00210340(int, int);

/* Per-frame update of the hero's seven item slots. */
void func_L00_00210E00(void) {
    int i;
    int k;
    char *obj;
    struct HeroItemSlot *slot;

    for (i = 0; i < 7; i++) {
        slot = &D_0013F450_0CDF0.items[i];
        obj = (char *)slot->moby;
        if (obj != 0) {
            switch (slot->state) {
            case 2:
                slot->unk20++;
                func_L00_00210558(i);
                if (i == 0) {
                    for (k = 0; k < 3; k++) {
                        if (D_0013F450_0CDF0.unk18F0[k].unkA0 != -1) {
                            D_0013F450_0CDF0.unk18F0[k].kind = 5;
                            func_L00_00205780(&D_0013F450_0CDF0.unk18F0[k]);
                        }
                    }
                }
                if ((*(unsigned char *)(obj + 0x70) & 2) && *(unsigned char *)(obj + 0x53) == 0) {
                    func_00213DE0(obj, 1, 0, 2);
                }
                break;
            case 3:
                slot->unk1B++;
                if ((*(unsigned char *)(obj + 0x70) & 2) || i == 1 || i == 0) {
                    func_L00_00210340(i, D_L00_0015F4F8_10E00 + 2);
                }
                break;
            }
        } else {
            func_L00_00210558(i);
        }
        if (obj != 0 && *(unsigned char *)(obj + 0x20) != 0xFE && *(unsigned char *)(obj + 0x20) != 0xFD) {
            if (i != 6 && *(int *)(obj + 0x74) != 0) {
                (*(void (**)(char *))(obj + 0x74))(obj);
            }
        }
    }
}
