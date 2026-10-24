int draw_selected_menu_item_text(char *arg0) {
    char *p = *(char **)(D_001D5F74 + 0x40);
    int row = *(int *)(*(char **)(p + 0x34) + *(int *)(p + 0x40) * 12 + 4);

    SetupGifPaging(0);
    if (row == -1) {
        FontPrintCenter(*(int *)(arg0 + 0x20) / 2, *(int *)(arg0 + 0x24) / 2 - 8,
                      0x80FFA888L, func_001FE540_id(0x501A), -1);
    } else {
        FontPrintCenter(*(int *)(arg0 + 0x20) / 2, *(int *)(arg0 + 0x24) / 3 - 8,
                      0x80FFA888L, func_001FE540_id(D_001DE0C0_i[row].a), -1);
        FontPrintCenter(*(int *)(arg0 + 0x20) / 2, *(int *)(arg0 + 0x24) * 2 / 3 - 8,
                      0x80FFA888L, func_001FE540_id(D_001DE0C0_i[row].b), -1);
    }
    DoGifPaging();
    return 2;
}
