void magnetboots_update(Moby *m) {
    char *d = (char *)m->pvars;
    m->rot.z = addrot_00299818(m->rot.z, D_0015EE6C_x * 1.5707964f);
    switch (m->state) {
    case 0:
        func_L00_002D80A0_00299818(m);
        if (*(D_0013D605_00299818 + 7) != 0) {
            remove_00299818(m);
            return;
        }
        m->state = 1;
        m->pos.z = m->pos.z + 0.5f;
        m->scale = m->scale * 0.667f;
        break;
    case 1:
        func_L10_00299AF0_00299818(m);
        if (dist_00299818(&m->pos, D_0013F450_00299818 + 0x80) < 3.0f) {
            char *h = D_0013F450_00299818;
            if (fabs_00299818(m->pos.z - *(float *)(h + 0x88)) < 2.0f && *(int *)(h + 0x22A8) != 0) {
                m->flags |= 0x41;
                start_scene_00299818(2);
                m->state = 2;
            }
        }
        break;
    case 2:
        if (D_L10_0015F6A8_00299818 != 2) {
            func_L00_00264DB8_00299818(0x271B, -1);
            D_L10_0015F720_00299818 = 0xB4;
            func_L00_002618D8_00299818(0x1C, 1);
            if (*(int *)(d + 4) != -1) {
                char *o = D_L10_00160058_00299818 + (*(int *)(d + 4) << 8);
                func_L00_00286128_00299818(o + 0x10, o + 0x40);
            }
            save_00299818(0, -1);
            m->state = 3;
        }
        break;
    case 3:
        remove_00299818(m);
        break;
    }
}
