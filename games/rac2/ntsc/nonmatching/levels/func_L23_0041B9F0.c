/* func_L23_0041B9F0 (level 23, 1148 bytes) -- NEAR MISS, not integrated.
   state: 1140 bytes against retail's 1148 (285 instructions against 287); tools/check.py: 0 of 1
   places match ("1140 bytes, expected 1148"); 22 of 287 words differ once the bodies are aligned.
   built as the last function of L23_00416388-g8.c (-G8: D_L23_001B1484, the two reach constants
   and D_L23_001B1694 are $gp loads); it uses that unit's own declarations of D_L23_001B1484,
   D_L23_001B1694, D_L23_001B36E0 (a plain small extern: its address comes out as retail's bare
   lui/addiu pair), func_00282CC0, func_00282D30, func_002A7878 and func_0029F338, repeated below.
   matches: the 0xE0 frame (the vector at sp+0, the particle velocity at sp+0x10, the joint matrix
   at sp+0x20), the prologue, both arms of the shield test, the hit filter (its two halfword loads
   and the f604 load hoisted by PRE come out as retail's lh/lhu/lw), the four-particle loop
   (retail's invariants, the bnel store of the callback, func_00273730 takes its two floats before
   the last int), the damage and death sequence, the generator loop with D_L23_001B13CC through
   $at (sda, sized past the $gp limit), and the held-object tail. Measured: the moby and the
   state as unsigned char (a char 0xFF store is CSE'd into the register that holds the -1 of the
   0x60C test), block-scoped temporaries in the generator loop (one variable for both float reads
   lands the threshold in $f0), the damage subtraction before the timer store, and 0.017453294f
   (0x3C8EFA36, one ulp above pi/180).
   what still differs:
   - the three 16-byte copies to D_L23_001B36E0: retail emits `lui $v1; addiu $v1; lq $v0,0($sp);
     sq $v0,0($v1)` back to back; this cc1 schedules the lq first (latency 2) and sinks the sq
     past the next call's arguments (in the generator arm past the loop set-up, so the loop
     pointer and counter take $a0/$a1 instead of retail's $v1/$a0). The address form itself is
     right; the order is the known wall of these copies (they behave as one unit in retail).
   - the PRE placement of st+0x140: retail computes it in the first block (after func_002A9640)
     and fills the `beqz $s3` slot with the tail's `daddu $a1,$s4,$zero`; this cc1 splits the
     critical edge into the tail (the tail is reached only by jumps) and inserts the computation
     there, so reorg moves it into that slot. Retail is one instruction longer here, and the
     particle loop then needs retail's alignment nop: that is the 8 bytes. Hypothesis: retail's
     GCSE does not split critical edges (gcse.c's compute_preds_succs(..., 1) does here); no
     source shape found that gives the tail a fall-through predecessor with retail's layout.
   - knock-on: the copy of moby+0x10 for the loop lands right after the first call instead of in
     the func_002A9640 delay slot.
   tried: st+0x140 in a variable (spilled, 0xF0 frame), the two hit tests as sibling ifs (same
   code), a variable for the loop's position argument (77 words off), five orders/spellings of
   the damage subtraction, func_00273730 with its floats last (the $f12 move then takes the
   call's delay slot instead of retail's `addiu $t3,$zero,0xFF`). */

typedef int TI16 __attribute__((mode(TI)));

/* from the unit */
extern char *D_L23_001B1484 __attribute__((sda));

extern int D_L23_001B1694 __attribute__((sda));

extern int D_L23_001B36E0;

extern void func_00282CC0(char *, char *, char *);

extern void func_00282D30(char *, char *, float);

extern float func_002A7878(float, float);

extern void func_0029F338(void *);


/* the shield's two reach constants */
extern float D_L23_001B16BC __attribute__((sda));

extern float D_L23_001B16C0 __attribute__((sda));

/* read as a bare symbol through $at: sized past the $gp limit */
extern float D_L23_001B13CC[3] __attribute__((sda));

extern int D_0018C0B0 __attribute__((nosda));

/* the hit-delay timer */
extern int D_L23_001B3714;

/* the camera block: its angle at +0x158 */
typedef struct {
    char pad[0x158];
    float f158;
} Cam_0041B9F0;

extern Cam_0041B9F0 D_L23_001BA500;

/* the player block */
typedef struct {
    char pad[0x1870];
    char f1870[0xC];
    int f187C;
    char pad2[0x2290 - 0x1880];
    int f2290;
    int f2294;
} Player_0041B9F0;

extern Player_0041B9F0 D_00189E20;

extern void func_L23_0041B968(void);

extern int func_00282948(void *);

extern int func_00282978(void *);

extern char *func_002A9640(void *, int, int);

extern void func_L23_0041E570(void);

extern void func_002AB800(void *, void *);

extern float func_002A78D8(float, float);

extern float func_00283B98(float, float);

extern float func_00283180(float);

extern float func_00283198(float);

extern char *func_00273730(void *, int, int, int, void *, void *, void *, float, float, int);

extern void func_00310C88();

extern void func_L23_0041C578(char *);

extern void func_002A7750(void *, int, int);

