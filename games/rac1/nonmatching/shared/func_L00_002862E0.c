/* NON_MATCHING func_L00_002862E0 -- src/overlays/shared/pause_00277208.c
 * Best so far: BYTES 46/436 (89.5% of the bytes match), checked 2026-10-15.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   2026-10-15: rewritten without goto. The save block at D_L00_001BB5C0 is a struct (PauseSave), read
 *   directly so retail's kept %hi comes out, the hero through Hero (motion.pos, motion.rot, moby re-read per
 *   store), the voice block as D_0013E650 (a separate symbol, so gcc no longer derives it from the hero's
 *   address), the kind as a switch with default, and the moby search as an index loop. Size 436 = retail,
 *   BYTES 46/436. Left: in the voice block retail loads the bytes 0x40-0x42 into t0/a1/a2 and the sound
 *   halfword last. Ours loads the halfword first and uses other registers, and the loop keeps e and i in other
 *   registers. A local voice pointer and the loop with the assignment in its condition change nothing.
 */
#include "common.h"
#include "hero.h"
extern Hero D_0013F450_2862E0 __asm__("D_0013F450");
typedef struct {
    int valid;
    char pad4[0xC];
    char pos[0x10];
    char rot[0x10];
    int moby38;
    int moby3C;
    int moby80;
    int voice64;
    unsigned char voice68[3];
    char pad43;
    int kind;
    int arg48;
    int arg4C;
    char pad50[0xC54 - 0x50];
    unsigned short sound;
} PauseSave;
extern PauseSave D_L00_001BB5C0_2862E0 __asm__("D_L00_001BB5C0");
extern char D_L00_001BA960[] NOT_SDA;
extern unsigned char *D_L00_00160098 MACRO_ADDR;
extern unsigned char *D_L00_0016009C MACRO_ADDR;
extern void func_001F99B0(void *, int, int);
extern void func_L00_001FF040(int, void *, unsigned short);
extern void func_001F9978(void);
extern void func_L00_002110C0(int, int);
extern void func_L00_00251E30(void *);
extern void func_001FA1F8(void *, void *);
extern void func_L00_001ED600(void);
extern void func_00216270(void);

/* loads the saved pause-menu state block into the game globals */
void func_L00_002862E0(void) {
    unsigned char *e;
    int k, s, i;
    if (D_L00_001BB5C0_2862E0.valid == 0) {
        func_001F99B0(D_L00_001BA960, 0, 0xC60);
        return;
    }
    func_L00_001FF040((int)D_L00_001BA960, &D_L00_001BB5C0_2862E0, 0xC60);
    qcopy(&D_0013F450_2862E0.motion.pos, D_L00_001BB5C0_2862E0.pos);
    qcopy(&D_0013F450_2862E0.motion.rot, D_L00_001BB5C0_2862E0.rot);
    *(int *)((char *)D_0013F450_2862E0.moby + 0x38) = D_L00_001BB5C0_2862E0.moby38;
    *(int *)((char *)D_0013F450_2862E0.moby + 0x3C) = D_L00_001BB5C0_2862E0.moby3C;
    *(int *)((char *)D_0013F450_2862E0.moby + 0x80) = D_L00_001BB5C0_2862E0.moby80;
    D_0013E650[0x6B] |= 7;
    *(unsigned short *)(&D_0014171B + 0x100ED) = D_L00_001BB5C0_2862E0.sound;
    *(int *)(D_0013E650 + 0x64) = D_L00_001BB5C0_2862E0.voice64;
    D_0013E650[0x68] = D_L00_001BB5C0_2862E0.voice68[0];
    D_0013E650[0x69] = D_L00_001BB5C0_2862E0.voice68[1];
    D_0013E650[0x6A] = D_L00_001BB5C0_2862E0.voice68[2];
    k = D_L00_001BB5C0_2862E0.kind;
    if (k != 0 && k != 3) {
        s = 0;
        switch (k) {
        case 1:
            s = 0x57;
            break;
        case 2:
            s = 0x1A3;
            break;
        default:
            func_001F9978();
            break;
        }
        for (i = 0; ; i++) {
            e = D_L00_00160098 + i * 0x100;
            if (e >= D_L00_0016009C) break;
            if (*(short *)(e + 0xA6) == s) {
                func_L00_002110C0(D_L00_001BB5C0_2862E0.kind, D_L00_001BB5C0_2862E0.arg4C);
                break;
            }
        }
    }
    func_L00_00251E30(D_0013F450_2862E0.moby);
    func_001FA1F8((char *)D_0013F450_2862E0.moby + 0xC0, &D_0013F450_2862E0.motion.rot);
    func_L00_001ED600();
    func_00216270();
}
