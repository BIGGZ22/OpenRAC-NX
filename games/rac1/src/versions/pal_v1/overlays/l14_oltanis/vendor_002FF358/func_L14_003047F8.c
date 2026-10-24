extern char D_E1_L14_001E9BE0[];
void update_moby_draw_registration(unsigned char *moby) {
    switch (moby[0x20]) {
    case 0:
        ScaleTableIntoBuffer(D_E1_L14_001E9BE0, 0.6666667f, 0x40);
        moby[0x20] = 1;
        moby[0x30] = 0xFF;
        break;
    case 1:
        AddDrawCallback(register_audio_stream_callback, moby);
        break;
    }
}