extern void func_L00_002AF7D0(void);

extern void func_L00_00310C88(int, void *);

extern void func_L22_002C9898(int, int);

extern void func_002AB8C0(void *, void *);

extern void func_002B3380(void *, void *, void *);

extern void func_0029FC20(void *, int, void *);

/* the boss shield: while it is down, takes the hits of the shots that reach it; while it is up,
   sums the strength of the live generators into the shield measure */
void func_L23_0041B9F0(unsigned char *moby) {
    char tmp[16];
    float vec[4];
    float mtx[16];
    unsigned char *st = *(unsigned char **)(D_L23_001B1484 + 0x68);
    char *hit;
    float total;
    int i;

    func_L23_0041B968();
    func_00282948(&D_L23_001B3714);
    if (*(int *)(st + 0x60C) == -1 && *(char **)(st + 0x610) == 0) {
        func_00282D30(tmp, moby + 0xE0, D_L23_001B16BC);
        func_00282CC0(tmp, tmp, moby + 0x10);
        *(TI16 *)&D_L23_001B36E0 = *(TI16 *)tmp;
        hit = func_002A9640(moby, 0x30000, 0);
        moby[0xA8] = 0xFF;
        func_00282978(st + 0x26);
        if (hit != 0) {
            if (*(short *)(st + 0x26) != 0 || *(int *)(st + 0x604) != 0
                || (*(int *)(hit + 0x20) == D_0018C0B0 && D_L23_001B1694 < 0)) {
                hit = 0;
                if (*(int *)(st + 0x604) != 0 && *(unsigned short *)(st + 0x26) == 0)
                    func_L23_0041E570();
            }
            if (hit != 0) {
                st[0x147] = 0xFF;
                func_002AB800(moby, st + 0x140);
                if (*(unsigned short *)(moby + 0x7C) != 0) {
                    for (i = 0; i < 4; i++) {
                        float angle;
                        float speed;
                        char *child;

                        angle = func_00283B98(func_002A78D8(90.0f, 120.0f) * 0.017453294f,
                                              D_L23_001BA500.f158);
                        speed = func_002A7878(2.0f, 4.0f) * 0.016666668f;
                        vec[0] = func_00283180(angle) * speed;
                        vec[1] = func_00283198(angle) * speed;
                        vec[2] = 0.0f;
                        vec[2] = func_002A7878(5.0f, 8.0f) * 0.016666668f;
                        child = func_00273730(moby, 1, *(short *)(moby + 0xAA), i, moby + 0x10,
                                              moby + 0xF0, vec, 90.0f, 15.0f, 0xFF);
                        if (child != 0)
                            *(void **)(child + 0x64) = (void *)func_00310C88;
                    }
                }
                *(unsigned short *)(moby + 0x7C) = 0;
                func_L23_0041C578(*(char **)(hit + 0x20));
                *(float *)(st + 0x20) -= *(float *)(hit + 0x2C);
                *(short *)(st + 0x26) = 20;
                if (*(float *)(st + 0x20) <= 0.0f) {
                    func_002A7750(moby, 4, 0);
                    D_00189E20.f2294 = 0;
                    func_L00_002AF7D0();
                    func_L00_00310C88(D_00189E20.f2290, D_00189E20.f1870);
                    D_00189E20.f187C = 0;
                    func_L22_002C9898(0, 1);
                    return;
                }
            }
        }
        func_002AB8C0(moby, st + 0x140);
        func_002B3380(moby, st + 0x210, st + 0x20);
    } else {
        char *held;

        total = *(float *)(st + 0x32C) * (float)*(short *)(st + 0x2F6);
        if (*(int *)(st + 0x60C) != -1) {
            int k;

            func_0029FC20(moby, *(int *)(st + 0x60C) + 1, mtx);
            func_00282D30(tmp, (char *)(mtx + 4), D_L23_001B16C0);
            func_00282CC0(tmp, tmp, moby + 0x10);
            *(TI16 *)&D_L23_001B36E0 = *(TI16 *)tmp;
            for (k = 0; k < 5; k++) {
                char *gen = ((char **)(st + 0x358))[k];

                if (gen != 0) {
                    float v = *(float *)(*(char **)(gen + 0x68) + 0x20);

                    if (D_L23_001B13CC[0] < v)
                        total += v;
                }
            }
        } else {
            func_00282D30(tmp, moby + 0xE0, D_L23_001B16BC);
            func_00282CC0(tmp, tmp, moby + 0x10);
            *(TI16 *)&D_L23_001B36E0 = *(TI16 *)tmp;
        }
        held = *(char **)(st + 0x610);
        if (held != 0) {
            float h = *(float *)(*(char **)(held + 0x68) + 0x20);

            if (h >= 0.0f) {
                total += h;
            } else {
                func_0029F338(held);
                *(char **)(st + 0x610) = 0;
                *(short *)(st + 0x26) = 30;
            }
        }
        if (*(unsigned short *)(moby + 0x7C) != 0)
            total += *(float *)(st + 0x324) * (float)*(short *)(st + 0x2F6);
        *(float *)(st + 0x20) = total;
    }
}
