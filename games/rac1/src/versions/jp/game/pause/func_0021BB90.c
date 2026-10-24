s32 update_menu_entry_actions(struct MenuScreen_1BB90 *menu) {
    s32 focused;
    s32 entry_index;
    s32 entry_count;
    s32 previous_selection;
    s32 flags;
    s32 buttons;
    s16 fade_timer;
    s16 message_index;

    focused = D_001D5F70_1BB90.current->focus == menu;
    for (entry_index = 0; menu->data.list.items[entry_index].text != 0; entry_index++) {
        if (focused && menu->data.list.selected == entry_index) {
            menu->data.list.items[entry_index].fade_timer =
                menu->data.list.items[entry_index].fade_timer + 1;
            if (menu->data.list.items[entry_index].fade_timer > 0x7D00) {
                menu->data.list.items[entry_index].fade_timer = 0x7D00;
            }
        } else {
            fade_timer = menu->data.list.items[entry_index].fade_timer;
            if (func_001F98C0_1BB90(D_001602B4_1BB90) < fade_timer) {
                menu->data.list.items[entry_index].fade_timer =
                    func_001F98C0_1BB90(D_001602B4_1BB90);
            }
            menu->data.list.items[entry_index].fade_timer =
                menu->data.list.items[entry_index].fade_timer > 0
                    ? menu->data.list.items[entry_index].fade_timer - 1
                    : 0;
        }
    }
    if (!focused) {
        return 0;
    }
    if (D_0013CA40_1BB90.pressed_unmasked & 0xD00) {
        if (menu->data.list.flags & 0x20) {
            D_001A0414_1BB90[0] = D_0015EE84_1BB90;
        }
        return -1;
    }
    if (D_0013CA40_1BB90.pressed_unmasked & 0x10) {
        if (menu->data.list.flags & 0x20) {
            D_001A0414_1BB90[0] = D_0015EE84_1BB90;
        }
        if (D_001D5F70_1BB90.current->back != 0) {
            D_001D5F70_1BB90.next = D_001D5F70_1BB90.current->back;
            return 0;
        } else if (D_001D5F70_1BB90.close_locked == 0) {
            return -1;
        }
    }
    if ((D_0013CA40_1BB90.pressed_unmasked & 0x60) && ((D_0015EFB4_1A1A0 ^ 1) & 1)) {
        /* Index through menu-> directly: locals for items/selected change
           which register keeps the copy of selected that actions 6 and 9 reuse. */
        switch (menu->data.list.items[menu->data.list.selected].action) {
        case 0:
            break;
        /* No case 1 in PAL: retail's table sends action 1 to the default. */
        case 3:
            D_001D5F70_1BB90.next =
                (struct MenuPage_1BB90 *)menu->data.list.items[menu->data.list.selected].param.value;
            break;
        case 4:
            func_0022ED80_1BB90(0, 0x11, menu->moby);
            D_001D5F70_1BB90.unk150 = 0;
            D_0015EFB4_1A1A0 = (D_0015EFB4_1A1A0 | 2) & ~4;
            func_001FBC80_1BB90(3, menu->data.list.items[menu->data.list.selected].param.value, 0);
            break;
        case 5:
            func_0022ED80_1BB90(0, 0x11, menu->moby);
            D_0015EFB4_1A1A0 = (D_0015EFB4_1A1A0 | 4) & ~2;
            func_001FBC80_1BB90(3, menu->data.list.items[menu->data.list.selected].param.value, 0);
            D_001D5F70_1BB90.unk150 = 1;
            break;
        case 6:
            /* This action passes the low halfword; the other indexed actions pass the full word. */
            message_index = menu->data.list.items[menu->data.list.selected].param.half.hi;
            if (message_index != 0) {
                D_001D5F70_1BB90.unkEC = D_00199578_1BB90[message_index];
            }
            D_001D5F70_1BB90.close_request = 5;
            D_001D5F70_1BB90.unkF0 = D_001D5F70_1BB90.current;
            D_001D5F70_1BB90.unkF4 = 0;
            D_001D5F70_1BB90.unkE4 =
                menu->data.list.items[menu->data.list.selected].param.half.lo;
            func_0022ED80_1BB90(0, 0x11, menu->moby);
            return 0;
        case 7:
            D_001D5F70_1BB90.unkF4 = 2;
            D_001D5F70_1BB90.unkF0 = D_001D5F70_1BB90.current;
            D_001D5F70_1BB90.close_request = 3;
            D_001D5F70_1BB90.unkE4 = menu->data.list.items[menu->data.list.selected].param.value;
            func_0022ED80_1BB90(0, 0x11, menu->moby);
            return 0;
        case 8:
            D_001D5F70_1BB90.unkF4 = 2;
            D_001D5F70_1BB90.unkF0 = D_001D5F70_1BB90.current;
            D_001D5F70_1BB90.close_request = 4;
            D_001D5F70_1BB90.unkE4 = menu->data.list.items[menu->data.list.selected].param.value;
            func_0022ED80_1BB90(0, 0x11, menu->moby);
            return 0;
        case 10:
            D_001D5F70_1BB90.unkF4 = 2;
            D_001D5F70_1BB90.unkF0 = D_001D5F70_1BB90.current;
            D_001D5F70_1BB90.close_request = 6;
            D_001D5F70_1BB90.unkE4 = menu->data.list.items[menu->data.list.selected].param.value;
            func_0022ED80_1BB90(0, 0x11, menu->moby);
            return 0;
        case 11:
            D_001D5F70_1BB90.unkF0 = D_001D5F70_1BB90.current;
            D_001D5F70_1BB90.unkF4 = 2;
            D_001D5F70_1BB90.close_request = 7;
            func_0022ED80_1BB90(0, 0x11, menu->moby);
            return 0;
        case 9:
            D_0015EE88_1BB90 = menu->data.list.items[menu->data.list.selected].param.value;
            return 0;
        case 2:
            func_0022ED80_1BB90(2, 0x11, menu->moby);
            break;
        }
    }
    /* Count first, then read selected/flags: in the other order (or as a
       do-while) the allocator shifts every register of the tail. */
    for (entry_count = 0; menu->data.list.items[entry_count].text != 0; entry_count++) {
    }
    previous_selection = menu->data.list.selected;
    flags = menu->data.list.flags;
    if (flags & 1) {
        buttons = D_0013CA40_1BB90.raw_pressed;
    } else {
        buttons = D_0013CA40_1BB90.pressed_unmasked;
    }
    if ((buttons & 0x1000) || ((flags & 0x100) && (buttons & 4))) {
        if (menu->data.list.selected != 0) {
            menu->data.list.selected--;
        } else if (flags & 0x1000) {
            menu->data.list.selected = entry_count - 1;
        } else {
            D_001D5F70_1BB90.current->pending_focus = menu->data.list.up;
        }
    }
    if ((buttons & 0x4000) || ((menu->data.list.flags & 0x100) && (buttons & 8))) {
        if (menu->data.list.items[menu->data.list.selected + 1].text != 0 &&
            menu->data.list.items[menu->data.list.selected + 1].action != 0) {
            menu->data.list.selected++;
        } else if (menu->data.list.flags & 0x1000) {
            menu->data.list.selected = 0;
        } else {
            D_001D5F70_1BB90.current->pending_focus = menu->data.list.down;
        }
    }
    if (menu->data.list.selected != previous_selection || D_001D5F70_1BB90.current->pending_focus != 0) {
        func_0022ED80_1BB90(1, 0x11, menu->moby);
        if (menu->data.list.flags & 0x20) {
            D_001A0414_1BB90[0] = D_001602E0_1BB90[menu->data.list.selected];
        }
    }
    return 0;
}
