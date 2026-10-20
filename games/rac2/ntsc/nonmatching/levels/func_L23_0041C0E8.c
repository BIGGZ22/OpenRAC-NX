/* 0 of 1 places match - last line: 23_damosel_orbit: 1148 bytes, expected 1168
   Belongs in src/levels/23_damosel_orbit/L23_00416388-g8.c (-G8: twelve $gp words).
   Control flow, frame (0x90, $s0-$s4/$f20), the shared `lui $s4,%hi(D_00189E20)` with a fresh
   `addiu %lo` per statement (one block-scoped hero pointer per statement: a single pointer
   variable moves it to $a1), the three float compares (`level > limit`), the bare-symbol
   D_L23_001B371C (plain extern: `lui $at`/`sw` in ordinary flow, $gp in delay slots) and the
   whole second half are retail's.
   The 20 missing bytes are one wall: this cc1's GCSE (alias-aware load_killed_in_block_p)
   treats the stores to D_L23_001B1694/16A4/16A8/371C as not killing the load of
   D_L23_001B1698, so PRE hoists that load before the `D_1694 > 0` test and keeps it in one
   register to the final `D_1694 == D_1698`; retail reloads D_1698 after every join (five
   loads), which is what the older "any store kills every load" GCSE model gives.  The knock-on
   is D_1694 in $v0 instead of $v1 at the join and a different filler in two delay slots.
   Measured: making only the D_1698 reads volatile (diagnostic, not a candidate) brings every
   other block to retail's bytes, so the C below is otherwise right; no non-volatile spelling
   can create a kill there (two distinct small symbols never alias in alias.c).  Swapping the
   first two stores of the inner then-part scores 250/292 aligned instead of 242, but only
   through that wrong register; the diagnostic confirms this order.
   Tried: 8 spellings of the compares, statement orders of the then-part, a single hero
   pointer, struct vs cast field accesses, probes removing each D_1698 load. */

typedef int TI128 __attribute__((mode(TI)));

/* The fields of the level state, the object and the hero this function reads. */
typedef struct {
    char pad0[0x408];
    int handle;
    char pad40C[0x588 - 0x40C];
    float limit;
    char pad58C[0x5B8 - 0x58C];
    float dist;
    float range;
    float level;
} C0E8State;

typedef struct {
    char pad0[0x10];
    TI128 pos;
    unsigned char state;
} C0E8Moby;

typedef struct {
    char pad0[0x80];
    float pos[4];
    char pad90[0x1DC - 0x90];
    short busy;
    char pad1DE[0x33C - 0x1DE];
    int armed;
    int charge;
    char pad344[0x34E - 0x344];
    short heat;
} C0E8Hero;

extern char *D_L23_001B1484 __attribute__((sda));

extern int D_L23_001B141C __attribute__((sda));

extern float D_L23_001B168C __attribute__((sda));

extern float D_L23_001B1690 __attribute__((sda));

extern int D_L23_001B1694 __attribute__((sda));

extern int D_L23_001B1698 __attribute__((sda));

extern int D_L23_001B169C __attribute__((sda));

extern int D_L23_001B16A4 __attribute__((sda));

extern int D_L23_001B16A8 __attribute__((sda));

extern int D_L23_001B16AC __attribute__((sda));

extern float D_L23_001B16B0 __attribute__((sda));

extern float D_L23_001B36D0[4] __attribute__((sda));

extern int D_L23_001B371C;

extern C0E8Hero D_00189E20;

extern int func_002E5CF8(int index, int a1, char *a2);

extern int func_00282948(int *);

extern float func_00282C48(float);

extern void func_00282CF0(float *, float *, float *);

extern void func_00282F20(float *, float *, float);

extern void func_L23_0041E5B0(void *, int, int, TI128 *, float *, int);

extern int func_0032C700(void *, int, int, int, int, int, int, int, float, float, int, int, int, int);

extern int func_L23_00426248(void *, void *, int, int, int, int, int, int, float, int, int, int, int, int);

