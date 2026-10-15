/* NON_MATCHING func_L00_001F92E8 -- src/overlays/shared/draw_001F3A78.c
 * Best so far: SIZE ours 252 / retail 248, checked 2026-10-12.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   func_L00_001F92E8: debug-draw tail. cand.c, SIZE 252/248 (a NOT_SDA variant reached 244). Left: D_L00_0015F708
 */
typedef struct {
    int key;
    int pad[3];
} DbgRec_1F92E8;
extern int D_L00_0015F708_i NOT_SDA;
extern float D_L00_0015F4FC_r __asm__("D_L00_0015F4FC") MACRO_ADDR;
extern DbgRec_1F92E8 D_L00_00173180[];
extern short D_L00_0015F706_i NOT_SDA;
extern void func_001F7680(int);
extern int func_001FA898_q(float) __asm__("func_001FA898");
extern void func_001F55C0(int, int, int, int);
extern void func_001F3C10(void);
extern void func_001F4630(int);
extern void func_L00_001FCC50(void *, int);
extern void func_001F4748(void);

void func_L00_001F92E8(void) {
    DbgRec_1F92E8 *p;
    DbgRec_1F92E8 *q;

    if (D_L00_0015F6BC == 0) {
        func_001F7680(D_L00_0015F708_i);
        if (0.0f < D_L00_0015F4FC_r) {
            if (1.0f < D_L00_0015F4FC_r) {
                D_L00_0015F4FC_r = 1.0f;
            }
            func_001F55C0(0, 0, 0, func_001FA898_q(D_L00_0015F4FC_r * 128.0f));
        }
        func_001F3C10();
        func_001F4630(0);
        p = D_L00_00173180;
        if (p->key >= 0) {
            q = p;
            do {
                func_L00_001FCC50(q, D_L00_0015F706_i);
                p++;
                q++;
            } while (p->key >= 0);
        }
        func_001F4748();
    }
}
