/* func_L05_0030DA08 -- src/overlays/shared/vendor_002CF2C0.c (functional C for the port, not a match)
 * Floor button update (levels 5, 11, 15, 17 and 18). State 0: sinks into place (z - 0.35) and starts
 * its pulse phase; if the button is already done (the level's pressed byte at +0x454 by its index
 * +0xB2, or its save bit, or its linked flag +0xB0 is still unset) it goes straight to the pressed
 * look (state 2: colour 0x80208020, +0xBC = 2) and clears its entries in its target list (pvars +0,
 * D_L05_001B0CB0, matched by pvars +4); else it waits in state 1. State 1: pulses its colour; when
 * Ratchet stands on it (hero +0x2FC, +0x30E clear) it records the press, plays its sound, goes to
 * state 2 and clears its list entries as above.
 * From the staged near miss (nonmatching/shared/func_L05_0030DA08.c); fixed: the level number is
 * D_0015EE84 (the near miss read D_L05_0015FE84).
 * equiv: DIFFERENT only by code shape, not behaviour: the colour clamp comes out as movn into a
 * copy instead of retail's movz in place, retail re-zero-extends the lbu'd +0xB0 (andi 0xFF),
 * one equality test has the other branch polarity, and the state-1 store's constant is set
 * before a label in retail. Every load, store, call and constant otherwise matches. */
extern char *D_L05_001B0CB0_30DA08[] __asm__("D_L05_001B0CB0");
typedef struct {
    unsigned char pad0[0x454];
    unsigned char done[1];
} Lvl_30DA08;
extern Lvl_30DA08 D_L05_001BBA40_30DA08 __asm__("D_L05_001BBA40");
extern Lvl_30DA08 D_L05_001BADE0_30DA08 __asm__("D_L05_001BADE0");
/* D_0014171B + 0xAB75 and + 0xAA35: the save's per-level bit words and per-level flag bytes */
extern int D_0014C290_30DA08[] __asm__("D_0014C290");
extern unsigned char D_0014C150_30DA08[] __asm__("D_0014C150");
extern unsigned char D_L05_0015FD48_30DA08[] __asm__("D_L05_0015FD48");
extern float D_0015EE6C_30DA08 __asm__("D_0015EE6C") MACRO_ADDR;
extern int D_0015EE84_30DA08 SDATA(D_0015EE84);
typedef struct {
    unsigned char pad0[0x2FC];
    void *standing;
    unsigned char pad300[0x30E - 0x300];
    short h30E;
} Hero_30DA08;
extern Hero_30DA08 G_30DA08 __asm__("D_0013F450");

extern float func_00214158_30DA08(void) __asm__("func_00214158");
extern float func_001FA748_30DA08(float, float) __asm__("func_001FA748");
extern float func_001F9FA8_30DA08(float) __asm__("func_001F9FA8");
extern int func_001FA898_30DA08(float) __asm__("func_001FA898");
extern void func_0022ED80_30DA08(int, int, void *) __asm__("func_0022ED80");

void func_L05_0030DA08(unsigned char *m) {
    char *d = *(char **)(m + 0x78);
    unsigned short u;
    int i, v, lvl;
    unsigned char b0;
    float f;

    switch (m[0x20]) {
    case 0:
        *(float *)(d + 0xC) = func_00214158_30DA08();
        *(float *)(m + 0x18) = *(float *)(m + 0x18) - 0.35f;
        u = *(unsigned short *)(m + 0xB2);
        lvl = D_0015EE84_30DA08;
        if (D_L05_001BBA40_30DA08.done[(short)u] == 0 &&
            ((*(int *)((char *)D_0014C290_30DA08 + (((short)u >> 5) << 2) + (lvl << 8)) >> (u & 0x1F)) & 1) == 0 &&
            (*(int *)(d + 8) == 0 || *(unsigned char *)((m[0xB0] + (lvl << 4)) + D_0014C150_30DA08) != 0xFF)) {
            m[0x20] = 1;
            return;
        }
        m[0x20] = 2;
        *(int *)(m + 0x90) = 0x80208020;
        m[0xBC] = 2;
        if (*(int *)d == -1) {
            return;
        }
        for (i = 0; i < *(int *)D_L05_001B0CB0_30DA08[*(int *)d]; i++) {
            char *e = D_L05_001B0CB0_30DA08[*(int *)d] + (i << 4);
            if (*(float *)(e + 0x1C) == *(float *)(d + 4)) {
                *(int *)(e + 0x1C) = 0;
            }
        }
        break;
    case 1:
        f = func_001FA748_30DA08(*(float *)(d + 0xC), D_0015EE6C_30DA08 * 6.2831855f);
        *(float *)(d + 0xC) = f;
        f = func_001F9FA8_30DA08(f);
        v = func_001FA898_30DA08((f * 4.0f - 3.0f) * 128.0f);
        if (v < 0x81) {
            v = (v > 0x1F) ? v : 0x20;
        } else {
            v = 0x80;
        }
        *(int *)(m + 0x90) = (v << 16) | ((v << 8) | 0x80000000) | v;
        if (G_30DA08.standing != m) {
            return;
        }
        if (G_30DA08.h30E != 0) {
            return;
        }
        D_L05_001BADE0_30DA08.done[*(short *)(m + 0xB2)] = m[0xB0] + 2;
        b0 = m[0xB0];
        if (b0 == 0xFF ||
            (D_L05_0015FD48_30DA08[b0] != 0xFF &&
             *(unsigned char *)((b0 + (D_0015EE84_30DA08 << 4)) + D_0014C150_30DA08) == 0xFF)) {
            D_L05_001BBA40_30DA08.done[*(short *)(m + 0xB2)] = b0 + 2;
        }
        m[0x20] = 2;
        *(int *)(m + 0x90) = 0x80208020;
        m[0xBC] = 1;
        func_0022ED80_30DA08(0, 0, m);
        if (*(int *)d == -1) {
            return;
        }
        for (i = 0; i < *(int *)D_L05_001B0CB0_30DA08[*(int *)d]; i++) {
            char *e = D_L05_001B0CB0_30DA08[*(int *)d] + (i << 4);
            if (*(float *)(e + 0x1C) == *(float *)(d + 4)) {
                *(int *)(e + 0x1C) = 0;
            }
        }
        break;
    default:
        return;
    }
}
