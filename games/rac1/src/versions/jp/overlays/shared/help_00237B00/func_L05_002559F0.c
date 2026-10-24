int Hero_PickHelpMode(void)
{
    H_9F0 *g;
    char *h;
    unsigned char *pp;
    unsigned char *m;
    float *vp;
    float (*pts)[4];
    float v1[4];
    float v2[4];
    float v3[4];
    float f0;
    float f1;
    float f2;
    float f20;
    int r16;
    int r4;
    int ok;
    int ee;
    unsigned char c;

    g = ((H_9F0 *)(D_0013E633 + 0xE1D));
    if (!(D_0015EE6C * 12.0f < g->f164)) goto L_BAC;
    f20 = 0.0f;
    LocalToWorldPoint(v1, f20, f20, 0.75f);
    vp = v2;
    qcopy(vp, v1);
    qcopy(v3, g->fE0);
    func_L00_001FF4B0(v3, v3, 0.55f);
    func_001F9BD8_4C558(v3, v3, g->fE0);
    v3[2] = f20;
    func_001F9BD8_4C558(vp, v1, v3);
    if (!func_L00_001EFFF0(v1, vp, 2, 0, 0)) goto L_BAC;
    h = (char *)&D_L05_00174340_4C558;
    if (*(int *)(h + 0x18) != 0) goto L_BAC;
    if (*(int *)(h + 0x1C) == 0) goto L_BAC;
    if (!(1.2217305f < func_L00_001FF860(*(float *)(h + 0x48), func_001F9CE8_4C558(h + 0x40)))) goto L_BAC;
    g->f8A4 = FastAddRots(func_L00_001FF860(*(float *)(h + 0x40), *(float *)(h + 0x44)), 3.1415927f);
    if (!(FastDiffRots(func_L00_001FF860(g->fE0[0], g->fE0[1]), g->f8A4) < 0.7853982f)) goto L_BAC;
    if (g->f2DC < 1.0f) {
        Hero_TryMoveStateL05(0x6F, 1);
        return 1;
    }
    Hero_TryMoveStateL05(0x6D, 1);
    return 1;

L_BAC:
    if (((H_9F0 *)(D_0013E633 + 0xE1D))->b257 != 0) {
        f20 = func_001F9CE8_4C558(((H_9F0 *)(D_0013E633 + 0xE1D))->f100);
        if (f20 < func_001F9CE8_4C558(((H_9F0 *)(D_0013E633 + 0xE1D))->fE0) * 0.25f) {
            Hero_TryMoveStateL05(0x6D, 1);
            return 1;
        }
    }
    ee = D_0015EE84_2523C8;
    if (ee == 5) {
        if (((H_9F0 *)(D_0013E633 + 0xE1D))->f80[2] < 61.4f) {
            ((H_9F0 *)(D_0013E633 + 0xE1D))->f8A0 = 61.4f;
            Hero_TryMoveStateL05(0x6E, 1);
            return 1;
        }
    }
    if (ee == 0x10) {
        if (((H_9F0 *)(D_0013E633 + 0xE1D))->f80[2] < 76.0f) {
            ((H_9F0 *)(D_0013E633 + 0xE1D))->f8A0 = 76.0f;
            Hero_TryMoveStateL05(0x6E, 1);
            return 1;
        }
        if (((H_9F0 *)(D_0013E633 + 0xE1D))->b12EA != 0) {
            if (((H_9F0 *)(D_0013E633 + 0xE1D))->f2DC < 0.1f) {
                Hero_TryMoveStateL05(0x6D, 1);
                return 1;
            }
        }
    }
    if (((H_9F0 *)(D_0013E633 + 0xE1D))->s30C != 0) goto L_DEC;
    pp = ((H_9F0 *)(D_0013E633 + 0xE1D))->p8B4;
    if (pp == 0) goto L_D68;
    vp = ((H_9F0 *)(D_0013E633 + 0xE1D))->f80;
    r16 = FindClosestDistanceEntry(vp, pp, 0.0f);
    r4 = offset_path_index(((H_9F0 *)(D_0013E633 + 0xE1D))->p8B4, r16, -1, 1);
    ((H_9F0 *)(D_0013E633 + 0xE1D))->s8C8 = r16;
    if (D_0015EE84_2523C8 != 5) goto L_D68;
    pts = (float (*)[4])((H_9F0 *)(D_0013E633 + 0xE1D))->p8B4;
    f1 = *(float *)((char *)pts + (r16 << 4) + 0x18);
    f0 = *(float *)((char *)pts + (r4 << 4) + 0x18);
    if (f1 < f0) f20 = f1;
    else f20 = f0;
    f2 = func_001F9D48(vp, pts[r16 + 1]);
    if (0.5f < f20 - ((H_9F0 *)(D_0013E633 + 0xE1D))->f80[2]) {
        if (4.0f < f2) {
            Hero_TryMoveStateL05(0x6D, 1);
            return 1;
        }
    }

L_D68:
    if (((H_9F0 *)(D_0013E633 + 0xE1D))->s30C != 0) goto L_DEC;
    if (((H_9F0 *)(D_0013E633 + 0xE1D))->s8BC == 0) goto L_DEC;
    if (((H_9F0 *)(D_0013E633 + 0xE1D))->s89C != 0) goto L_DEC;
    m = ((H_9F0 *)(D_0013E633 + 0xE1D))->p2080;
    if (m[0x53] != 0x55 && m[0x53] != 0x7F) goto L_DEC;
    if (m[0x51] < 0xD) {
        int r = Hero_GetHelpAnimId();
        int t = scale_ticks(7);
        set_hero_animation(r, 14, (float)t);
    }

L_DEC:
    ok = (((H_9F0 *)(D_0013E633 + 0xE1D))->s30C == 0);
    if (((H_9F0 *)(D_0013E633 + 0xE1D))->b257 != 0) {
        if (FastAbsF(((H_9F0 *)(D_0013E633 + 0xE1D))->f218 - ((H_9F0 *)(D_0013E633 + 0xE1D))->f2D8) < 0.05f) ok = 1;
    }
    if (ok == 0) return 0;
    if (((H_9F0 *)(D_0013E633 + 0xE1D))->s89C == 0) return 0;
    if (!(((H_9F0 *)(D_0013E633 + 0xE1D))->fE0[2] < 0.0f)) return 0;
    f20 = 1.0471976f;
    if (f20 < FastAbsF(((H_9F0 *)(D_0013E633 + 0xE1D))->f94)) {
        Hero_TryMoveStateL05(0x6D, 1);
        return 1;
    }
    if (f20 < FastAbsF(((H_9F0 *)(D_0013E633 + 0xE1D))->f90)) {
        Hero_TryMoveStateL05(0x6D, 1);
        return 1;
    }
    if (1.3962634f < FastDiffRots(((H_9F0 *)(D_0013E633 + 0xE1D))->f98, ((H_9F0 *)(D_0013E633 + 0xE1D))->f874)) {
        Hero_TryMoveStateL05(0x6D, 1);
        return 1;
    }
    if (((H_9F0 *)(D_0013E633 + 0xE1D))->p2080[0x53] >= 0x69 && ((H_9F0 *)(D_0013E633 + 0xE1D))->p2080[0x53] <= 0x6C) {
        if (((H_9F0 *)(D_0013E633 + 0xE1D))->fAA8 < (float)((int *)&D_L05_0015F778)[((H_9F0 *)(D_0013E633 + 0xE1D))->b88F]) {
            Hero_TryMoveStateL05(0x6D, 1);
            return 1;
        }
    }

    if (D_0013D355_254DE8[0x13B] != 0) {
        if (D_0015EE84_2523C8 == 5 && ((H_9F0 *)(D_0013E633 + 0xE1D))->i6F0 == 4 && ((H_9F0 *)(D_0013E633 + 0xE1D))->i70C >= 4) {
            Flags_9F0 *fl = (Flags_9F0 *)(menu_item_enabled_115 + 1);
            if (fl->f7 == 0) {
                fl->f7 = 1;
                func_0022EE28_254DE8(1, 0, 0);
                func_L00_00264DB8_254DE8(0x542D, -1);
            }
        }
        ((H_9F0 *)(D_0013E633 + 0xE1D))->i714 = 0;
    }

    if (((H_9F0 *)(D_0013E633 + 0xE1D))->i2084 == 0x6C) {
        Hero_TryMoveStateL05(0x6B, 0);
        ee = (int)((H_9F0 *)(D_0013E633 + 0xE1D))->p2080;
        if (((unsigned char *)ee)[0x53] != 0x55 && ((unsigned char *)ee)[0x53] != 0x7F) {
            int r = Hero_GetHelpAnimId();
            int t = scale_ticks(7);
            set_hero_animation(r, 14, (float)t);
        }
        return 1;
    }
    c = ((H_9F0 *)(D_0013E633 + 0xE1D))->p2080[0x53];
    if (c != 0x55 && c != 0x7F) {
        int r = Hero_GetHelpAnimId();
        int t = scale_ticks(7);
        set_hero_animation(r, 14, (float)t);
    }
    return 0;
}
