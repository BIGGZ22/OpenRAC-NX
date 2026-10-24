void render_vendor_buy_label_pass(char *unused, int x, int y) {
    char sp[0x18];
    VendorA *v;
    int idx;
    int m;
    int price;
    int sz = 0x18;

    draw_framebuffer_rect(0, 0, 0x200, 0x80, 0x200, 0x80, 0);
    ((void *(*)(void *, int, int))func_001153FC)(sp, 0, 0x18);
    *(short *)(sp + 0x2) = y;
    *(short *)(sp + 0x6) = x;
    *(short *)(sp + 0x8) = x >> 1;
    *(short *)(sp + 0xA) = (y >> 1) - 7;
    *(short *)(sp + 0x10) = 0x10;
    *(short *)(sp + 0x12) = 1;
    if (((VendorA *)vendor_state)->sel != 0) {
        if (((VendorA *)vendor_state)->slots[((VendorA *)vendor_state)->idx].kind == 1 &&
            weapon_ammo_counts[((VendorA *)vendor_state)->slots[((VendorA *)vendor_state)->idx].type] >=
                ((WeaponInfoA *)(vendor_item_prices + ((VendorA *)vendor_state)->slots[((VendorA *)vendor_state)->idx].type * sz))->max) {
            m = 0x528A;
        } else {
            v = (VendorA *)vendor_state;
            if (v->slots[v->idx].kind == 1) {
                if (v->alt) {
                    price = ((WeaponInfoA *)(vendor_item_prices + v->slots[v->idx].type * 0x18))->priceAlt;
                } else {
                    price = ((WeaponInfoA *)(vendor_item_prices + v->slots[v->idx].type * 0x18))->price;
                }
                m = 0x528A;
                if (gBolts >= price)
                    m = 0x528B;
            } else {
                int cost;
                if (discount_purchase_pricing[0] != 0) {
                    cost = ((struct { WeaponInfoA a[1]; } *)vendor_item_prices)->a[v->slots[v->idx].type].second;
                } else {
                    cost = ((WeaponInfoA *)(vendor_item_prices + v->slots[v->idx].type * 0x18))->first;
                }
                m = gBolts < cost ? 0x528A : 0;
            }
        }
    } else {
        m = 0x528B;
    }
    if (m != 0) {
        *(unsigned short *)(sp + 0x12) |= 4;
        func_001F75D0(sp, 0x80F0F0F0L, func_001FE540_id(m), -1);
        *(unsigned short *)(sp + 0x12) ^= 4;
        *(short *)(sp + 0xA) = (y - *(short *)(sp + 0xE)) >> 1;
        func_001F75D0(sp, 0x80F0F0F0L, func_001FE540_id(m), -1);
    }
}
