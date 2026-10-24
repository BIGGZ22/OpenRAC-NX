extern char D_L07_00208820[];
void AmbientStreamL07Update(unsigned char *moby)
{
    switch (moby[0x20]) {
    case 0:
        moby[0x20] = 1;
        moby[0x30] = 0xFF;
        ScaleTableIntoBuffer(D_L07_00208820, 0x80, 1.0f);
        break;
    case 1:
        AddDrawCallback(register_audio_stream_callback, moby);
        break;
    }
}
