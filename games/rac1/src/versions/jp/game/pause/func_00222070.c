s32 DrawCheatsMenu(struct CheatsMenu *menu) {
    FontWindow text_window;
    s16 window_fields[12];
    struct CheatMenuEntry *entry;
    s32 entry_count;
    s32 entry_index;
    s32 draw_index;
    s32 draw_y;
    s32 line_spacing;
    s32 color;
    s32 enabled;

    func_00234C98_22070(0x47, 0x2004B);
    SetupGifPaging(0);
    if ((menu->flags & 1) && menu->entries->text_id == 0) {
        func_001153FC_22070(window_fields, 0, sizeof(window_fields));
        window_fields[1] = menu->height + 1;
        window_fields[3] = menu->width + 1;
        window_fields[4] = menu->width >> 1;
        window_fields[0] = 1;
        window_fields[2] = 1;
        window_fields[5] = menu->height / 3;
        window_fields[8] = 16;
        window_fields[9] = 1;

        text_window = *(FontWindow *)window_fields;
        func_001F7560_22070(&text_window, 0x80FFA888, func_001FE540_22070(0x4FC1), -1);
    }
    /* Entries end at a zero text ID; each retail entry occupies 0x14 bytes. */
    entry_count = 0;
    while (menu->entries[entry_count].text_id != 0) {
        entry_count++;
    }
    line_spacing = menu->height / (entry_count + 1);
    draw_y = menu->height / (entry_count + 1) - 8;
    entry_index = 0;
    draw_index = 0;
    if (menu->entries[0].text_id != 0) {
        do {
            entry = &menu->entries[draw_index];
            color = entry_index == menu->selected_entry ? 0x8020FFFF : 0x80FFA888;
            enabled = 0;
            if (entry->enabled_flag != 0) {
                enabled = *entry->enabled_flag;
            }
            func_001F68E8_22070(0xC, draw_y, color, func_001FE540_22070(entry->text_id), -1);
            font_print_right(
                menu->width - 0xC, draw_y, 0x80FFA888,
                func_001FE540_22070(enabled ? entry->enabled_text_id : entry->disabled_text_id),
                -1);
            draw_y += line_spacing;
            draw_index++;
            entry_index++;
        } while (menu->entries[entry_index].text_id != 0);
    }
    DoGifPaging();
    return 2;
}
