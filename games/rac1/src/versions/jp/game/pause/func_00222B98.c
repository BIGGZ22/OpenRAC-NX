int draw_prompt_box(char *arg0) {
    char *text;
    int v;

    func_00234C98_l(0x42, 0x44);
    func_00234C98_l(0x47, 0x2004B);
    SetupGifPaging(0);
    text = D_001603E8;
    v = D_001D6044;
    switch (v) {
    case 0:
    case 1:
    case 2:
        text = func_001FE540_id(0x4FB4);
        break;
    case 3:
        text = func_001FE540_id(0x4FB6);
        break;
    }
    {
        TextBox c = { { 1, *(int *)(arg0 + 0x24) + 1, 1, *(int *)(arg0 + 0x20) + 1,
                        *(int *)(arg0 + 0x20) >> 1, 5, 0, 0, 0x10, 5 } };

        func_001F75D0(&c, 0x80000000L, text, -1);
        c.s[5] = (*(int *)(arg0 + 0x24) - c.s[7]) >> 1;
        c.s[9] ^= 4;
        func_001F75D0(&c, 0x80000000L, text, -1);
        c.s[0]--;
        c.s[1]--;
        c.s[2]--;
        c.s[3]--;
        c.s[4]--;
        c.s[5]--;
        func_001F75D0(&c, 0x80FFA888L, text, -1);
    }
    DoGifPaging();
    return 2;
}