void func_L23_0041C0E8(C0E8Moby *arg0) {
    C0E8State *state = *(C0E8State **)(D_L23_001B1484 + 0x68);
    float v[4] __attribute__((aligned(16)));
    TI128 t;

    func_00282948(&D_L23_001B371C);
    if (arg0->state == 0x10 && state->dist < D_L23_001B168C && state->level > state->limit &&
        func_00282C48(state->level - state->limit) < D_L23_001B1690) {
        C0E8Hero *hero = &D_00189E20;

        if (hero->charge >= 11 && D_L23_001B1694 == 0) {
            D_L23_001B16A4 = 1;
            D_L23_001B1694 = D_L23_001B169C;
            if (D_L23_001B16A8 != 0) {
                D_L23_001B16A8 = 0;
                D_L23_001B1694 = D_L23_001B1698;
            }
        }
    }
    if (D_L23_001B1694 > 0) {
        if (D_L23_001B1694 == D_L23_001B1698) {
            if (D_L23_001B16A4 != 0)
                D_L23_001B371C = 0;
            else
                D_L23_001B1694 += 2;
        } else {
            C0E8Hero *hero = &D_00189E20;

            if (hero->heat >= 11)
                D_L23_001B16A8 = 1;
        }
    }
    if (D_L23_001B16A8 != 0 && D_L23_001B1694 > D_L23_001B1698) {
        C0E8Hero *hero = &D_00189E20;

        if (hero->charge >= 11)
            D_L23_001B1694 = D_L23_001B1698;
    }
    {
        C0E8Hero *hero = &D_00189E20;

        if (hero->charge != 0 && hero->armed == 0)
            D_L23_001B16A8 = 0;
    }
    {
        C0E8Hero *hero = &D_00189E20;

        if (hero->charge >= 11 && D_L23_001B16A4 == 0 && D_L23_001B1694 > 0 && D_L23_001B1694 <= D_L23_001B1698)
            D_L23_001B1694 = D_L23_001B1698 + 5;
    }
    if (D_L23_001B1694 == D_L23_001B1698) {
        func_002E5CF8(10, 0, (char *)arg0);
        func_002E5CF8(0x11, 0, (char *)arg0);
    }
    if (D_L23_001B1694 < 0) {
        D_L23_001B1694++;
        if (D_L23_001B371C == 0) {
            C0E8Hero *hero = &D_00189E20;

            if (hero->charge != 0 && state->range < 10.0f && state->level > state->limit) {
                func_00282CF0(v, hero->pos, (float *)&arg0->pos);
                func_00282F20(v, v, 3.0f);
                t = arg0->pos;
                v[3] = 5627.9248046875f;
                func_L23_0041E5B0((void *)state->handle, 0x10007, 0x46, &t, v, 0);
                D_L23_001B371C = D_L23_001B141C;
                func_0032C700(D_L23_001B36D0, 1, 5, 0x46, 3, 15, 20, 1, 3.0f, 20.0f, 0, 1, 0, 0);
            }
        }
    } else if (D_L23_001B1694 > 0) {
        func_00282948(&D_L23_001B1694);
        if (D_L23_001B1694 == 0) {
            state->handle = func_L23_00426248(&arg0->pos, arg0, 1, 0x19, 0x14, 0x14, 0x14, 0x2D, 1.0f, 1, 1, 0, 0, 0x40);
            D_L23_001B1694 = -D_L23_001B16AC;
            func_002E5CF8(7, 0, (char *)arg0);
            func_002E5CF8(12, 0, (char *)arg0);
            if (state->range < 15.0f && state->level > state->limit) {
                C0E8Hero *hero = &D_00189E20;

                if (hero->busy == 0) {
                    func_00282CF0(v, hero->pos, (float *)&arg0->pos);
                    func_00282F20(v, v, D_L23_001B16B0);
                    t = arg0->pos;
                    v[3] = 5627.9248046875f;
                    func_L23_0041E5B0(arg0, 0x10007, 0x46, &t, v, 0);
                    func_0032C700(D_L23_001B36D0, 1, 5, 0x46, 3, 15, 20, 1, 3.0f, 20.0f, 0, 1, 0, 0);
                }
            }
        }
    }
}
