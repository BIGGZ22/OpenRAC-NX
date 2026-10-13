/* C candidate for src/game/002E10B8.c; one instruction differs at +0x1c. */
s32 func_002E5F40(s32 index, s32 callback) {
    u8 *p = D_00188660 + 0x84;
    *(s32 *)(p + index * 0x70) = callback;
    return 1;
}
