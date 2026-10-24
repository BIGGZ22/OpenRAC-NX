void render_vendor_item_details_pass(void) {
    char buf[0x100];
    int w;

    draw_framebuffer_rect(0, 0, 0x200, 0x80, 0x200, 0x80, 0);
    DrawMobyList(D_001E66C0_m.f20 + 0x100, 1);
    DrawMobyList(D_001E66C0_m.f20, 1);
    if (D_001E66C0_m.slots[D_001E66C0_m.idx].kind == 1) {
        func_001F6968_c(6, 8, 0x80F0F0F0L, func_001FE540_id(D_001864D0_t[D_001E66C0_m.slots[D_001E66C0_m.idx].id].text), -1);
        func_001F6968_c(0x18, 0x18, 0x80F0F0F0L, func_001FE540_id(0x4F5E), -1);
        format_scaled_display_value(buf, D_001E02B0_p[D_001E66C0_m.slots[D_001E66C0_m.idx].id].c);
        func_001F6D88_c(0x76, 0x65, 0x80F0F0F0L, buf, -1);
        if (D_001E66C0_m.f40 != 0) {
            w = measure_text_width_regular((unsigned char *)buf, -1);
            func_00201640(0x75 - w, 0x6D, 0x7B, 0x70, 0x20959544L, 0);
            func_00201640(0x76 - w, 0x6D, 0x7A, 0x70, 0x30959544L, 0);
            func_00201640(0x77 - w, 0x6D, 0x79, 0x70, 0x40959544L, 0);
            func_00201640(0x78 - w, 0x6D, 0x78, 0x70, 0x50959544L, 0);
            func_00201640(0x79 - w, 0x6D, 0x77, 0x70, 0x60959544L, 0);
            func_00201640(0x7A - w, 0x6D, 0x76, 0x70, 0x70959544L, 0);
            func_00201640(0x7B - w, 0x6D, 0x75, 0x70, 0x80959544L, 0);
            format_scaled_display_value(buf, D_001E66C0_m.f40 ? D_001E02B0_p[D_001E66C0_m.slots[D_001E66C0_m.idx].id].d
                                                 : D_001E02B0_p[D_001E66C0_m.slots[D_001E66C0_m.idx].id].c);
            func_001F6D88_c(0x76, 0x55, 0x80F0F0F0L, buf, -1);
        }
    } else {
        func_001F6968_c(6, 8, 0x80F0F0F0L, func_001FE540_id(D_001864D0_t[D_001E66C0_m.slots[D_001E66C0_m.idx].id].text), -1);
        format_scaled_display_value(buf, D_001E02B0_p[D_001E66C0_m.slots[D_001E66C0_m.idx].id].a);
        func_001F6D88_c(0x76, 0x65, D_0013D5C8_b[0x23] ? 0x80808080L : 0x80F0F0F0L, buf, -1);
        if (D_0013D5C8_b[0x23] != 0) {
            w = measure_text_width_regular((unsigned char *)buf, -1);
            func_00201640(0x75 - w, 0x6D, 0x7B, 0x70, 0x20959544L, 0);
            func_00201640(0x76 - w, 0x6D, 0x7A, 0x70, 0x30959544L, 0);
            func_00201640(0x77 - w, 0x6D, 0x79, 0x70, 0x40959544L, 0);
            func_00201640(0x78 - w, 0x6D, 0x78, 0x70, 0x50959544L, 0);
            func_00201640(0x79 - w, 0x6D, 0x77, 0x70, 0x60959544L, 0);
            func_00201640(0x7A - w, 0x6D, 0x76, 0x70, 0x70959544L, 0);
            func_00201640(0x7B - w, 0x6D, 0x75, 0x70, 0x80959544L, 0);
            format_scaled_display_value(buf, D_0013D5C8_b[0x23] ? D_001E02B0_p[D_001E66C0_m.slots[D_001E66C0_m.idx].id].b
                                                 : D_001E02B0_p[D_001E66C0_m.slots[D_001E66C0_m.idx].id].a);
            func_001F6D88_c(0x76, 0x55, 0x80F0F0F0L, buf, -1);
        }
    }
}
