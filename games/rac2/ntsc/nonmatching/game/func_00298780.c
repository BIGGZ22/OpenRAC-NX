/* from src/game/00298780.c: C that compiles but does not link at its places yet */
/* When mode at data offset 0x15C equals 2 and the signed selection at offset 0x164 is negative, write selection 9, clear the word at offset 0x168 and write absolute state value 15. */
void func_00298780(void)
{
    if (D_001393E0_1.mode_15c == 2 &&
        D_001393E0_1.selected_164 < 0) {
        D_001393E0_1.selected_164 = 9;
        D_001393E0_1.value_168 = 0;
        D_001A7420_5 = 15;
    }
}
