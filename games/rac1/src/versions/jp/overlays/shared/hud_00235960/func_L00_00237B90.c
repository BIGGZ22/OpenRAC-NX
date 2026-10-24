int draw_hud_text_panel(char *e) {
    int w1, tex, l1, y, i, width, left, w2, l2, alpha, color, c, level, elapsed, score, interval, per, seconds, minutes, rem, idx;
    float fade;
    HudPoint_237B90 points[2];
    HudPoint_237B90 point;
    char text[64];
    int text_y;
    void *pt;
    unsigned char *opacity = (unsigned char *)e + 0x70;

    if (D_0015EE80 != 0) {
        y = *(int *)&D_L00_0015F850 + 0xa;
    } else {
        y = *(int *)&D_L00_0015F850 + 0x12;
    }
    fade = IntToFloat(*opacity);
    fade /= func_001F9878(8.0f);
    if (fade < 0.0f) fade = 0.0f;
    if (fade > 1.0f) fade = 1.0f;
    text_y = y + 8;
    i = 0;
    alpha = truncate_float_to_s32(fade * 128.0f);
    color = (alpha << 24) | 0x40f040;
    if (D_L00_0017E630[0] >= 2) {
        do {
            i++;
        } while (D_L00_0017E630[i] >= 2 && i < 0x80);
    }
    if (D_L00_0017E630[i] == 1) {
        D_L00_0017E630[i] = 0;
        w1 = measure_text_width_regular((const char *)D_L00_0017E630, -1);
        l1 = 0xe0 - (w1 >> 1);
        w2 = measure_text_width_regular((const char *)&D_L00_0017E630[1] + i, -1);
        l2 = 0xe0 - (w2 >> 1);
        if (l1 >= l2) {
            left = l2;
            width = w2;
        } else {
            left = l1;
            width = w1;
        }
        HudSprite(GetIconFrame(0x7580, 0), left + 0x20, y, width, 0x36, alpha);
        c = color;
        HudSprite(GetIconFrame(0x7580, 1), left, y, 0x20, 0x36, alpha);
        tex = GetIconFrame(0x7580, 1);
        {
            int t = width + 0x20;
            draw_hud_sprite_flipped(tex, left + t, y, 0x20, 0x36, alpha);
        }
        FontPrintCenter(0x100, text_y, c, D_L00_0017E630, -1);
        FontPrintCenter(0x100, y + 0x1b, c, &D_L00_0017E630[1] + i, -1);
    } else {
        int w, x;
        w = measure_text_width_regular((const char *)D_L00_0017E630, -1);
        x = 0xe0 - (w >> 1);
        HudSprite(GetIconFrame(0x7580, 1), x, y, 0x20, 0x20, alpha);
        HudSprite(GetIconFrame(0x7580, 0), x + 0x20, y, w, 0x20, alpha);
        tex = GetIconFrame(0x7580, 1);
        {
            int t = w + 0x20;
            draw_hud_sprite_flipped(tex, x + t, y, 0x20, 0x20, alpha);
        }
        FontPrintCenter(0x100, text_y, color, D_L00_0017E630, -1);
    }
    level = D_0015EE84;
    if (level == 5 || level == 0x10) {
        typedef unsigned int quadword __attribute__((mode(TI), aligned(16)));
        *(quadword *)&points[0] = 0;
        *(quadword *)&points[1] = 0;
        points[0].x = 294.5f;
        points[0].y = 225.4f;
        points[0].z = 100.0f;
        points[1].x = 115.7f;
        points[1].y = 287.0f;
        points[1].z = 119.5f;
        if (level == 5) {
            qcopy(&point, &points[0]);
        } else {
            qcopy(&point, &points[1]);
        }
        pt = &point;
        if (FastVecDist(gHeroPos, pt) < 5.0f) {
            D_L00_0015F728 = y + 100;
            elapsed = D_0015EF58[D_0015EE84 == 0x10 ? 1 : 0];
            score = D_0015EF68[D_0015EE84 == 0x10 ? 1 : 0];
            if (elapsed != 0) {
                interval = D_0015EE80 != 0 ? 3000 : 3600;
                per = interval / 60;
                seconds = elapsed / interval;
                rem = elapsed - seconds * interval;
                minutes = rem / per;
                rem = (rem - minutes * per) * 100 / per;
                func_00116248(text, D_L00_001E8840, msg_string(0x5372), seconds, minutes, rem);
                draw_framed_text(0x100, y + 0x28, color, text);
                if (score != 0) {
                    func_00116248(text, D_L00_0015F858, msg_string(0x5373), score);
                    draw_framed_text(0x100, y + 0x48, color, text);
                }
            }
        }
    }
    return *(int *)(e + 0x58);
}
