int DrawQuitGameMenu(char *arg0) {
    short box[12];
    int w = *(int *)(arg0 + 0x20);
    int h = *(int *)(arg0 + 0x24);
    int cx;
    int y1, y2, y3;
    int a, b, x;

    func_001153FC(box, 0, 0x18);
    cx = w / 2;
    y1 = h - 0x40;
    y2 = h - 0x28;
    y3 = h - 0x14;
    box[2] = 4;
    box[3] = w - 4;
    box[4] = cx;
    box[5] = 6;
    box[6] = w;
    box[7] = h;
    box[8] = 0x10;
    box[9] = 1;
    box[1] = h;
    SetupGifPaging(0);
    func_00234C98_l(0x47, 0x2004B);
    font_print_window_regular(box, 0x80FFA888L, func_001FE540_id(0x4F6E), -1);
    FontPrintCenter(cx, y1, 0x80FFA888L, func_001FE540_id(0x4F40), -1);
    a = measure_text_width_regular(func_001FE540_id(0x52A2), -1);
    b = measure_text_width_regular(func_001FE540_id(0x52A6), -1);
    if (b >= a) {
        a = b;
    }
    x = (*(int *)(arg0 + 0x20) - a) >> 1;
    func_001F68E8_c(x, y2, 0x80FFA888L, func_001FE540_id(0x52A2), -1);
    func_001F68E8_c(x, y3, 0x80FFA888L, func_001FE540_id(0x52A6), -1);
    DoGifPaging();
    return 2;
}
