int draw_trick_combo_hud(char *a) {
    unsigned char *u = (unsigned char *)(a + 0x70);
    char buf[0x80];
    int out[4];
    int xa, xb;
    float f20, f21, f0, f12;
    char *E;
    int r, sel, cnt, k, idx, h, acc, id, id2, i, s16, r3, t;

    f21 = (float)u[0] / 10.0f;
    if (f21 > 1.0f) {
        f21 = 1.0f;
    }
    E = D_0013E633 + 0xE1D;
    if (*(int *)(E + 0x208C) != 0x16) {
        return *(int *)(a + 0x58);
    }
    if (u[2] != 0) {
        func_00116248_002670A0(buf, D_L05_0015F940, D_L05_0015FB8C * 0x168);
        f20 = IntToFloat(u[1]);
        r = scale_ticks(*(int *)&D_L05_0015F95C);
        f12 = f20 / (float)r;
        if (u[2] == 1 || u[2] == 3) {
            acc = FastTweenColor(*(int *)&D_L05_0015F974, *(int *)&D_L05_0015F978, f12);
        } else {
            acc = *(int *)&D_L05_0015F980;
        }
        h = func_001FA898_r(f21 * 128.0f) << 24;
        acc += h;
        if (u[2] == 1 || u[2] == 2) {
            func_001F6FD8_x(*(int *)&D_L05_0015F994 + 1, *(int *)&D_L05_0015F998 + 1, h, (int)buf, -1);
            func_001F6FD8_x(*(int *)&D_L05_0015F994, *(int *)&D_L05_0015F998, acc, (int)buf, -1);
            func_00116248_002670A0(buf, D_L05_0015F9B0, D_L05_0015FB90,
                                   func_001FE540_002670A0(D_L05_0015FB90 == 1 ? 0x5177 : 0x5178));
            func_001F6FD8_x(*(int *)&D_L05_0015F99C + 1, *(int *)&D_L05_0015F9A0 + 1, h, (int)buf, -1);
            func_001F6FD8_x(*(int *)&D_L05_0015F99C, *(int *)&D_L05_0015F9A0, acc, (int)buf, -1);
        } else if (u[2] == 3) {
            sel = -1;
            if (D_L05_0015FB90 == 3) {
                sel = 0;
                if (D_L05_0015FBA4 != 0) {
                    sel = 2;
                    if (D_L05_0015FBA0 == 0) {
                        sel = 1;
                    } else if (D_L05_0015FB9C != 0) {
                        sel = 3;
                    }
                }
            } else if (D_L05_0015FB90 == 4) {
                sel = 4;
            }
            if (sel >= 0) {
                if (D_L05_0015FB8C >= 4) {
                    k = 2;
                } else {
                    k = D_L05_0015FB8C >= 2;
                }
                idx = sel + k * 5;
                D_L05_0015FBA8 = idx;
                D_L05_0015FBB8 = D_L05_0017EA18[idx];
                cnt = 1;
                for (i = 1; i < 4; i++) {
                    if ((&D_L05_0015FBA8)[i] == D_L05_0015FBA8) {
                        cnt++;
                    }
                }
                D_L05_0015FBB8 = D_L05_0015FBB8 / cnt;
                func_00116248_002670A0(buf, D_L05_0015F940, D_L05_0015FBB8);
                draw_framed_text(*(int *)&D_L05_0015F9A4, *(int *)&D_L05_0015F9A8 - 0x18, acc, (int)buf);
                func_00116248_002670A0(buf, D_L05_0015F9B8, func_001FE540_002670A0(idx + 0x5092));
                draw_framed_text(*(int *)&D_L05_0015F9A4, *(int *)&D_L05_0015F9A8, acc, (int)buf);
            } else if (D_L05_0015FB90 > 0) {
                int n = 0;
                int m;
                for (m = 0; m < 4 && n < D_L05_0015FB90; m++) {
                    if ((&D_L05_0015FB98)[m] != 0) {
                        out[n] = m;
                        n++;
                    }
                }
                if (D_L05_0015FB90 == 1) {
                    D_L05_0015FBB8 = D_L05_0015FB8C * 10;
                    func_00116248_002670A0(buf, D_L05_0015F940, D_L05_0015FBB8);
                    draw_framed_text(*(int *)&D_L05_0015F9A4, *(int *)&D_L05_0015F9A8 - 0x18, acc, (int)buf);
                    func_00116248_002670A0(buf, D_L05_0015F9C0, func_001FE540_002670A0(out[0] + 0x517E), D_L05_0015FB8C,
                                           func_001FE540_002670A0(D_L05_0015FB8C == 0x168 ? 0x50A7 : 0x50A8));
                    draw_framed_text(*(int *)&D_L05_0015F9A4, *(int *)&D_L05_0015F9A8, acc, (int)buf);
                } else {
                    D_L05_0015FBB8 = D_L05_0015FB8C * 20;
                    func_00116248_002670A0(buf, D_L05_0015F940, D_L05_0015FBB8);
                    draw_framed_text(*(int *)&D_L05_0015F9A4, *(int *)&D_L05_0015F9A8 - 0x18, acc, (int)buf);
                    func_00116248_002670A0(buf, D_L05_00216FC0, func_001FE540_002670A0(out[0] + 0x517E),
                                           func_001FE540_002670A0(out[1] + 0x517E), D_L05_0015FB8C);
                    draw_framed_text(*(int *)&D_L05_0015F9A4, *(int *)&D_L05_0015F9A8, acc, (int)buf);
                }
            }
        }
    }

    xa = *(int *)(a + 0x50);
    xb = *(int *)(a + 0x54);
    *(int *)(a + 0x58) = 0x100;
    *(int *)(a + 0x5C) = 0x40;
    hud_screen_position(a, &xa, &xb);
    s16 = (*(int *)(a + 0x74) * 0xDD) / *(int *)(a + 0x8) + 0x1B;
    id = GetIconFrame(0x7558, 1);
    draw_cropped_hud_sprite(id, xa, xb, s16, 0x40, 0x80);
    id2 = GetIconFrame(0x7558, 0);
    HudSprite(id2, xa, xb, 0x100, 0x40, 0x80);
    E = D_0013E633 + 0xE1D;
    t = (unsigned char)E[0x8AE];
    if (scale_ticks(0x78) < t) {
        r3 = scale_ticks(0x3C);
        f0 = IntToFloat(D_L05_0015F6B0 % r3) / (float)r3;
        f0 = FastSin(f0 * 6.283180236816406f - 3.141590118408203f);
        r3 = FastTweenColor(*(int *)&D_L05_0015F98C, *(int *)&D_L05_0015F990, f0 * 0.5f + 0.5f);
        func_00116248_002670A0(buf, func_001FE540_002670A0(0x531A));
        draw_framed_text(*(int *)&D_L05_0015F984 + 1, *(int *)&D_L05_0015F988 + 1, 0x80000000, (int)buf);
        draw_framed_text(*(int *)&D_L05_0015F984, *(int *)&D_L05_0015F988, r3, (int)buf);
    }
    return *(int *)(a + 0x58);
}
