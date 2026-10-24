int draw_selected_item_ammo(char *arg0) {
    char text[0x50];
    short box[10];
    char *p = *(char **)(D_001D5F74 + 0x40);
    int id = *(short *)(*(int *)(p + 0x3C) * 10 + *(char **)(p + 0x48) + 6);
    int have, total;
    char *rec;

    if (((unsigned char *)&D_0013D5C8)[id] == 0) {
        return 0;
    }
    have = weapon_ammo_counts[id];
    rec = vendor_item_prices + id * 0x18;
    total = *(unsigned short *)(rec + 0xE);
    func_00234C98_l(0x42, 0x44);
    func_00234C98_l(0x47, 0xB);
    if (*(unsigned short *)(rec + 8) == 0) {
        func_00116248(text, func_001FE540_id(0x4F53));
    } else {
        char *q;
        if (have < 1000) {
            q = text + func_00116248(text, D_001603B8, have);
        } else {
            q = text + func_00116248(text, D_001603C0, have / 1000, have % 1000);
        }
        if (total < 1000) {
            func_00116248(q, D_001603A0, total);
        } else {
            func_00116248(q, D_001603D0, total / 1000, total % 1000);
        }
    }
    SetupGifPaging(0);
    pack_image_descriptor(box, arg0);
    box[8] = 0x10;
    box[9] = 3;
    font_print_window_regular(box, 0x80FFA888L, text, -1);
    DoGifPaging();
    return 2;
}
