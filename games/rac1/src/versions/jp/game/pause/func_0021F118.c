int draw_item_preview_screen(char *arg0) {
    short buf[10];
    char *p = *(char **)(D_001D5F74 + 0x40);

    if (((unsigned char *)&D_0013D5C8)[*(short *)(*(int *)(p + 0x3C) * 10 + *(char **)(p + 0x48) + 6)] == 0) {
        return 0;
    }
    if (*(int *)(arg0 + 0x44) != 0) {
        DrawMobyList(*(int *)(arg0 + 0x44), 1);
        if (*(int *)(arg0 + 0x48) != 0) {
            DrawMobyList(*(int *)(arg0 + 0x48), 1);
        }
        return 8;
    }
    SetupGifPaging(0);
    pack_image_descriptor(buf, arg0);
    buf[8] = 0x10;
    buf[9] = 3;
    font_print_window_regular(buf, 0x80FFA888L, func_001FE540_id(0x4F4E), -1);
    DoGifPaging();
    return 2;
}
