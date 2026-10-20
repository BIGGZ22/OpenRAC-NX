/* 0 of 1 places match - last line: 23_damosel_orbit: 1448 bytes, expected 1520
   Built in src/levels/23_damosel_orbit/L23_00416388-g8.c: it has no $gp access, but it calls
   func_L23_00419088 with an int while the -G0 file defines that function with an s16
   parameter (which would add a sign-extension retail does not have).
   Matching: the prologue, the n == 0 and mode-0 paths (`dir *= -1`, `v == 0 ? 1 : -1`, a
   function-scope `k` that retail keeps in $s0 for the tail too), the fill loop's shape, the
   switch tree (case 2 shares default), and the case-1, case-3 and first default loops apart
   from scratch registers (one counter `j` per loop, `ok`/`d`/`f` block-scoped: retail gives
   each loop its own pseudos).
   The wall is the second default loop: retail copies its four 16-byte vectors through
   pointers held in registers (`lq $v0,0($v1)` / `sq $v0,0($s2)`, the pointers hoisted as loop
   invariants and spilled to 0xF0-0x100), the shape of an inline-asm qcopy; this cc1's CSE
   folds the frame offset back into every `lq`/`sq` (`*(TI128 *)a = *(TI128 *)n1`, an inline
   copy helper and TI scalars all measured the same).  With those four addresses no longer
   loop invariants, fewer pseudos spill (frame 0x170 against 0x180, the res base in 0xF0
   against 0x104) and reload picks other scratch registers in the earlier loops, which is
   most of what remains.  The fill loop also allocates k*4 to $s0 and swaps $a0/$a1 against
   retail; declaration orders, statement orders and branch polarity (16 variants) do not move
   it.  `-0.35f` is a local set before the loop guard ($f22), as in retail. */

typedef int TI128 __attribute__((mode(TI)));

typedef unsigned long long u64;

/* The level state fields the target picker reads. */
typedef struct {
    char pad0[0x230];
    int group[0x58];
    char pad390[2];
    short dir;
    unsigned short kind;
    short res;
    char pad398[0x3E0 - 0x398];
    float origin[4];
    float target[4];
    float *ref;
    char pad404[0x410 - 0x404];
    char frame[0x40];
} PickState;

typedef struct {
    char pad0[0x10];
    float pos[4];
    char pad20[0x68 - 0x20];
    PickState *state;
    char pad6C[0xE0 - 0x6C];
    TI128 up;
} PickMoby;

/* 16-byte rows: the key tested for a hit, then the key tested for a miss. */
typedef struct {
    u64 hit;
    u64 miss;
} PickKeys;

extern PickKeys D_L23_002595C8[];

extern float (*D_L23_001FF300[])[4];

extern char D_00189EA0[];

extern int func_L23_00418F30(PickMoby *, u64, int *, int);

extern int func_L23_00418FD0(PickMoby *, int, short);

extern void func_L23_00419088(PickMoby *, int);

extern int func_L23_00419CE0(u64);

extern int func_002A77E0(int);

extern float func_00282E48(float *, float *);

extern float func_00282DB0(float *, float *);

extern void func_00282DD8(float *, float *, float *);

extern void func_L00_00325600(void *, void *, void *);

extern void func_00282CF0(float *, float *, float *);

extern void func_00282F20(float *, float *, float);

extern float func_00282C48(float);

/* Picks one of the targets whose bits are set in the mask: at random, or the one nearest,
   farthest or best aligned, and latches its side and result in the state. */
