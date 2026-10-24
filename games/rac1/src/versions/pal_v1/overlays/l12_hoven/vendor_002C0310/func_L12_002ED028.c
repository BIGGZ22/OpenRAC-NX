void Moby336_HandleHit(char *m, int flag) {
    char *d = *(char **)(m + 0x78);
    float a[4];
    float b[4];
    unsigned char *q;
    if (MobyGetHitMessage(m, 0xA30000, 0) != 0) {
        if (*(int *)(D_0013E633 + 0x2EA1) != 0x32) {
            *(int *)&D_0015EF04 += 1;
            if (*(int *)&D_0015EF04 >= 5) {
                q = (unsigned char *)(menu_item_enabled_115 + 1);
                if (q[0x15] == 0) {
                    q[0x15] = 1;
                    PlayBankSound(1, 0, 0);
                    ShowBanner(0x53D6, -1);
                }
            }
        }
        qcopy(a, m + 0x10);
        clear_u64_value(b);
        func_L00_00261B00(m, 0x1F, 0x4A, 2, -1);
        *(int *)(d + 0xA4) = scale_ticks(500);
        func_L00_0025F4A8(m, b, a, 0.0f, 0.0f, 0x14, 3, 4, 4.0f, 2.0f, 100000.0f, -1, 3.0f, 15.0f, 1, 1, -1, 0);
    }
    *(unsigned char *)(m + 0xA4) = 0xFF;
    if (*(int *)&D_L12_00161A34 != 0 && flag != 0) {
        *(int *)(m + 0x94) = 0;
        *(unsigned short *)(m + 0x34) = (*(unsigned short *)(m + 0x34) | 0x41) & 0xEFFF;
    }
    if (*(int *)(d + 0xA4) == 0 && (*(unsigned short *)(m + 0x34) & 1) == 0) return;
    if (func_001F9908((int *)(d + 0xA4)) != 0 && flag != 0 && *(int *)&D_L12_00161A34 == 0) {
        *(int *)(m + 0x94) = *(int *)(*(char **)(m + 0x24) + 0x10);
        *(unsigned short *)(m + 0x34) = (*(unsigned short *)(m + 0x34) & 0xFFBE) | 0x1000;
    } else {
        *(int *)(m + 0x94) = 0;
        *(unsigned short *)(m + 0x34) = (*(unsigned short *)(m + 0x34) | 0x41) & 0xEFFF;
    }
}
