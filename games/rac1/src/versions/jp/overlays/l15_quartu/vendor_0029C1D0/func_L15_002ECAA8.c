void draw_quartu_cutscene_messages(void) {
    char buf[0x50];
    int flag = 0;
    int val = 0;
    char *s = D_L15_0016CEE0;
    int n;
    char *s2;
    char *s3;
    if (*(int *)(s + 0x30) == 3) {
        if (scale_ticks(0xB5) < *(int *)(s + 0x34)) {
            if (*(int *)(s + 0x34) < scale_ticks(0x104)) {
                flag = 1;
                func_00116248(buf, D_L15_00162210, msg_string(0x52C1));
                val = (*(int *)(s + 0x34) - scale_ticks(0xB5)) >> 2;
            }
        }
        s2 = D_L15_0016CEE0;
        if (scale_ticks(0x1C2) < *(int *)(s2 + 0x34)) {
            if (*(int *)(s2 + 0x34) < scale_ticks(0x280)) {
                flag = 1;
                func_00116248(buf, D_L15_00162210, msg_string(0x52C2));
                val = (*(int *)(s2 + 0x34) - scale_ticks(0x1C2)) >> 2;
            }
        }
        s3 = D_L15_0016CEE0;
        if (scale_ticks(0x2EE) < *(int *)(s3 + 0x34)) {
            if (*(int *)(s3 + 0x34) < scale_ticks(0x3A2)) {
                flag = 1;
                func_00116248(buf, D_L15_00162210, msg_string(0x52C3));
                val = (*(int *)(s3 + 0x34) - scale_ticks(0x2EE)) >> 2;
            }
        }
    } else if (*(int *)(s + 0x30) == 4) {
        if (scale_ticks(0x8FC) < *(int *)(s + 0x34)) {
            if (*(int *)(s + 0x34) < scale_ticks(0x960)) {
                flag = 1;
                func_00116248(buf, D_L15_00162210, msg_string(0x52C5));
                val = (*(int *)(s + 0x34) - scale_ticks(0x8FC)) >> 2;
            }
        }
    }
    if (flag == 0) return;
    n = func_00116810(buf);
    if (val < n) {
        n = val;
    } else if (n < val) {
        if ((n - val) & 4) n = n - 1;
    }
    print_text_with_texture_3((void *)(*(int *)&D_L15_00162200 + 1), (void *)(*(int *)&D_L15_00162204 + 1), (void *)*(int *)&D_L15_0016220C, buf, (void *)n);
    print_text_with_texture_3((void *)*(int *)&D_L15_00162200, (void *)*(int *)&D_L15_00162204, (void *)*(int *)&D_L15_00162208, buf, (void *)n);
}
