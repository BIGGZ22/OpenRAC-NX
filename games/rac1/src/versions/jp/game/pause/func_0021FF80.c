int DrawGBsShipMenu__maybe(char *arg0) {
    char buf[0x100];
    char *t = gCamera;
    char *o = *(char **)(arg0 + 0x44);
    int count;

    *(float *)(o + 0x10) = *(float *)(t + 0x140) + 8.0f;
    *(float *)(o + 0x14) = *(float *)(t + 0x144) + D_001603A4_f;
    *(float *)(o + 0x18) = *(float *)(t + 0x148) - 0.1f;
    if (*(int *)(arg0 + 0x44) != 0) {
        DrawMobyList(*(int *)(arg0 + 0x44), 1);
    }
    SetupGifPaging(0);
    count = 0;
    {
        int k;
        for (k = 0; k < 4; k++) {
            if (D_0014BFC0[level_map_selection[0x89] * 4 + k] != 0) {
                count = count + 1;
            }
        }
    }
    func_00116248(buf, D_001603A8, func_001FE540_id(0x4F50), count,
                  func_001FE540_id(0x4F54), D_001E0B88[level_map_selection[0x89]]);
    func_001F6CF8_c(*(int *)(arg0 + 0x20) - 0x10, (D_0013E600[1] >> 1) - 8,
                    0x80000000L, buf, -1);
    func_001F6CF8_c(*(int *)(arg0 + 0x20) - 0x11, (D_0013E600[1] >> 1) - 9,
                    0x80FFA888L, buf, -1);
    DoGifPaging();
    if (0) {
        /* no code: registers the 2-byte view last (see above) */
        (void)D_001603A4_s;
    }
    return 8;
}
