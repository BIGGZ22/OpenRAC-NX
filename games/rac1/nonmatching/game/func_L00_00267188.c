/* Stream handle callback (MusicStartCallback): stores the handle at +0 of the player; a zero handle sets state 0; state 1 (requested) becomes 8 (queue body). The exit is taken as a plain return. */
void func_L00_00267188(s32 handle, char *p) {
    if (p == 0) {
        return;
    }
    *(s32 *)p = handle;
    if (handle == 0) {
        *(s16 *)(p + 0xA) = 0;
        return;
    }
    if (*(s16 *)(p + 0xA) == 1) {
        *(s16 *)(p + 0xA) = 8;
    }
}
