/* Carried from port/game/rac1-pal/hand/level_checkpoint_restore.c through the version map (../README.md): names and
 * addresses are this version's, raw addresses in comments are PAL's. Not yet checked in play.
 * Checked against the US assembly (2026-10-18): same behaviour. */
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Native recovery of PAL 002862E0 (0x1B4), checked against retail.
 * Restore the saved checkpoint block and reconnect the hero state.
 * Explicit word offsets avoid the candidate's conflicting byte-array
 * declarations; pointer arguments remain pointers on the native host.
 * This is not a compiler-matching PS2 implementation. */
extern char D_L00_001BA5D0[] NOT_SDA;
extern unsigned char D_L00_001BB230[];
extern char D_0013F350[], D_0013E550[], D_001516D0[];
extern char *D_L00_0015FFD8 MACRO_ADDR;
extern char *D_L00_0015FFDC MACRO_ADDR;
extern void func_001F97E8(void *, int, int);
extern void checkpoint_copy(void *, void *, unsigned short)
    __asm__("func_L00_001FF090");
extern void func_001F97B0(void);
extern void func_L00_00210A08(int, int, void *);
extern void func_L00_00250DF8(void *);
extern void func_001FA030(void *, void *);
extern void func_L00_001ED280(void);
extern void func_00215420(void);

void FUN_L00_00285008(void) {
    unsigned char *snapshot = D_L00_001BB230;
    char *hero = D_0013F350;
    char *status = D_0013E550;
    int i;
    if (*(int *)snapshot == 0) {
        func_001F97E8(D_L00_001BA5D0, 0, 0xC60);
        return;
    }
    checkpoint_copy(D_L00_001BA5D0, snapshot, 0xC60);
    for (i = 0; i < 4; ++i)
        ((unsigned *)(hero + 0x80))[i] = ((unsigned *)(snapshot + 0x10))[i];
    for (i = 0; i < 4; ++i)
        ((unsigned *)(hero + 0x90))[i] = ((unsigned *)(snapshot + 0x20))[i];
    *(int *)(*(char **)(hero + 0x2080) + 0x38) = *(int *)(snapshot + 0x30);
    *(int *)(*(char **)(hero + 0x2080) + 0x3C) = *(int *)(snapshot + 0x34);
    *(int *)(*(char **)(hero + 0x2080) + 0x80) = *(int *)(snapshot + 0x38);
    status[0x6B] |= 7;
    *(unsigned short *)(D_001516D0 + 0x38) = *(unsigned short *)(snapshot + 0xC54);
    *(int *)(status + 0x64) = *(int *)(snapshot + 0x3C);
    status[0x68] = snapshot[0x40];
    status[0x69] = snapshot[0x41];
    status[0x6A] = snapshot[0x42];
    int mode = *(int *)(snapshot + 0x44);
    if (mode != 0 && mode != 3) {
        int object_class = 0;
        if (mode == 1) object_class = 0x57;
        else if (mode == 2) object_class = 0x1A3;
        else func_001F97B0();
        char *end = D_L00_0015FFDC;
        for (char *entry = D_L00_0015FFD8; entry < end; entry += 0x100) {
            if (*(short *)(entry + 0xA6) == object_class) {
                /* Retail keeps the found object in a2 across the call. */
                func_L00_00210A08(*(int *)(snapshot + 0x44), *(int *)(snapshot + 0x4C), entry);
                break;
            }
        }
    }
    func_L00_00250DF8(*(void **)(hero + 0x2080));
    func_001FA030(*(char **)(hero + 0x2080) + 0xC0, hero + 0x90);
    func_L00_001ED280();
    func_00215420();
}
