void update_moby_list_empty_dialogue_trigger(char *moby) {
    int *data = *(int **)(moby + 0x78);
    if (data[0] == -1) {
        DeleteMoby(moby);
    } else if (data[1] != -1 && count_group_e81e0(data[1], -1) == 0) {
        unsigned char *g = menu_item_enabled_115 + 1;
        if (g[0x13] == 0) {
            g[0x13] = 1;
            PlayBankSound(1, 0, 0);
            ShowBanner(0x542D, -1);
        }
    }
}
