/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), src/overlays/shared/pause_00277208.c (games/rac1): func_L00_00280708, matched C there.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */

struct MenuPage_280708_c00_27f700 {
    unsigned char pad0[0x38];
    struct MenuPage_280708_c00_27f700 *back;
    unsigned char pad3C[0x84 - 0x3C];
    int unk84;
};
struct MenuSystem_280708_c00_27f700 {
    unsigned char pad0[4];
    struct MenuPage_280708_c00_27f700 *current;
    struct MenuPage_280708_c00_27f700 *next;
    unsigned char pad0C[0xD4 - 0xC];
    int unkD4;
};
struct Pad_280708_c00_27f700 { unsigned char pad[0x1A4]; unsigned int pressed; };
extern struct MenuSystem_280708_c00_27f700 D_L00_001BA070_280708_c00_27f700 __asm__("D_L00_001B9CF0");
extern struct Pad_280708_c00_27f700 D_0013CA40_280708_c00_27f700 __asm__("D_0013C940");
extern int D_0015EFB0_280708_c00_27f700 __asm__("D_0015EEB0") ;
int FUN_L00_0027f700(void) {
    int state = D_0015EFB0_280708_c00_27f700;
    unsigned int pressed;
    if (state != 0x10 && state != 1) {
        D_L00_001BA070_280708_c00_27f700.next = D_L00_001BA070_280708_c00_27f700.current->back;
        return 0;
    }
    pressed = D_0013CA40_280708_c00_27f700.pressed;
    if (pressed & 0x20) {
        D_L00_001BA070_280708_c00_27f700.unkD4 = 0;
        D_L00_001BA070_280708_c00_27f700.next = D_L00_001BA070_280708_c00_27f700.current->back;
        D_L00_001BA070_280708_c00_27f700.current->unk84 = 1;
    } else if (pressed & 0x10) {
        D_L00_001BA070_280708_c00_27f700.next = D_L00_001BA070_280708_c00_27f700.current->back;
        D_L00_001BA070_280708_c00_27f700.current->unk84 = 0;
    }
    return 0;
}
