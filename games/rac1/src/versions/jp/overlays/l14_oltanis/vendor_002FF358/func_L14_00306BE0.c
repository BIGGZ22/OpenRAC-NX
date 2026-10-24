void CollectiblePickupUpdate(struct Moby *moby) {
    u8 *o = (u8 *)moby;
    u8 *d = (u8 *)moby->pvars;
    f32 v[4];
    u8 state = moby->state;

    switch (state) {
    case 0:
        if (D_0014C150_06BE0[moby->mission + D_0015EE84_06BE0 * 16] == 0xFF) {
            func_0020D678_06BE0(moby);
            return;
        }
        func_L14_00306EC0_06BE0((void *)moby);
        func_00214358_06BE0(o + 0x10, 0, 0.5f);
        moby->rot.x = 0.0f;
        moby->rot.y = -1.5707964f;
        moby->pos.z = D_L14_00174668_06BE0[0] + D_L14_001622B8_06BE0;
        qcopy(d, o + 0x10);
        moby->state = 1;
        break;
    case 1:
        *(f32 *)(o + 0x48) = func_001FA748_06BE0(*(f32 *)(o + 0x48), D_L14_001622BC_06BE0);
        qcopy(o + 0x10, d);
        func_001F9C30_06BE0(v, o + 0xd0, D_L14_001622C0_06BE0);
        func_001F9BD8_06BE0(o + 0x10, o + 0x10, v);
        func_L14_00306FA0_06BE0(moby);
        if (D_L14_0015F6B0_06BE0 % 10 != 0)
            return;
        if (!(func_001F9D48_06BE0(o + 0x10, D_0013F450_06BE0.motion.pos.f) < 1.0f))
            return;
        if (!(func_001F9B88_06BE0(*(f32 *)(o + 0x18) - D_0013F450_06BE0.motion.pos.f[2]) < 2.0f))
            return;
        if (D_0013D5DD_06BE0 != 0)
            *(s32 *)(d + 0x18) = state;
        else
            *(s32 *)(d + 0x18) = 0;
        func_L00_002618D8_06BE0(0x15, 1);
        func_L00_00299B68_06BE0(6);
        moby->state = 2;
        break;
    case 2:
        if (D_L14_0015F6A8_06BE0 != state) {
            if (D_0014C150_06BE0[moby->mission + D_0015EE84_06BE0 * 16] != 0xFF) {
                func_L00_002512D8_06BE0(moby->mission);
                func_0020BFC8_06BE0(0, -1);
            }
            if (*(s32 *)(d + 0x18) != 0)
                func_L00_00264DB8_06BE0(0x5437, -1);
            else
                func_L00_00264DB8_06BE0(0x36b2, -1);
            func_L00_00203F20_06BE0(0x36b0, 0x7a);
            func_0020D678_06BE0(moby);
            return;
        }
        *(u16 *)(o + 0x34) |= 0x41;
        break;
    case 3:
        func_00214D28_06BE0((f32 *)(o + 0x2c), 0.0f, *(f32 *)(*(u8 **)(o + 0x24) + 0x24) * 0.02f);
        if (*(f32 *)(o + 0x2c) == 0.0f)
            func_0020D678_06BE0(moby);
        break;
    }
}