void func_L23_004190C8(PickMoby *arg0, u64 mask, int mode, int check) {
    PickState *state = arg0->state;
    int idx[4];
    float *pts[4];
    int res[4];
    u64 *keys[4];
    float d1[4] __attribute__((aligned(16)));
    float d2[4] __attribute__((aligned(16)));
    float n1[4] __attribute__((aligned(16)));
    float n2[4] __attribute__((aligned(16)));
    float a[4] __attribute__((aligned(16)));
    float c1[4] __attribute__((aligned(16)));
    float up1[4] __attribute__((aligned(16)));
    float b[4] __attribute__((aligned(16)));
    float c2[4] __attribute__((aligned(16)));
    float up2[4] __attribute__((aligned(16)));
    int n;
    int i;
    int k;
    int best;

    n = func_L23_00418F30(arg0, mask, idx, check);
    if (n == 0) {
        state->dir *= -1;
        return;
    }
    if (mode == 0) {
        int v;

        k = idx[func_002A77E0(n)];
        v = func_L23_00418FD0(arg0, k, state->kind);

        state->dir = v == 0 ? 1 : -1;
        state->res = v;
        func_L23_00419088(arg0, k);
        return;
    }
    for (i = 0; i < n; i++) {
        int k = idx[i];
        int v = func_L23_00418FD0(arg0, k, state->kind);
        float (*e)[4];

        res[i] = v;
        e = D_L23_001FF300[state->group[k]];
        if (v == 0) {
            pts[i] = e[*(int *)e];
            keys[i] = &D_L23_002595C8[state->group[k]].miss;
        } else {
            pts[i] = e[1];
            keys[i] = &D_L23_002595C8[state->group[k]].hit;
        }
    }
    best = 0;
    switch (mode) {
    case 1: {
        float f = 30000.0f;
        int j;

        for (j = 0; j < n; j++) {
            int ok = 0;
            float d;

            if (check != 0)
                ok = func_L23_00419CE0(*keys[j]);
            d = func_00282E48(pts[j], state->ref);
            if ((d < f && ok == 0) || n == 1) {
                best = j;
                f = d;
            }
        }
        break;
    }
    case 3: {
        float f = -30000.0f;
        int j;

        for (j = 0; j < n; j++) {
            int ok = 0;
            float d;

            if (check != 0)
                ok = func_L23_00419CE0(*keys[j]);
            func_00282CF0(d1, (float *)D_00189EA0, arg0->pos);
            func_00282CF0(d2, pts[j], arg0->pos);
            d = func_00282DB0(d1, d2);
            if ((f < d && ok == 0) || n == 1) {
                best = j;
                f = d;
            }
        }
        break;
    }
    case 2:
    default:
        if (state->kind >= 0x22 && state->kind <= 0x25) {
            float f = 30000.0f;
            int j;

            for (j = 0; j < n; j++) {
                int ok = 0;
                float d;

                if (check != 0)
                    ok = func_L23_00419CE0(*keys[j]);
                func_00282CF0(d1, (float *)D_00189EA0, arg0->pos);
                func_00282CF0(d2, pts[j], arg0->pos);
                d = func_00282DB0(d1, d2);
                if ((d < f && ok == 0) || n == 1) {
                    best = j;
                    f = d;
                }
            }
        } else {
            float lim = -0.35f;
            float f = 30000.0f;
            int j;

            for (j = 0; j < n; j++) {
                int ok = 0;
                float d;

                if (check != 0)
                    ok = func_L23_00419CE0(*keys[j]);
                func_00282CF0(d1, state->target, state->origin);
                func_00282CF0(d2, pts[j], state->origin);
                func_00282F20(n1, d1, 1.0f);
                func_00282F20(n2, d2, 1.0f);
                *(TI128 *)a = *(TI128 *)n1;
                *(TI128 *)b = *(TI128 *)n2;
                *(TI128 *)up1 = arg0->up;
                *(TI128 *)up2 = arg0->up;
                func_00282DD8(c1, up1, a);
                func_00282DD8(c2, up2, b);
                func_L00_00325600(a, state->frame, 0);
                func_L00_00325600(b, state->frame, 0);
                d = func_00282C48(func_00282DB0(a, b) - lim);
                if ((d < f && ok == 0) || n == 1) {
                    best = j;
                    f = d;
                }
            }
        }
        break;
    }
    k = idx[best];
    state->dir = res[best] == 0 ? 1 : -1;
    state->res = res[best];
    func_L23_00419088(arg0, k);
}
