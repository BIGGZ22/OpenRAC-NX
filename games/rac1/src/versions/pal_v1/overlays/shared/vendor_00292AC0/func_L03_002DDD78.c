extern char D_E1_L03_001DC0A0[];
void update_moby_draw_callback(unsigned char *moby) {
    switch (moby[0x20]) {
    case 0:
        ScaleTableIntoBuffer(D_E1_L03_001DC0A0, 0.6666667f, 0x20);
        moby[0x20] = 1;
        moby[0x30] = 0xFF;
        break;
    case 1:
        AddDrawCallback(register_audio_stream_callback, moby);
        break;
    }
}
