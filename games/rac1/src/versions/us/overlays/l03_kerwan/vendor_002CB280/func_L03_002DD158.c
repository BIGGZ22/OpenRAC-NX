void robot_qwark_update(unsigned char *m) {
    Vars_DD158 *d = *(Vars_DD158 **)(m + 0x78);
    Hit_DD158 *hit;
    unsigned char type;
    char *w;
    float vel[4] __attribute__((aligned(16)));
    float pos[4] __attribute__((aligned(16)));
    float tmp[4] __attribute__((aligned(16)));

    if (m[0x31] != 0 && FastVecDist(m + 0x10, D_L03_00166F40) < 26.0f) {
        func_L00_0025B178(m);
        m[0x7F] = 21;
    }
    if (SAVEFLAGS->collected[(short)*(unsigned short *)(m + 0xB2)] != 0 ||
        (level_moby_killed[D_0015EE84][(short)*(unsigned short *)(m + 0xB2) >> 5] >> (*(unsigned short *)(m + 0xB2) & 0x1F)) & 1) {
        if (m[0x20] != 3) {
            m[0x20] = 3;
            MobySetAnimation(m, 1, 0);
        }
    }
    hit = (Hit_DD158 *)MobyGetHitMessage(m, -1, 0);
    if (hit != 0 && hit->source != 0 && (type = hit->type) == 3 &&
        SAVEFLAGS->collected[(short)*(unsigned short *)(m + 0xB2)] == 0 &&
        (((level_moby_killed[D_0015EE84][(short)*(unsigned short *)(m + 0xB2) >> 5] >> (*(unsigned short *)(m + 0xB2) & 0x1F)) ^ 1) & 1)) {
        unsigned char *p5 = menu_item_enabled_115 + 1;
        if (p5[5] == 0) {
            p5[5] = 1;
            PlayBankSound(1, 0, 0);
            ShowBanner(0x53D6, -1);
        }
        *(u128_DD158 *)pos = *(u128_DD158 *)(m + 0x10);
        pos[2] += 1.0f;
        *(u128_DD158 *)vel = 0;
        func_L00_0025F4A8(m, vel, pos, 0.0f, 0.0f, 10, 3, 16, 4.0f, 2.0f, 9.0f, 1.0f, 0, 15.0f, 1, 1, -1, 0);
        m[0x20] = type;
        MobySetAnimation(m, 1, 0);
        level_moby_killed[D_0015EE84][(short)*(unsigned short *)(m + 0xB2) >> 5] |= 1 << (*(unsigned short *)(m + 0xB2) & 0x1F);
        D_L03_001BA8E0[(short)*(unsigned short *)(m + 0xB2) >> 5] |= 1 << (*(unsigned short *)(m + 0xB2) & 0x1F);
    }
    m[0xA4] = 0xFF;
    if (gHaveHeliPack[0xA] != 0 && m[0x20] != 3) {
        return;
    }
    switch (m[0x20]) {
    case 0:
        *(unsigned long *)(m + 0x38) = *(unsigned long *)(D_001414D0[0] + 0x38);
        m[0x20] = 1;
        DialogRequest_Init(m, (unsigned char *)d);
        if (menu_item_enabled_9[4] != 0) {
            d->mode = 1;
            d->unk36 = 2;
        }
        break;
    case 1:
        if (Npc_TryStartDialog(m, (unsigned char *)d)) {
            menu_item_enabled_9[4] = 1;
            PlaceMarkerAheadOfMoby(2.7f, m);
            m[0x20] = 2;
        }
        break;
    case 2:
        if (D_L03_0015F6A8 == 2) {
            break;
        }
        m[0x20] = 1;
        if (d->mode == 2 && d->warp != -1) {
            w = WARPTAB + d->warp * 128;
            jet_reset_camera((unsigned char *)(w + 0x30), (unsigned char *)(w + 0x70));
        }
        break;
    case 3:
        vel[0] = FastCos(random_angle_radians()) * (random_float_between(0.0f, 0.25f) * D_0015EE6C);
        vel[1] = FastSin(random_angle_radians()) * (random_float_between(0.0f, 0.25f) * D_0015EE6C);
        vel[2] = 0.0f;
        pos[0] = FastCos(random_angle_radians()) * random_float_between(0.0f, 0.5f) * random_float_between(0.0f, 1.0f);
        pos[1] = FastSin(random_angle_radians()) * random_float_between(0.0f, 0.5f) * random_float_between(0.0f, 1.0f);
        pos[2] = 0.0f;
        func_L00_001FF240(tmp, pos, m + 0x10);
        if (random_integer_below(3) == 0) {
            PartType22Spawn(pos, vel, 0x20202020, 0x7F7F7F, 157500.0f, scale_ticks(rand_range(0x3C, 0x78)));
        }
        break;
    }
}
