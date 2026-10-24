s32 draw_save_slot_list(SaveMenu_24C30 *menu) {
    char text[0x50];
    SaveSlot_24C30 *slot;
    s32 color;
    s32 width;
    s32 inner_right;
    s32 highlight_top;
    s32 highlight_bottom;
    s32 inner_top;
    s32 inner_bottom;
    s32 i;
    s32 y;
    s32 play_ticks;
    s32 hours;
    s32 bolts;
    s32 minutes;

    y = 4;
    func_001F4630_24C30(0);
    for (i = 0; i < 5; i++) {
        width = menu->width;
        highlight_top = y - 4;
        highlight_bottom = y + 0x34;
        inner_right = width - 3;
        inner_top = y - 1;
        inner_bottom = y + 0x31;
        if (D_0013D390_24C30.xDC < 3 && D_0013D390_24C30.xE4 < 0 && D_001D5F70_24C30.unk154 >= 0xB &&
            menu->selected_slot == i) {
            func_00201640_24C30(0, highlight_top, width, highlight_bottom, 0x8020FFFF, 0);
            func_00201640_24C30(3, inner_top, inner_right, inner_bottom, D_001602B0_24C30, 0);
        }
        func_00201640_24C30(3, inner_top, inner_right, inner_bottom, 0x80303030, 0);
        slot = &D_0013D390_24C30.slots[i];
        if (D_0013D390_24C30.xDC >= 3 || D_0013D390_24C30.xE4 >= 0 || D_001D5F70_24C30.unk154 < 0xB ||
            D_0013D390_24C30.state != 2) {
            y += 0x30;
        } else {
            if (slot->id == -1) {
                y += 0x10;
                func_001F6F40_24C30(menu->width / 2, y, 0x80FFA888,
                                        func_001FE540_24C30(0x526D), -1);
                y += 0x20;
            } else {
                play_ticks = slot->time;
                hours = play_ticks / 216000;
                minutes = play_ticks / 3600 - hours * 60;
                color = menu->selected_slot == i ? (s32)0x8020FFFF : (s32)0x80FFA888;
                if (hours > 99) {
                    hours = 99;
                    minutes = 59;
                }
                func_00116248_24C30(text, D_00160400_24C30, hours, minutes);
                func_00200468_24C30(func_00200198_24C30(0xE99E, 3), 4, y, 0x10, 0x10,
                                0x80);
                func_001F6968_24C30(0x16, y, color, text, -1);
                if (slot->count != 0) {
                    func_00200468_24C30(func_00200198_24C30(0xE99E, 4), 0x4E, y, 0x10,
                                    0x10, 0x80);
                    func_00116248_24C30(text, D_001603A0_24C30, slot->count < 100 ? slot->count : 99);
                    func_001F6968_24C30(0x60, y, color, text, -1);
                }
                bolts = slot->bolts;
                if (bolts > 9999999) {
                    bolts = 9999999;
                }
                y += 0x10;
                if (bolts < 1000) {
                    func_00116248_24C30(text, D_001603A0_24C30, bolts);
                } else if (bolts < 1000000) {
                    func_00116248_24C30(text, D_001603D0_24C30, bolts / 1000, bolts % 1000);
                } else {
                    func_00116248_24C30(text, D_00160410_24C30, bolts / 1000000, bolts % 1000000 / 1000,
                            bolts % 1000);
                }
                func_00200468_24C30(func_00200198_24C30(0x754F, 0xF), 4, y, 0x10, 0x10,
                                0x80);
                func_001F6968_24C30(0x16, y, color, text, -1);
                y += 0x10;
                func_00116248_24C30(text, D_00160420_24C30, slot->b16, slot->b15, slot->b17);
                func_00200468_24C30(func_00200198_24C30(0xE99E, 2), 4, y, 0x10, 0x10,
                                0x80);
                func_001F6968_24C30(0x16, y, color, text, -1);
                y += 0x10;
            }
        }
        if (D_0015EE80_24C30 != 0) {
            y += 0x24;
        } else {
            y += 0x1B;
        }
    }
    func_001F4748_24C30();
    return 2;
}
