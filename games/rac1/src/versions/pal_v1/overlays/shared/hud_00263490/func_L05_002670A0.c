int draw_hud_timer(HudElem_002670A0 *e) {
    char buf[64];
    int y0, y1, y2;
    int frames, per_min, per_sec;
    int min, sec, hund;
    int x;

    if (e->unk6C <= 0) {
        return 0;
    }
    y0 = -(D_0015EE80_002670A0 ? D_L05_0015FAD0_002670A0 + 10 : D_L05_0015FAD0_002670A0 + 18) + D_0013E600_002670A0[1];
    y1 = -(D_0015EE80_002670A0 ? D_L05_0015FAEC_002670A0 + 10 : D_L05_0015FAEC_002670A0 + 18) + D_0013E600_002670A0[1];
    y2 = -(D_0015EE80_002670A0 ? D_L05_0015FAFC_002670A0 + 10 : D_L05_0015FAFC_002670A0 + 18) + D_0013E600_002670A0[1];
    per_min = D_0015EE80_002670A0 ? 3000 : 3600;
    per_sec = per_min / 60;
    frames = hero_002670A0.unk894;
    min = frames / per_min;
    frames -= min * per_min;
    sec = frames / per_sec;
    frames -= sec * per_sec;
    hund = frames * 100 / per_sec;

    if (D_0013D490_002670A0[0]) {
        func_00201960_002670A0(D_L05_0015FACC_002670A0, y0, D_L05_0015FAD4_002670A0, D_L05_0015FAD8_002670A0, D_L05_0015FADC_002670A0);
        x = D_L05_0015FAE8_002670A0;
        func_00116248_002670A0(buf, D_L05_0015F868_002670A0, func_001FE540_002670A0(0x50A6), hero_002670A0.unk8A8);
        x -= func_001F6600_002670A0(buf, -1) >> 1;
        func_00116248_002670A0(buf, D_L05_0015FAB0_002670A0, func_001FE540_002670A0(0x50A6));
        func_L00_001FB7F8_002670A0(x + 1, y1 + 1, 0x80000000, buf, -1);
        func_L00_001FB7F8_002670A0(x, y1, D_L05_0015FAE0_002670A0, buf, -1);
        x += func_001F6600_002670A0(buf, -1);
        func_00116248_002670A0(buf, D_L05_0015FB08_002670A0, hero_002670A0.unk8A8);
        func_L00_001FB7F8_002670A0(x + 1, y1 + 1, 0x80000000, buf, -1);
        func_L00_001FB7F8_002670A0(x, y1, D_L05_0015FAE4_002670A0, buf, -1);
    }
    x = D_L05_0015FAE8_002670A0;
    func_00116248_002670A0(buf, D_L05_0015FB10_002670A0, func_001FE540_002670A0(0x5241));
    x -= func_001F6600_002670A0(buf, -1) >> 1;
    func_00116248_002670A0(buf, D_L05_0015FB20_002670A0, func_001FE540_002670A0(0x5241));
    func_L00_001FB7F8_002670A0(x + 1, y2 + 1, 0x80000000, buf, -1);
    func_L00_001FB7F8_002670A0(x, y2, D_L05_0015FAE0_002670A0, buf, -1);
    x += func_001F6600_002670A0(buf, -1);
    func_00116248_002670A0(buf, D_L05_0015FB28_002670A0, min, sec, hund);
    func_L00_001FB7F8_002670A0(x + 1, y2 + 1, 0x80000000, buf, -1);
    func_L00_001FB7F8_002670A0(x, y2, D_L05_0015FAE4_002670A0, buf, -1);
    return e->w;
}
