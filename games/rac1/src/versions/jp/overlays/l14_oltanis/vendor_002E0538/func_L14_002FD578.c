void update_enemy_fighter(struct Moby *moby) {
    Class908Vars_FD578 *data = (Class908Vars_FD578 *)moby->pvars;
    int hit = func_L00_0025B478_FD578(moby, 0x800000, 0);
    int index;
    int path_index;
    char *path;
    moby->hit_slot = 0xff;
    if (hit && moby->state != 2) {
        if (D_0015EE84_FD578 == 14) {
            D_0015EF08_FD578 = D_0015EF08_FD578 + 1;
            if (D_0015EF08_FD578 > 2 && D_0013D510_FD578.flag == 0) {
                D_0013D510_FD578.flag = 1;
                func_0022EE28_FD578(1, 0, 0);
                func_L00_00264DB8_FD578(0x542D, -1);
            }
        }
        moby->state = 2;
        func_L00_00260108_FD578(5.0f, 13.0f, moby, &moby->pos, -1);
        moby->collision = 0;
        moby->flags |= 0x41;
        index = data->sound;
        if (index != -1) {
            char *entry = D_0013E650_FD578 + index * 0x70;
            if (*(struct Moby **)(entry + 0x88) == moby && *(unsigned char *)(entry + 0x74) != 0)
                func_L00_0028EBF0_FD578(index);
        }
        data->sound = -1;
    }
    switch (moby->state) {
    case 0:
        path_index = data->path;
        if (path_index == -1) {
            func_001E9730_FD578(D_L14_001FD180_FD578, moby->save_id);
            func_0020D678_FD578(moby);
            return;
        }
        if (*(int *)D_L14_001B0F30_FD578[path_index] == 0) {
            func_001E9730_FD578(D_L14_001FD1B8_FD578, moby->save_id);
            func_0020D678_FD578(moby);
            return;
        }
        func_L14_002FDD18_FD578(moby);
        moby->state = 1;
        moby->update_dist = 0xff;
        moby->draw_dist = 0xff;
        data->sound = -1;
        data->timer = func_001F9850_FD578(D_L14_0016202C_FD578);
        break;
    case 1:
        func_L14_002FDE28_FD578(moby);
        break;
    case 2:
      if (func_001F9908_FD578(&data->timer)) {
        path = D_L14_001B0F30_FD578[data->path];
        moby->flags &= 0xffbe;
        moby->collision = moby->pclass->collision;
        data->timer = func_001F9850_FD578(D_L14_0016202C_FD578);
        moby->state = 1;
        data->segment = 0;
        data->along = 0;
        qcopy(&moby->pos, path + 0x10);
        moby->rot.z = func_L00_001FF860_FD578(*(float *)(path + 0x20) - *(float *)(path + 0x10),
                                              *(float *)(path + 0x24) - *(float *)(path + 0x14));
        data->f80 = data->f84 =
            data->f88 = data->f70 =
            data->f74 = data->f78 = 0.0f;
      }
      break;
    }
    func_L14_002FD870_FD578(moby);
    if (moby->state != 0 && moby->state != 2 &&
        func_L00_0028EB98_FD578(moby, data->sound) == 0)
        data->sound = func_0022ED80_FD578(0, 4, moby);
}
