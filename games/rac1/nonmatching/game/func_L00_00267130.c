/* Stream handle callback for the player's other state: a negative handle is stored at +0 (the delay-slot store of a branch-likely); a zero handle sets state 0; state 9 becomes 4 when the word at +0x10 is set, and then a global at 0x001521F0 is set to 1. Exits are taken as plain returns. */
extern short D_0014171B_b[] __asm__("D_0014171B");

void func_L00_00267130(s32 handle, char *p) {
    if (p == 0) {
        return;
    }
    if (handle < 0) {
        *(s32 *)p = handle;
    }
    if (handle == 0) {
        *(s16 *)(p + 0xA) = 0;
        return;
    }
    if (*(s16 *)(p + 0xA) != 9) {
        return;
    }
    *(s16 *)(p + 0xA) = 4;
    if (*(s16 *)(p + 0x10) == 0) {
        return;
    }
    *(s16 *)((char *)D_0014171B_b + 0x100D5) = 1;
}
