s32 sound_options_menu(struct SoundMenu_1DB30 *menu) {
    s32 previous_selection;
    s32 *sound_volume_ptr;
    s32 previous_music_volume;
    s32 previous_sound_volume;
    s32 scaled_sound_volume;

    if ((D_0013CA40_1DB30.pressed_buttons & 0xD00) && (D_001D6094_1DB30 == 0)) {
        return 1;
    }
    if (D_0013CA40_1DB30.pressed_buttons & 0x10) {
        struct MenuPage_1DB30 *back_page = D_001D5F70_1DB30.current->back;

        if (back_page != 0) {
            D_001D5F70_1DB30.next = back_page;
            return 0;
        } else if (D_001D5F70_1DB30.close_locked == 0) {
            return -1;
        }
    }
    previous_selection = menu->selected_option;
    if (D_0013CA40_1DB30.pressed_buttons & 0x1000) {
        menu->selected_option = (previous_selection + 2) % 3;
    }
    if (D_0013CA40_1DB30.pressed_buttons & 0x4000) {
        menu->selected_option = (menu->selected_option + 1) % 3;
    }
    if ((menu->selected_option != previous_selection) || (D_001D5F70_1DB30.current->pending_focus != 0)) {
        func_0022ED80_1DB30(1, 0x11, menu->sound_owner);
        if (menu->flags & 0x20) {
            D_001A0414_1DB30 = D_001602E0_1CE60[menu->selected_option];
        }
    }
    previous_music_volume = D_0015EEEC_1DB30;
    previous_sound_volume = D_0015EEF0_1DB30;
    sound_volume_ptr = &D_0015EEF0_1DB30;
    if (D_0013CA40_1DB30.held_buttons & 0x2000) {
        if (menu->selected_option == 0) {
            D_0015EEF0_1DB30 = (previous_sound_volume + 3 < 0x401) ? previous_sound_volume + 3 : 0x400;
        }
        if (menu->selected_option == 1) {
            D_0015EEEC_1DB30 = (previous_music_volume + 3 < 0x401) ? previous_music_volume + 3 : 0x400;
        }
    }
    if (D_0013CA40_1DB30.held_buttons & 0x8000) {
        if (menu->selected_option == 0) {
            D_0015EEF0_1DB30 = (D_0015EEF0_1DB30 - 3 <= 0) ? 0 : D_0015EEF0_1DB30 - 3;
        }
        if (menu->selected_option == 1) {
            D_0015EEEC_1DB30 = (D_0015EEEC_1DB30 - 3 <= 0) ? 0 : D_0015EEEC_1DB30 - 3;
        }
    }
    if ((previous_music_volume != D_0015EEEC_1DB30) || (previous_sound_volume != *sound_volume_ptr)) {
        D_0013E650_1DB30.group_0_volume = *sound_volume_ptr * 8 / 10;
        D_0013E650_1DB30.group_2_volume = D_0013E650_1DB30.group_1_volume = D_0015EEEC_1DB30;
        D_0013E650_1DB30.group_3_volume = *sound_volume_ptr * 7 / 10;
        scaled_sound_volume = *sound_volume_ptr * 7 / 10;
        D_0013E650_1DB30.group_4_volume = scaled_sound_volume;
        D_0013E650_1DB30.group_5_volume = *sound_volume_ptr;
    }
    if (D_0013CA40_1DB30.pressed_buttons & 0x60) {
        if (menu->selected_option == 2) {
            D_0015EEE8_1DB30 = !D_0015EEE8_1DB30;
        }
        func_0012E380_1DB30(!D_0015EEE8_1DB30);
        func_0022ED80_1DB30(0, 0x11, menu->sound_owner);
    }
    return 0;
}
