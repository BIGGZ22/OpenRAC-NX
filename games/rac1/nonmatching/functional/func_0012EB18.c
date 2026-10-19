/* functional: high. fcheck: unchecked (the harness cannot compare it: retail's own assembly in its place gets the same DIFFERS; read against the assembly instruction by instruction instead; OpenRAC tools/fcheck_gate.py, 2026-10-18). The NTSC decomp's matched C for the same instructions (snd_send_current_batch), carried to PAL's
   names by OpenRAC's tools/port.py. Not a PAL match: 8 of 276 bytes differ under PAL's compiler.
   Call-site fix: func_0012E038 reads only its first two arguments (asm: $4, $5), so the third ($6) is dropped from the declaration and the call. */
extern short D_0015EDC0;
extern short D_0015EDA0;
extern short D_0015EDA8;
extern short D_0015EDB8;
extern u8 D_0015ECC0_2EB18[] __asm__("D_0015ECC0");
extern char D_00153E20[];
extern void func_0012E038_2EB18(void *, s32) __asm__("func_0012E038");
extern s32 func_0011B6B8(void *);
extern s32 func_00116078_2EB18(const char *, ...) __asm__("func_00116078");
extern void func_00118D80(s32);
extern s32 func_0011B4C8(void *, u32, u32, void *, s32, void *, s32, void (*)(void *), void *);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/audio/rpc/snd_send_current_batch.c, snd_send_current_batch. */
void func_0012EB18(void) {
    s32 next;

    func_0012E038_2EB18((((void * *)&D_0015EDB8))[(*(s32 *)&D_0015EDC0)], *(((s32 * *)&D_0015EDA0))[(*(s32 *)&D_0015EDC0)]);
    while (func_0011B6B8(D_0015ECC0_2EB18) != 0) {
        func_00116078_2EB18(D_00153E20);
        func_00118D80(0);
    }
    func_0011B4C8(D_0015ECC0_2EB18, 0x4D, 1, (((s32 * *)&D_0015EDA0))[(*(s32 *)&D_0015EDC0)], 0x1000 - (((s32 *)&D_0015EDA8))[(*(s32 *)&D_0015EDC0)],
                  (((void * *)&D_0015EDB8))[(*(s32 *)&D_0015EDC0)], (*(((s32 * *)&D_0015EDA0))[(*(s32 *)&D_0015EDC0)] << 2) + 8, 0, 0);
    next = (*(s32 *)&D_0015EDC0) != 1;
    (*(s32 *)&D_0015EDC0) = next;
    *(((s32 * *)&D_0015EDA0))[next] = 0;
    (((s32 *)&D_0015EDA8))[next] = 0xFFC;
}
