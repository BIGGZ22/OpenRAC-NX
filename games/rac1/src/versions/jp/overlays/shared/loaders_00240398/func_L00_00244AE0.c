extern s32 D_J_L00_001AFE00[];
s32 func_L00_00244AE0(s32 full, s32 keep) {
    Load244110 *ld = ((Load244110 *)&D_L00_00173F00);
    Hdr244 *h = (Hdr244 *)ld->f4;
    s32 *src = D_0015EF48.shared;
    char *base;
    s32 used;
    char *e;
    char *free;
    char *end;
    Span244110 *span;
    TexRef244110 *tex;
    TexRef244110 *t1;
    TexRef244110 *t3;
    TexRef244110 *t4;
    char *tbl;
    s32 i;
    s32 j;
    s32 n;
    s32 k;
    s32 c1, c2, c3, c4;
    s32 m;
    char *p;
    s32 sq;
    char *sp;
    char *q;
    s32 z;
    s32 ci;
    char *x;
    char *y;
    s32 a1, a2, a3;
    char *q1, *q2;
    u64 *t;

    if (full)
        LoadHudBanks();
    base = ld->f14;
    if (full) {
        FastMemCopy(h, (char *)(src[4] + (s32)src), src[5]);
        D_L00_0015F50C = 0;
        func_L00_001FF040(D_L00_0015FD48, D_0014C150 + D_0015EE84 * 16, 0x10);
        func_00203958((char *)(src[6] + (s32)src), h->v[0], (char *)h + h->v[1]);
        D_L00_001600C4 = D_0015EE5C;
        func_00118D80(0);
        used = func_0020C468((char *)(src[0x14] + (s32)src), base);

        D_L00_0015F50C = (s32)(base + h->v[0x18]);
        D_L00_001828C0[0] = (((u64)((D_0015EF8C + h->v[0x24]) >> 8) | ((u64)(2) << 14) | ((u64)(0x13) << 20) | ((u64)(7) << 26) | ((u64)(7) << 30) | ((u64)(1) << 34) | ((u64)(0) << 35) | ((u64)((D_0015EF8C + h->v[0x25]) >> 8) << 37) | ((u64)(0) << 51) | ((u64)(0) << 55) | ((u64)(0) << 56) | ((u64)(4) << 61)));
        D_L00_001828C0[1] = 0x0000FFA0000000E0;
        D_L00_001828C0[2] = 0x0040000400004000;
        D_L00_001828D8[0] = (((u64)((D_0015EF8C + h->v[0x26]) >> 8) | ((u64)(1) << 14) | ((u64)(0x13) << 20) | ((u64)(6) << 26) | ((u64)(6) << 30) | ((u64)(1) << 34) | ((u64)(0) << 35) | ((u64)((D_0015EF8C + h->v[0x27]) >> 8) << 37) | ((u64)(0) << 51) | ((u64)(0) << 55) | ((u64)(0) << 56) | ((u64)(4) << 61)));
        D_L00_001828D8[1] = 0x0000FFA0000000E0;
        D_L00_001828D8[2] = 0x0040000400004000;
        D_L00_001828F0[0] = (((u64)((D_0015EF8C + h->v[0x30]) >> 8) | ((u64)(2) << 14) | ((u64)(0x13) << 20) | ((u64)(7) << 26) | ((u64)(7) << 30) | ((u64)(1) << 34) | ((u64)(0) << 35) | ((u64)((D_0015EF8C + h->v[0x31]) >> 8) << 37) | ((u64)(0) << 51) | ((u64)(0) << 55) | ((u64)(0) << 56) | ((u64)(4) << 61)));
        D_L00_001828F0[1] = 0x0000FFA0000000E0;
        D_L00_001828F0[2] = 0x0040000400004000;

        t1 = (TexRef244110 *)((char *)h + h->v[0xD]);
        D_L00_00161014 = h->v[0xC];
        for (c1 = 0; c1 < D_L00_00161014; c1++)
            D_L00_001C4D00[c1] = D_L00_0015F50C + t1[c1].offset + (log2dim(t1[c1].kind) << 28);
        tex = (TexRef244110 *)((char *)h + h->v[0xF]);
        D_L00_00160088 = h->v[0xE];
        for (c2 = 0; c2 < D_L00_00160088; c2++)
            D_L00_00199E00[c2] = D_L00_0015F50C + tex[c2].offset + (log2dim(tex[c2].kind) << 28);
        FastMemCopy(D_L00_001AEF40, tex, D_L00_00160088 * 16);
        t3 = (TexRef244110 *)((char *)h + h->v[0x11]);
        D_L00_00161094 = h->v[0x10];
        for (c3 = 0; c3 < D_L00_00161094; c3++)
            D_L00_001C6A00[c3] = D_L00_0015F50C + t3[c3].offset + (log2dim(t3[c3].kind) << 28);
        t4 = (TexRef244110 *)((char *)h + h->v[0x13]);
        D_L00_0016056C = h->v[0x12];
        for (c4 = 0; c4 < D_L00_0016056C; c4++)
            D_L00_001BC940[c4] = D_L00_0015F50C + t4[c4].offset + (log2dim(t4[c4].kind) << 28);

        initialize_tfrag_render_data(base + h->v[2], (char *)h + h->v[0xD]);
        if (h->v[3])
            D_L00_0015F6E0 = (s32)(base + h->v[3]);
        else
            D_L00_0015F6E0 = 0;
        if (h->v[4])
            func_00203118(base + h->v[4]);
        else
            D_L00_001605DC = 0;
        CollInit(base + h->v[5]);
        for (i = 0, q2 = (char *)h + h->v[7]; i < h->v[6]; i++, q2 += 0x20)
            func_00203E78(*(s32 *)q2 ? base + *(s32 *)q2 : 0, (char *)h + h->v[0xF], q2 + 0x10, *(s32 *)(q2 + 4));
        print_moby_class_sizes((int *)((char *)h + h->v[7]), h->v[7]);
        for (sq = 0, p = (char *)h + h->v[0x1E]; sq < D_L00_00197680[0]->seq_count; sq++, p += 4) {
            if (*(s32 *)p) {
                D_L00_00197680[0]->seqs[sq] = base + *(s32 *)p;
                relocateObjectPointers(D_L00_00197680[0], sq);
            }
        }
        D_L00_001600CC = -1;
        q = (char *)h + h->v[0x21];
        D_L00_001600D0 = 0;
        D_L00_001600C8 = h->v[0x20];
        for (m = 0; m < h->v[0x20]; m++, q += 0x10) {
            D_L00_001AFF40[m] = *(s32 *)(q + 4);
            D_L00_001AFFA0[m] = base + *(s32 *)q;
            D_J_L00_001AFE00[m] = *(s32 *)(q + 8);
            { s32 o = D_L00_00197F40[*(s32 *)(q + 4)] * 32 + 0x10;
            qcopy(&D_L00_001B0060[m], (char *)h + h->v[7] + o); }
        }
        for (i = 0, q2 = (char *)h + h->v[9]; i < h->v[8]; i++, q2 += 0x20)
            register_object_render_class(base + *(s32 *)q2, (char *)h + h->v[0x11], q2 + 0x10, *(s32 *)(q2 + 4));
        Profile_Mark((char *)h + h->v[9], h->v[8]);
        for (i = 0, q2 = (char *)h + h->v[0xB]; i < h->v[0xA]; i++, q2 += 0x30)
            register_shrub_render_class(base + *(s32 *)q2, (char *)h + h->v[0x13], q2 + 0x10, q2 + 0x20, *(s32 *)(q2 + 4));
        D_L00_0015F520 = base + h->v[0x1A];
        func_00203038((char *)h + h->v[0x17], h->v[0x16]);
        if (h->v[0x28]) {
            relocate_counted_pointer_table(base + h->v[0x28]);
        } else {
            D_L00_0015F538 = 0;
            D_L00_0015F53C = 0;
            D_L00_0015F540 = 0;
        }
        if (h->v[0x29]) {
            LoadLightHeader(base + h->v[0x29]);
        } else {
            D_L00_0015FD58 = 0;
            D_L00_0015FD5C = 0;
            D_L00_0015FD64 = 0;
            D_L00_0015FD60 = 0;
            D_L00_0015FD68 = 0;
        }
        if (h->v[0x2A])
            D_L00_0015F6E4 = base + h->v[0x2A];
        else
            D_L00_0015F6E4 = 0;
        {
            char *x1 = base + h->v[0x19];
            char *x2 = (char *)h + h->v[0x15];
            ParseParticleTexs((char *)h + h->v[0x1B], x1, x2, h->v[0x14]);
        }

        tbl = (char *)h + h->v[0x1C];
        span = (Span244110 *)tbl;
        free = (char *)((u32)(base + used + 0x3F) & ~0x3F);
        n = span->count;
        D_L00_0015F6D4 = (Slot244110 *)free;
        D_L00_0015F6D0 = n;
        func_L00_001FF040(free, tbl + span->off, n * 32);
        span++;
        free += n * 32;
        e = tbl + span->off;
        for (a3 = 0; a3 < D_L00_0015F6D0; a3++) {
            if (D_L00_0015F6D4[a3].sound >= span->count) {
                ((void (*)())STUB_printf)(D_L00_001E8B00);
                D_L00_0015F6D4[a3].sound = -1;
            } else {
                D_L00_0015F6D4[a3].sound = *(u16 *)(e + D_L00_0015F6D4[a3].sound * 4);
            }
            D_L00_0015F6D4[a3].time = D_0015EE5C;
        }
        end = free + 0x3F;
        span++;
        for (ci = 0; ci < D_L00_00160080; ci++, span++) {
            Class244110 *cls = D_L00_00197680[ci];
            if (cls == 0) {
                for (j = 0; j < D_L00_001600C8; j++)
                    if (D_L00_001AFF40[j] == D_L00_00197D80[ci])
                        break;
                if (j < D_L00_001600C8) {
                    FastMemSet(D_L00_001B01E0[j], -1, 0x20);
                    e = tbl + span->off;
                    for (k = 0; k < span->count && k < 15; k++, e += 4)
                        D_L00_001B01E0[j][k] = *(u16 *)e;
                }
            } else {
                if (span->count != cls->count0D)
                    ((void (*)())STUB_printf)(D_L00_001E8B20, D_L00_00197D80[ci]);
                e = tbl + span->off;
                for (k = 0; k < D_L00_00197680[ci]->count0D; k++, e += 4) {
                    D_L00_00197680[ci]->f28[k].sound = *(u16 *)e;
                    D_L00_00197680[ci]->f28[k].time = D_0015EE5C;
                }
            }
        }
        ((Load244110 *)&D_L00_00173F00)->f18 = load_level_texture_bank((char *)((u32)end & ~0x3F), (s32 *)h);
    }
    ((Load244110 *)&D_L00_00173F00)->f1C = load_level_gameplay(full);
    if (full) {
        char *top;
        ((void (*)())STUB_printf)(D_L00_001E8B50);
        ((void (*)())STUB_printf)(D_L00_001E8B68, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)(0x100000)) * (1.0f / 1024.0f)))));
        top = D_L00_002F2298;
        ((void (*)())STUB_printf)(D_L00_001E8B80, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)(top - 0x100000)) * (1.0f / 1024.0f)))));
        ((void (*)())STUB_printf)(D_L00_001E8B98, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)(((Load244110 *)&D_L00_00173F00)->f0 - top)) * (1.0f / 1024.0f)))));
        ((void (*)())STUB_printf)(D_L00_001E8BB0, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)(((Load244110 *)&D_L00_00173F00)->fC - ((Load244110 *)&D_L00_00173F00)->f0)) * (1.0f / 1024.0f)))));
        ((void (*)())STUB_printf)(D_L00_001E8BC8, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)(((Load244110 *)&D_L00_00173F00)->f10 - ((Load244110 *)&D_L00_00173F00)->fC)) * (1.0f / 1024.0f)))));
        ((void (*)())STUB_printf)(D_L00_001E8BE0, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)(((Load244110 *)&D_L00_00173F00)->f14 - ((Load244110 *)&D_L00_00173F00)->f10)) * (1.0f / 1024.0f)))));
        if (D_L00_0015F6E0 != 0)
            ((void (*)())STUB_printf)(D_L00_001E8BF8, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)((char *)D_L00_0015F6E0 - ((Load244110 *)&D_L00_00173F00)->f14)) * (1.0f / 1024.0f)))));
        else if (D_L00_001605DC != 0)
            ((void (*)())STUB_printf)(D_L00_001E8BF8, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)((char *)D_L00_001605DC - ((Load244110 *)&D_L00_00173F00)->f14)) * (1.0f / 1024.0f)))));
        else
            ((void (*)())STUB_printf)(D_L00_001E8C10);
        if (D_L00_0015F6E0 != 0) {
            if (D_L00_001605DC != 0)
                ((void (*)())STUB_printf)(D_L00_001E8C30, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)(D_L00_001605DC - (s32)D_L00_0015F6E0)) * (1.0f / 1024.0f)))));
            else
                ((void (*)())STUB_printf)(D_L00_001E8C30, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)(((char *)D_L00_00173F40.f0) - (s32)D_L00_0015F6E0)) * (1.0f / 1024.0f)))));
        }
        if (h->v[4])
            ((void (*)())STUB_printf)(D_L00_001E8C48, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)(((char *)D_L00_00173F40.f0) - (char *)D_L00_001605DC)) * (1.0f / 1024.0f)))));
        ((void (*)())STUB_printf)(D_L00_001E8C60, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)(D_L00_0015F50C - (s32)((char *)D_L00_00173F40.f0))) * (1.0f / 1024.0f)))));
        ((void (*)())STUB_printf)(D_L00_001E8C78, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)((D_L00_001B1E00[1] >> 4) - D_L00_0015F50C)) * (1.0f / 1024.0f)))));
        ((void (*)())STUB_printf)(D_L00_001E8C90, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)((s32)D_L00_0015F520 - (D_L00_001B1E00[1] >> 4))) * (1.0f / 1024.0f)))));
        ((void (*)())STUB_printf)(D_L00_001E8CA8, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)((char *)D_L00_00197680[0] - D_L00_0015F520)) * (1.0f / 1024.0f)))));
        ((void (*)())STUB_printf)(D_L00_001E8CC0, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)(((char *)D_L00_001C5B00[0]) - (char *)D_L00_00197680[0])) * (1.0f / 1024.0f)))));
        if (h->v[0xA])
            ((void (*)())STUB_printf)(D_L00_001E8CD8, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)(((char *)D_L00_001BC3C0[0]) - ((char *)D_L00_001C5B00[0]))) * (1.0f / 1024.0f)))));
        else
            ((void (*)())STUB_printf)(D_L00_001E8CD8, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)(((char *)D_L00_00161080) - ((char *)D_L00_001C5B00[0]))) * (1.0f / 1024.0f)))));
        if (h->v[0xA])
            ((void (*)())STUB_printf)(D_L00_001E8CF0, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)(D_L00_00197680[0]->seqs[0] - ((char *)D_L00_001BC3C0[0]))) * (1.0f / 1024.0f)))));
        ((void (*)())STUB_printf)(D_L00_001E8D08, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)(((char *)D_L00_00161080) - D_L00_00197680[0]->seqs[0])) * (1.0f / 1024.0f)))));
        ((void (*)())STUB_printf)(D_L00_001E8D20, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)(((char *)D_L00_00160554) - ((char *)D_L00_00161080))) * (1.0f / 1024.0f)))));
        ((void (*)())STUB_printf)(D_L00_001E8D38, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)(((char *)D_L00_00160098) - ((char *)D_L00_00160554))) * (1.0f / 1024.0f)))));
        ((void (*)())STUB_printf)(D_L00_001E8D50, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)(D_L00_001600A8 - ((char *)D_L00_00160098))) * (1.0f / 1024.0f)))));
        ((void (*)())STUB_printf)(D_L00_001E8D68, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)(D_L00_001601C8 - D_L00_001600A8)) * (1.0f / 1024.0f)))));
        ((void (*)())STUB_printf)(D_L00_001E8D80, (((*(f64 (*)(f32))func_00120778)(IntToFloat((s32)(D_L00_0016022C - D_L00_001601C8)) * (1.0f / 1024.0f)))));
        ((void (*)())STUB_printf)(D_L00_001E8D98, 128.0);
        ((void (*)())STUB_printf)(D_L00_0015FD40);
        ((void (*)())STUB_printf)(D_L00_001E8DB0, (*(f64 (*)(f32))func_00120778)(IntToFloat((s32)D_L00_0016022C + 0x20000) * (1.0f / 1048576.0f)));
        ((void (*)())STUB_printf)(D_L00_0015FD40);
        {
            char *lo = ((Load244110 *)&D_L00_00173F00)->f0 - 0x2D0000;
            ((void (*)())STUB_printf)(D_L00_001E8DC8, (*(f64 (*)(f32))func_00120778)(IntToFloat(((Load244110 *)&D_L00_00173F00)->f1C - lo) * (1.0f / 1048576.0f)));
        }
        ((void (*)())STUB_printf)(D_L00_0015FD40);
    }
    if (keep) {
        restore_or_clear_level_state();
    } else {
        clear_scene_state_buffers();
        for (z = 15; z >= 0; z--)
            D_0014EF90[z] = 0;
    }
    D_L00_00179200.f10 = 0;
    *(s32 *)&D_L00_00179200.f8 = 0;
    *(s32 *)&D_L00_00179200.fC = 0;
    ((void (*)(void))func_L00_002657B8)();
    D_L00_0015F6B0++;
    return 0;
}
