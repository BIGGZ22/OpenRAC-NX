int draw_checking_memory_card_data_menu(struct Stream_21380 *menu) {
    FontWindow_21380 text_window;
    short window_fields[12];
    struct Cell_21380 *entry;
    char *text;
    int state = menu->state;
    long texture;
    extern void func_001F75D0(FontWindow_21380 *, long, char *, int);
    int first;
    unsigned char *tp;

    if (state < 2) {
        if (!(menu->flags & 0x100)) {
            return 1;
        }
        if (((struct Card_21380 *)memory_card_state)->type != 2) {
            goto done;
        }
        if (((struct Card_21380 *)memory_card_state)->state < 3 && ((struct Card_21380 *)memory_card_state)->pending_state < 0 && ((struct MS_21380 *)menu_system)->ticks >= 0xB) {
            goto done;
        }
        SetupGifPaging(0);
        text = ((char *(*)(int))msg_string)(((struct MS_21380 *)menu_system)->card_op_pending ? ((struct MS_21380 *)menu_system)->card_op_text : 0x4FBA);
        ((void *(*)(void *, int, unsigned int))func_001153FC)(window_fields, 0, sizeof(window_fields));
        window_fields[1] = menu->height + 1;
        window_fields[0] = 1;
        window_fields[2] = 1;
        window_fields[3] = menu->width + 1;
        window_fields[4] = menu->width >> 1;
        window_fields[5] = 5;
        window_fields[8] = 16;
        window_fields[9] = 5;
        text_window = *(FontWindow_21380 *)window_fields;
        func_001F75D0(&text_window, 0x80000000L, text, -1);
        text_window.anchor_y = (menu->height - text_window.rendered_height) >> 1;
        text_window.flags ^= 4;
        func_001F75D0(&text_window, 0x80000000L, text, -1);
        text_window.top--;
        text_window.bottom--;
        text_window.left--;
        text_window.right--;
        text_window.anchor_x--;
        text_window.anchor_y--;
        func_001F75D0(&text_window, 0x80FFA888L, text, -1);
        DoGifPaging();
        goto done;
    }
    if (menu->flags & 4) {
        first = state < 4;
        entry = &((struct MS_21380 *)menu_system)->current->focus->cells[menu->loaded_entry[first ^ 1]];
        if (entry->kind == 0 && (tp = (unsigned char *)&D_0013D5C8, tp[entry->id] == 0)) {
            return 1;
        }
        if (entry->kind == 1 && gSpecialItems[entry->id] == 0) {
            return 1;
        }
    }
    SetupGifPaging(0);
    if (menu->state < 4) {
        texture = load_texture_file(menu->buffer[0]);
    } else {
        texture = load_texture_file(menu->buffer[1]);
    }
    DrawTexturedQuad(0, 0, D_00151880[0xB0], D_00151880[0xB1], 0, 0, menu->tw, menu->th, 0x80808080L, texture);
    DoGifPaging();
    return 0x10;
done:
    return 2;
}
