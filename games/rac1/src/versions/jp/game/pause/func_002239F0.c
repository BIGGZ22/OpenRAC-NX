int ObtainAllGoldWeaponsMenu(char *arg0) {
    Box18 buf;
    Box18 tmp;
    int x, y;

    func_001153FC(&tmp, 0, 0x18);
    tmp.f2 = *(unsigned short *)(arg0 + 0x24);
    tmp.f6 = *(unsigned short *)(arg0 + 0x20);
    tmp.f10 = 0x10;
    buf = tmp;
    x = 0x18;
    y = 4;
    SetupGifPaging(0);
    HudSprite(GetIconFrame(0xE99A, 6), 4, y + 8, 0x10, 0x10, 0x80);
    buf.x = x;
    buf.y = y;
    font_print_window_regular(&buf, 0x80FFA888L, func_001FE540_id(0x51D8), -1);
    y = (short)buf.fE + 0x10;
    buf.y = y;
    HudSprite(GetIconFrame(0xE99A, 6), 4, y + 8, 0x10, 0x10, 0x80);
    font_print_window_regular(&buf, 0x80FFA888L, func_001FE540_id(0x51D9), -1);
    DoGifPaging();
    return 2;
}
