extern char D_J_L07_00208C20[];
void ocean_umbris_update(unsigned char *moby) {
    switch (moby[0x20]) {
    case 0:
        prepare_scaled_table_with_byte_128(D_J_L07_00208C20, 0.66666669f);
        moby[0x20] = 1;
        moby[0x30] = 0xFF;
        break;
    case 1:
        if (!is_point_inside_clip_volume(debris_far_point, **(int **)(moby + 0x78))) {
            AddDrawCallback((void *)register_audio_stream_callback, moby);
        }
        break;
    }
}
