int draw_menu_icon_list(char *arg0) {
    int x = (*(int *)(arg0 + 0x20) * 16 - 0x200) >> 1;
    int y = *(int *)(arg0 + 0x5C);
    int top = y;
    int i;

    SetupGifPaging(0);
    for (i = 0; i < *(int *)(arg0 + 0x40); i++) {
        char *e = *(char **)(arg0 + 0x48) + i * 10;

        if (*(int *)(arg0 + 0x3C) == i) {
            func_00201640(x - 0x30, y - 0x30, x + 0x230, y + 0x230,
                          (unsigned int)((subtract_integer_with_clamp((D_0015F538 & 0x3F) - 0x20) + 0x40)
                                         * 0x10202 | 0x80000000), 1);
            func_00201640(x - 0x10, y - 0x10, x + 0x210, y + 0x210,
                          *(int *)&D_001602B0, 1);
        }
        func_002008B8(GetIconFrame(*(unsigned short *)e, *(short *)(e + 2)), x, y,
                      0x200, 0x200, 0x80);
        y += 0x260;
    }
    if (top < 0) {
        func_00201640(0, 0, *(int *)(arg0 + 0x20), 0x14, *(int *)&D_001602B0, 0);
        HudSprite(GetIconFrame(0xE99E, 6), x >> 4, 2, 0x20, 0x10, 0x80);
    }
    if (*(int *)(arg0 + 0x24) * 16 < y) {
        func_00201640(0, *(int *)(arg0 + 0x24) - 0x14, *(int *)(arg0 + 0x20),
                      *(int *)(arg0 + 0x24), *(int *)&D_001602B0, 0);
        draw_hud_sprite_flipped(GetIconFrame(0xE99E, 6), x >> 4, *(int *)(arg0 + 0x24) - 0x12,
                      0x20, 0x10, 0x80);
    }
    DoGifPaging();
    return 2;
}
