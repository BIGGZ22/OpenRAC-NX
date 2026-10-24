int draw_menu_selection_text(char *arg0) {
    char *g = menu_system;
    char *b = memory_card_state;
    int row = ((PageTbl *)b)->rec[*(int *)(*(char **)(*(char **)(g + 4) + 0x40) + 0x40)].row;

    SetupGifPaging(0);
    if (*(int *)(b + 0xDC) < 3 && *(int *)(b + 0xE4) < 0
        && *(int *)(g + 0x154) >= 0xB && *(int *)(b + 8) == 2) {
        if (row == -1) {
            FontPrintCenter(*(int *)(arg0 + 0x20) / 2, *(int *)(arg0 + 0x24) / 2 - 8,
                          0x80FFA888L, func_001FE540_id(0x526D), -1);
        } else {
            FontPrintCenter(*(int *)(arg0 + 0x20) / 2, 4, 0x80FFA888L,
                          func_001FE540_id(D_001DE0C0_i[row].a), -1);
            FontPrintCenter(*(int *)(arg0 + 0x20) / 2, 0x14, 0x80FFA888L,
                          func_001FE540_id(D_001DE0C0_i[row].b), -1);
        }
    }
    DoGifPaging();
    return 2;
}
