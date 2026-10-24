int draw_hud_progress_and_rank(char *e) {
    char buf[0x50] __attribute__((aligned(16)));
    int x, y, y1, tx, px, py, w;
    int pal;
    char *hd;
    char *t, *f, *h2;

    if (*(int *)(e + 0x6C) <= 0)
        return 0;
    pal = D_0015EE80;
    x = hud_sub_66D10(D_0013E600[1], pal, &D_L05_0015FA60);
    y = hud_sub_66D10(D_0013E600[1], pal, &D_L05_0015FA7C);
    hd = D_0013E633 + 0xE1D;
    y1 = y + 1;
    draw_stretchable_ui_frame(*(int *)&(D_L05_0015FA5C), x, *(int *)&(D_L05_0015FA64), *(int *)&(D_L05_0015FA68), *(int *)&(D_L05_0015FA6C));
    tx = *(int *)&(D_L05_0015FA78);
    func_00116248(buf, D_L05_0015FAB0, msg_string(0x5296));
    print_text_with_texture_3(tx - 1, y1, 0x80000000L, buf, -1);
    print_text_with_texture_3(tx, y, *(int *)&(D_L05_0015FA70), buf, -1);
    w = measure_text_width_regular(buf, -1);
    tx = tx + w;
    {
        int n = *(short *)(hd + 0x89A) + 1;
        if (!(n < 4)) n = 3;
        func_00116248(buf, D_L05_0015FAB8, n);
    }
    print_text_with_texture_3(tx - 1, y + 2, 0x80000000L, buf, -1);
    print_text_with_texture_3(tx, y, *(int *)&(D_L05_0015FA74), buf, -1);
    {
        int n = *(int *)(hd + 0x8C0);
        tx = *(int *)&(D_L05_0015FA88);
        n = n - 1;
        if (!(n < 5)) n = 4;
        func_00116248(buf, D_L05_0015F9B8, msg_string(n + 0x527A));
    }
    w = measure_text_width_regular(buf, -1);
    tx = tx - w;
    print_text_with_texture_3(tx + 1, y1, 0x80000000L, buf, -1);
    print_text_with_texture_3(tx, y, *(int *)&(D_L05_0015FA74), buf, -1);
    func_00116248(buf, D_L05_0015FAC0, msg_string(0x5297));
    w = measure_text_width_regular(buf, -1);
    tx = tx - w;
    print_text_with_texture_3(tx + 1, y1, 0x80000000L, buf, -1);
    print_text_with_texture_3(tx, y, *(int *)&(D_L05_0015FA70), buf, -1);
    if (D_0015EE84 != 0x10)
        return *(int *)(e + 0x58);
    func_00116248(buf + 0x40, D_L05_0015F940, *(unsigned char *)(hd + 0x8CE));
    py = D_0015EE80 ? 0xA : 0x12;
    px = *(int *)(e + 0x50);
    if (*(unsigned char *)(D_0013D355 + 0x13B) == 0) {
        py = D_0015EE80 ? 0xA : 0x12;
    } else {
        px = *(int *)&(D_L05_0015FA98);
        py = *(int *)&(D_L05_0015FA9C);
    }
    draw_stretchable_ui_frame(px - 0x18, py, *(int *)&(D_L05_0015FAA0), 0x20, 0x60);
    w = GetIconFrame(0x7558, 2);
    HudSprite(w, px, py, 0x20, 0x20, 0x80);
    print_text_with_texture_3(px + 0x29, py + 8, 0x80000000L, buf + 0x40, -1);
    h2 = D_0013E633 + 0xE1D;
    if (*(unsigned char *)(h2 + 0x8CE) != 0)
        print_text_with_texture_3(px + 0x28, py + 7, 0x80E08060L, buf + 0x40, -1);
    else
        print_text_with_texture_3(px + 0x28, py + 7, 0x80202080L, buf + 0x40, -1);
    return *(int *)(e + 0x58);
}
