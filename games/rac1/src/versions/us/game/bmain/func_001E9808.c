void start_level(int level) {
    int a;
    int b;
    int t;

    if (level < 0) {
        return;
    }
    if (D_0015EE80 != 0) {
        b = D_00137C80_g.alt[level].b;
        a = D_00137C80_g.alt[level].a;
    } else {
        b = D_00137C80_g.normal[level].b;
        a = D_00137C80_g.normal[level].a;
    }
    D_0015EFD8 = 2;
    D_0013E650[0x6B] |= 8;
    func_00118D80(0);
    sound_StopAllSounds();
    music_Stop();
    FadeToBlack(func_001F98C0(12));
    D_0015F6E8 = 1;
    func_00118D80(0);
    sound_StopAllSounds();
    music_Stop();
    func_0012EF48(0);
    for (;;) {
        char *ld = memory_card_state;
        if (*(int *)(ld + 0xD4) < 3 && *(int *)(ld + 0xDC) < 0) {
            break;
        }
        memcard_update_state();
    }
    {
        char *g = level_resource_buffers;
        t = *(int *)(g + 0x1C);
    }
    Movie_Play(b, a, t + 0x100000, t + 0x400000, 0);
    func_00120F30(0);
    func_00122598(0);
    func_00120858(0, 0);
    func_00123168(func_0012F308);
    Hud_sendTexture(0x1000000, D_0015EF88, 0x1B, 6, 6, 1);
    D_0015EFD8 = 0;
    FadeToBlack(4);
    D_0015F6E8 = 0;
    D_0013E650[0x6B] |= 0x10;
}
