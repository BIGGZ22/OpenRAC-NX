s32 DrawItemsMenu(ItemsMenu *menu) {
    char text_buffer[0x80];
    char *hyphen;
    char *source_end;
    char *destination_end;
    char *format;
    s32 text_length;
    s32 column_divisor;
    s32 x;
    s32 y;
    s32 collected_count;
    if (menu->help_tip != 0) {
        DrawMobyList(menu->help_tip, 1);
    }
    SetupGifPaging(0);
    column_divisor = 3;
    {
        /* Retail clears the 24-byte text window, fills the height, left (8), signed
           width / 3, line advance 0x10 and flags 5, copies it, then centres the anchor. */
        struct TextRegion text_window;
        struct TextRegion window_fields;

        func_001153FC_1FB28(&window_fields, 0, sizeof(window_fields));
        window_fields.bottom = menu->height;
        window_fields.left = 8;
        window_fields.right = divide_coordinate(menu->width, column_divisor);
        window_fields.line_advance = 0x10;
        window_fields.flags = 5;
        text_window = window_fields;
        text_window.anchor_x = add_offset(text_window.left, text_window.right) >> 1;
        func_001166FC_1FB28(text_buffer, func_001FE540_1FB28(0x4F4F));
        if (D_0015EE88_1FB28[0] == column_divisor) {
            hyphen = func_00116428(text_buffer, 0x2D);
            if (hyphen != 0) {
                text_length = func_00116810_1FB28(text_buffer);
                source_end = text_buffer + text_length;
                if (hyphen < source_end) {
                    destination_end = source_end;
                    do {
                        destination_end[1] = *source_end;
                        destination_end--;
                        text_length--;
                        source_end = destination_end;
                    } while (hyphen < source_end);
                }
                text_buffer[add_offset(text_length, 1)] = 0x20;
            }
        }
        func_001F7560_1FB28(&text_window, 0x8000C0C0L, text_buffer, -1);
        text_window.flags ^= 4;
        text_window.anchor_y = (D_0013E600_1FB28.height - text_window.rendered_height) >> 1;
        func_001F7560_1FB28(&text_window, 0x8000C0C0L, text_buffer, -1);
    }
    x = add_offset((*(s32 *)&D_001602B8), 0xC8);
    y = add_offset((*(s32 *)&D_001602BC), 0x1D);
    font_print_right(x, y, 0x80000000L, func_001FE540_1FB28(0x4F50), -1);
    x = add_offset((*(s32 *)&D_001602B8), 0xC8);
    y = add_offset((*(s32 *)&D_001602BC), 0x36);
    font_print_right(x, y, 0x80000000L, func_001FE540_1FB28(0x4F51), -1);
    x = add_offset((*(s32 *)&D_001602B8), 0xC8);
    y = add_offset((*(s32 *)&D_001602BC), 0x54);
    font_print_right(x, y, 0x80000000L, func_001FE540_1FB28(0x4F52), -1);
    font_print_right(0xC8, 0x1D, 0x80FFA888L, func_001FE540_1FB28(0x4F50), -1);
    font_print_right(0xC8, 0x36, 0x80FFA888L, func_001FE540_1FB28(0x4F51), -1);
    font_print_right(0xC8, 0x54, 0x80FFA888L, func_001FE540_1FB28(0x4F52), -1);
    collected_count = count_nonzero_entries_up_to_40();
    func_00116248(text_buffer, D_001603A0, collected_count);
    font_print_right_large(add_offset((*(s32 *)&D_001602B8), 0xF0), add_offset((*(s32 *)&D_001602BC), 0x1D), 0x80000000L,
                    text_buffer, -1);
    font_print_right_large(0xF0, 0x1D, 0x80FFA888L, text_buffer, -1);
    func_00116248(text_buffer, D_001603A0, count_nonzero_entries_up_to_10() * 4);
    font_print_right_large(add_offset((*(s32 *)&D_001602B8), 0xF0), add_offset((*(s32 *)&D_001602BC), 0x36), 0x80000000L,
                    text_buffer, -1);
    font_print_right_large(0xF0, 0x36, 0x80FFA888L, text_buffer, -1);
    func_00116248(text_buffer, D_001603A0, compute_clamped_count_difference());
    font_print_right_large(add_offset((*(s32 *)&D_001602B8), 0xF0), add_offset((*(s32 *)&D_001602BC), 0x54), 0x80000000L,
                    text_buffer, -1);
    font_print_right_large(0xF0, 0x54, 0x80FFA888L, text_buffer, -1);
    func_00201640_1FB28(add_offset((*(s32 *)&D_001602B8), 0xD0), add_offset((*(s32 *)&D_001602BC), 0x4D),
                              add_offset((*(s32 *)&D_001602B8), 0xF2), add_offset((*(s32 *)&D_001602BC), 0x50),
                              0x80000000L, 0);
    func_00201640_1FB28(0xD0, 0x4D, 0xF2, 0x50, 0x80FFA888L, 0);
    DoGifPaging();
    return 8;
}
