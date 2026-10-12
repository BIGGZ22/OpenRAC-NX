/* from src/game/002985F0.c: C that compiles but does not link at its places yet */
/* When mode at data offset 0x15C equals 2 and the signed selection at offset 0x164 is negative, write selection 7, clear the word at offset 0x168 and write absolute state value 11. */
void func_002985F0(void)
{
    if (D_001393E0.mode_15c == 2 &&
        D_001393E0.selected_164 < 0) {
        D_001393E0.selected_164 = 7;
        D_001393E0.value_168 = 0;
        D_001A7420 = 11;
    }
}
