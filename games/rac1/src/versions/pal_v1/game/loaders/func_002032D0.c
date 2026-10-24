void LoadHudBanks(void) {
    SrcHdr *base;
    int *out;
    int *p;
    int n;
    int v;
    int size24;
    HudHdr *header;
    int bank2;
    unsigned int shift54;
    int size58;
    char *bank58;
    unsigned int shift5C;
    unsigned int shift60;
    unsigned int shift64;
    char *fname;
    int *d941c0;

    base = D_0015EF4C_src;
    out = D_0019A520;
    p = &base->bank[1].size;
    for (n = 0; n < 5; n++) {
        *out++ = ALIGN64(*p);
        p += 2;
    }

    fname = D_0015FC70;
    d941c0 = level_resource_buffers;
    size24 = ALIGN64(base->bank[0].size);
    header = (HudHdr *) func_001FFAB8_d(size24, 0, fname, 0x23A);
    FastMemCopy(header, (void *) (base->bank[0].off + (int) base), size24);

    D_0019A4E8_arena.header = header;
    D_0019A4E8_arena.unk1C = (char *) header + header->unk04;
    bank2 = d941c0[2] + 0x60000;
    D_0019A4E8_arena.unk20 = (char *) header + header->unk08;
    D_0019A4E8_arena.unk28 = (char *) header + header->unk0C;
    D_0019A4E8_arena.unk24 = (char *) header + header->unk10;

    if (header->unk54 != 0) {
        shift54 = (unsigned int) ALIGN64(base->bank[1].size) >> 4;
        LoadCompressedHudBank(0, bank2);
        D_0019A4E8_arena.header->unk94 = Stash_SendData(
            base->bank[1].off + (int) base, shift54, shift54, (int) D_0015FC80);
        Hud_SendResidentBank(0, (void *) bank2, 1);
    }

    size58 = D_0019A4E8_arena.header->unk58;
    if (size58 != 0) {
        bank58 = func_001FFAB8_d(size58, 0, fname, 0x261);
        LoadCompressedHudBank(1, (int) bank58);
        func_00118D80(0);
        LinkHudBank(1, (int) bank58);
    }

    if (D_0019A4E8_arena.header->unk5C != 0) {
        shift5C = (unsigned int) ALIGN64(base->bank[3].size) >> 4;
        D_0019A4E8_arena.header->unk9C = Stash_SendData(
            base->bank[3].off + (int) base, shift5C, shift5C, (int) D_0015FC90);
    }

    if (D_0019A4E8_arena.header->unk60 != 0) {
        shift60 = (unsigned int) ALIGN64(base->bank[4].size) >> 4;
        D_0019A4E8_arena.header->unkA0 = Stash_SendData(
            base->bank[4].off + (int) base, shift60, shift60, (int) D_0015FCA0);
    }

    if (D_0019A4E8_arena.header->unk64 != 0) {
        shift64 = (unsigned int) ALIGN64(base->bank[5].size) >> 4;
        D_0019A4E8_arena.header->unkA4 = Stash_SendData(
            base->bank[5].off + (int) base, shift64, shift64, (int) D_0015FCB0);
    }
}
