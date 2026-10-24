int rebuild_configured_text_label_list(void) {
    int ids[8];
    int names[7];
    int i, n;

    *(Blk32 *)ids = D_001E8A58;
    *(Blk28 *)names = D_001E8A78;
    n = 0;
    for (i = 0; i < 12 && ids[i] != -1; i++) {
        if (selector_available[ids[i]] != 0) {
            D_001D3E90[n].val = names[i];
            D_001D3E90[n].addr = &gCheats[ids[i]];
            D_001D3E90[n].c1 = 0x4F5B;
            D_001D3E90[n].c2 = 0x4F5C;
            D_001D3E90[n].zero = 0;
            n++;
        }
    }
    D_001D3E90[n].val = 0;
    return 0;
}
