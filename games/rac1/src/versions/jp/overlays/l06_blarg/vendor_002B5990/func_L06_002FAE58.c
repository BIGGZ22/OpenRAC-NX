void alien_queen_update(unsigned char *moby)
{
    unsigned char *data = *(unsigned char **)(moby + 0x78);
    char *target;
    float v[4];

    AlienQueen_ApplyDamage(moby);
    target = *(char **)(data + 0xB0);
    func_L00_00264B40_q(moby, *(int *)&D_L06_00161F80, 1.9f, data + 0x130);
    if ((unsigned char)moby[0x31] != 0 && func_001F9D10_q(moby + 0x10, D_L06_00167640_q) < 45.0f) {
        func_L00_0025B178_q(moby);
        moby[0x7F] = 0x24;
    }
    if ((unsigned char)moby[0x20] >= 3 && (unsigned char)moby[0x20] <= 13) {
        *(int *)(data + 0x10C) = func_001FFB38_q(0x16, 0xFFFF, func_L00_00236710_q, func_L00_00236830_q,
                                                 func_L06_0024A080_q + 0x20, data + 0x110, 0x1EF);
    }
    switch ((unsigned char)moby[0x20]) {
    case 0: {
        int *p;
        *(float *)(data + 0x20) = 30.0f;
        *(short *)(data + 0x24) = 0x1E;
        data[0x29] = 1;
        data[0x28] = 3;
        data[0x2E] = 1;
        data[0x58] = func_001FA898_q(14.0f);
        data[0x5A] = func_001FA898_q(18.4f);
        *(int *)(data + 0x120) = func_001F9850_q(0x4B0);
        *(int *)(data + 0x178) = 0;
        qcopy(data + 0xC0, moby + 0x10);
        *(int *)(data + 0x10C) = -1;
        *(int *)(data + 0x114) = *D_L06_001B0FB0_q[*(int *)(data + 0xEC)];
        p = D_L06_001B0FB0_q[*(int *)(data + 0xEC)];
        func_001F9C08_q(data + 0xD0, p + 4, (char *)p + ((*p / 2) * 16 + 16), 0.5f);
        *(float *)(data + 0xDC) = func_001F9D48_q(data + 0xC0, data + 0xD0);
        *(float *)(data + 0x128) = func_00214158_q();
        *(int *)(data + 0x12C) = func_001F9850_q(0x1E);
        *(int *)(moby + 0x94) = 0;
        *(unsigned short *)(moby + 0x34) = (*(unsigned short *)(moby + 0x34) | 0x4001) & 0xEFFF;
        *(float *)(moby + 0x18) = *(float *)(moby + 0x18) + 17.5f;
        func_L06_002FAD78_q(*(int *)(data + 0xF4));
        if (D_0013D4AF_q[4] != 0 && D_0013D605_q[8] == 0) {
            func_0020D678_q(moby);
            return;
        }
        moby[0x20] = 1;
        break;
    }
    case 1:
        if (*(int *)(data + 0x170) == 0) {
            if (*(int *)(data + 0xB4) == 2) return;
            if (*(short *)(target + 0xA6) != 0) return;
            *(int *)(data + 0x170) = 1;
            *(int *)(data + 0x12C) = func_001F9850_q(10);
            break;
        }
        if (*(int *)(data + 0x12C) != 0) return;
        func_L06_002F6818_q(D_L06_00160058_q + (*(int *)(data + 0xFC) << 8));
        func_L06_002F6818_q(D_L06_00160058_q + (*(int *)(data + 0x100) << 8));
        func_L06_002DB1D8_q(*(int *)(data + 0x104));
        *(int *)(data + 0x170) = 0;
        moby[0x20] = 2;
        *(int *)(moby + 0x94) = *(int *)(*(char **)(moby + 0x24) + 0x10);
        *(unsigned short *)(moby + 0x34) = (*(unsigned short *)(moby + 0x34) | 0x1000) & 0xFFFE;
        if ((unsigned char)moby[0x53] != 8) {
            func_00213DE0_q(moby, 8, 0, func_001F9850_q(1));
        }
        func_L06_002FC578_q(moby);
        return;
    case 2: {
        float g = D_0015EE70_q * 30.0f;
        *(float *)(moby + 0x18) = *(float *)(moby + 0x18) + *(float *)(data + 0x48);
        *(float *)(data + 0x48) = *(float *)(data + 0x48) - g;
        if ((unsigned char)moby[0x53] == 8) {
            if (func_001F9B88_q((*(float *)(moby + 0x18) - *(float *)(data + 0xC8)) / *(float *)(data + 0x48)) <= 6.0f
                && (unsigned char)moby[0x53] != 9) {
                func_00213DE0_q(moby, 9, 0, func_001F9850_q(5));
            }
        } else {
            if (*(float *)(moby + 0x18) < *(float *)(data + 0xC8)) {
                *(float *)(moby + 0x18) = *(float *)(data + 0xC8);
            }
            if (*(float *)(moby + 0x54) == 0.0f && (moby[0x70] & 2)) {
                if ((unsigned char)moby[0x53] == 9) {
                    func_00213DE0_q(moby, 6, 0, func_001F9850_q(0x1E));
                } else if ((unsigned char)moby[0x53] != 1) {
                    func_00213DE0_q(moby, 1, 0, func_001F9850_q(0x1E));
                }
            }
        }
        if (func_L06_002FC6D8_q(moby) != 0) {
            moby[0x20] = 7;
            func_L06_002FC8E0_q(moby);
        }
        break;
    }
    case 3:
        if ((unsigned char)moby[0x53] == 6 && *(float *)(moby + 0x54) == 0.0f && (moby[0x70] & 2)) {
            func_00213DE0_q(moby, 1, 0, func_001F9850_q(0x14));
            func_L06_002FC4C0_q(moby);
        }
        if (func_L06_002FCA98_q(moby) != 0) {
            moby[0x20] = 4;
            func_L06_002FCCA0(moby);
            return;
        }
        break;
    case 5:
        if ((unsigned char)moby[0x53] == 6 && *(float *)(moby + 0x54) == 0.0f && (moby[0x70] & 2)) {
            func_00213DE0_q(moby, 1, 0, func_001F9850_q(0x14));
            func_L06_002FADE0_q(*(int *)(data + 0xF4));
        }
        if (func_L06_002FCE58(moby) != 0) {
            moby[0x20] = 6;
            func_L06_002FD060(moby);
        }
        break;
    case 4:
        func_L06_002FC4C0_q(moby);
    case 6:
        func_L06_002FD0B8_q(moby);
        if (((unsigned char)moby[0x20] == 4 && func_L06_002EB2D0_q(*(int *)(data + 0xF0)) != 0)
            || ((unsigned char)moby[0x20] == 6 && func_L01_0026EFB8_q(*(int *)(data + 0xF4), -1) == 0)) {
            moby[0x20] = 8;
            *(int *)(data + 0x120) = func_001F9850_q(0x4B0);
            if ((unsigned char)moby[0x53] != 3) {
                func_00213DE0_q(moby, 3, 0, func_001F9850_q(0x14));
            }
        }
        return;
    case 7:
        if (*(int *)(data + 0xB4) == 2) break;
        moby[0x20] = 8;
        if ((unsigned char)moby[0x53] == 3) break;
        func_00213DE0_q(moby, 3, 0, func_001F9850_q(0x14));
        return;
    case 8: {
        float dist = func_001F9D48_q(moby + 0x10, target + 0x10);
        float turn = func_001FA850_q(*(float *)(moby + 0x48),
                                     func_L00_001FF860_q(*(float *)(target + 0x10) - *(float *)(moby + 0x10),
                                                         *(float *)(target + 0x14) - *(float *)(moby + 0x14)));
        func_L00_0025CE58_q((float *)(moby + 0x48),
                            func_L00_001FF860_q(*(float *)(target + 0x10) - *(float *)(moby + 0x10),
                                                *(float *)(target + 0x14) - *(float *)(moby + 0x14)),
                            (float *)(data + 0xE0), D_0015EE70_q * 6.2831855f, D_0015EE70_q * 6.2831855f,
                            D_0015EE6C_q * 3.1415927f);
        if (turn < 0.7853982f && *(float *)&D_L06_00161F90 < dist) {
            func_00214D28_q((float *)(data + 0xE8), *(float *)&D_L06_00161F84 * D_0015EE6C_q,
                            *(float *)&D_L06_00161F8C * D_0015EE70_q);
        } else if (dist < *(float *)&D_L06_00161F94) {
            func_00214D28_q((float *)(data + 0xE8), -(*(float *)&D_L06_00161F84 * D_0015EE6C_q),
                            *(float *)&D_L06_00161F8C * D_0015EE70_q);
        } else {
            func_00214D28_q((float *)(data + 0xE8), 0.0f, D_0015EE70_q * 8.0f);
        }
        func_001F9BF0_q(v, target + 0x10, moby + 0x10);
        v[2] = 0.0f;
        func_L00_001FF4B0_q(v, v, *(float *)(data + 0xE8));
        func_001F9BD8_q(moby + 0x10, moby + 0x10, v);
        func_L00_00262DF0_q(*(int *)(data + 0xEC), moby + 0x10, 1.0f, moby + 0x10);
        if (*(int *)(data + 0xB4) == 2) {
            moby[0x20] = 0xD;
            break;
        }
        if (func_001F9908_q(data + 0x120) != 0) {
            *(int *)(data + 0x120) = func_001F9850_q(0x168);
            moby[0x20] = 0xC;
            break;
        }
        if (dist < *(float *)&D_L06_00161F90 && turn < *(float *)&D_L06_00161F98 * 0.017453292f) {
            moby[0x20] = 9;
            moby[0xBC] = 7;
            if ((unsigned char)moby[0x53] != 4) {
                func_00213DE0_q(moby, 4, 0, func_001F9850_q(0x14));
                return;
            }
        }
        break;
    }
    case 12:
        func_L06_002FD0B8_q(moby);
        if (func_001F9908_q(data + 0x120) != 0) {
            *(int *)(data + 0x120) = func_001F9850_q(0x4B0);
            moby[0x20] = 0xB;
            moby[0xBC] = 7;
            if ((unsigned char)moby[0x53] != 6) {
                func_00213DE0_q(moby, 6, 0, func_001F9850_q(0x14));
            }
        }
        return;
    case 9:
        func_001F9908_q(data + 0x120);
        func_L00_0025CE58_q((float *)(moby + 0x48), *(float *)(moby + 0x48), (float *)(data + 0xE0),
                            D_0015EE70_q * 6.2831855f, D_0015EE70_q * 6.2831855f, D_0015EE6C_q * 3.1415927f);
        if (moby[0x70] & 2) {
            moby[0x20] = 0xA;
            if ((unsigned char)moby[0x53] != 5) {
                func_00213DE0_q(moby, 5, 0, func_001F9850_q(0x14));
                return;
            }
        }
        break;
    case 10: {
        QueenProbe probe;
        float buf[4];
        func_001F9908_q(data + 0x120);
        if (1.0f < func_0020D830_q(moby) && func_0020D830_q(moby) < 8.0f) {
            v[0] = func_001F9F90_q(*(float *)(moby + 0x48)) * 0.45f;
            v[1] = func_001F9FA8_q(*(float *)(moby + 0x48)) * 0.45f;
            v[2] = 0.0f;
            probe.owner = (char *)moby;
            probe.flags = 1;
            probe.scale = 1.0f;
            probe.count = 1;
            probe.dir[0] = func_001F9F90_q(*(float *)(moby + 0x48));
            probe.dir[1] = func_001F9FA8_q(*(float *)(moby + 0x48));
            probe.dir[2] = 0.0f;
            func_L00_00250800_q(moby, 0, buf);
            func_L00_001F2BE8_q(buf, 0.5f, 0, moby, &probe);
            func_L00_00250800_q(moby, 1, buf);
            func_L00_001F2BE8_q(buf, 0.5f, 0, moby, &probe);
            func_L00_00250800_q(moby, 2, buf);
            func_L00_001F2BE8_q(buf, 0.5f, 0, moby, &probe);
            func_L00_00250800_q(moby, 3, buf);
            func_L00_001F2BE8_q(buf, 0.5f, 0, moby, &probe);
            func_L00_00250800_q(moby, 4, buf);
            func_L00_001F2BE8_q(buf, 0.5f, 0, moby, &probe);
            func_L00_00250800_q(moby, 5, buf);
            func_L00_001F2BE8_q(buf, 1.1f, 0, moby, &probe);
            func_L00_00250800_q(moby, 6, buf);
            func_L00_001F2BE8_q(buf, 1.1f, 0, moby, &probe);
            return;
        }
        if (moby[0x70] & 2) {
            moby[0x20] = moby[0xBC];
            if ((unsigned char)moby[0x53] != 1) {
                func_00213DE0_q(moby, 1, 0, func_001F9850_q(0x14));
            }
        }
        return;
    }
    case 11:
        if (moby[0x70] & 2) {
            moby[0x20] = moby[0xBC];
            if ((unsigned char)moby[0x53] != 1) {
                func_00213DE0_q(moby, 1, 0, func_001F9850_q(0x14));
            }
        }
        return;
    case 13: {
        char *pos = (char *)moby + 0x10;
        char *home = (char *)data + 0xC0;
        float dist = func_001F9D48_q(pos, home);
        float turn = func_001FA850_q(*(float *)(moby + 0x48),
                                     func_L00_001FF860_q(*(float *)(data + 0xC0) - *(float *)(moby + 0x10),
                                                         *(float *)(data + 0xC4) - *(float *)(moby + 0x14)));
        func_L00_0025CE58_q((float *)(moby + 0x48),
                            func_L00_001FF860_q(*(float *)(data + 0xC0) - *(float *)(moby + 0x10),
                                                *(float *)(data + 0xC4) - *(float *)(moby + 0x14)),
                            (float *)(data + 0xE0), D_0015EE70_q * 6.2831855f, D_0015EE70_q * 6.2831855f,
                            D_0015EE6C_q * 3.1415927f);
        if (turn < 0.7853982f) {
            func_001F9BF0_q(v, home, pos);
            v[2] = 0.0f;
            func_L00_001FF4B0_q(v, v, *(float *)&D_L06_00161F84 * D_0015EE6C_q);
            func_001F9BD8_q(pos, pos, v);
            func_L00_00262DF0_q(*(int *)(data + 0xEC), pos, 1.0f, pos);
        }
        if (*(int *)(data + 0xB4) != 2) {
            moby[0x20] = 7;
            break;
        }
        if (dist < 1.0f) {
            moby[0x20] = 7;
            if ((unsigned char)moby[0x53] != 1) {
                func_00213DE0_q(moby, 1, 0, func_001F9850_q(0x14));
                return;
            }
        }
        break;
    }
    case 14: {
        unsigned char *flag;
        D_0013D4AF_q[4] = 1;
        *(int *)(data + 0x110) = 0;
        moby[0x30] = 0xFF;
        if (data[0x2E] == 2) {
            func_L06_002F6790_q(D_L06_00160058_q + (*(int *)(data + 0x100) << 8));
            func_L06_002DB248_q(*(int *)(data + 0x104));
            *(unsigned short *)(moby + 0x34) |= 1;
            moby[0x20] = 0xF;
            moby[0x31] = 0;
            *(int *)(moby + 0x94) = 0;
            if ((unsigned char)moby[0x53] != 1) {
                func_00213DE0_q(moby, 1, 0, func_001F9850_q(0x14));
                return;
            }
            break;
        }
        if (!(moby[0x70] & 2)) break;
        qcopy(v, moby + 0x10);
        v[2] = v[2] + 1.0f;
        func_L00_00260460_q(moby, v, -1, 2.0f, 20.0f);
        func_L00_00264EA8_q(moby, 0x6A2, 1, 0x6A3, 3, 6, 2);
        func_L06_002F6790_q(D_L06_00160058_q + (*(int *)(data + 0x100) << 8));
        func_L06_002DB248_q(*(int *)(data + 0x104));
        moby[0x20] = 0xF;
        if ((unsigned char)moby[0x53] != 1) {
            func_00213DE0_q(moby, 1, 0, func_001F9850_q(0x14));
        }
        *(int *)(moby + 0x94) = 0;
        moby[0x31] = 0;
        *(unsigned short *)(moby + 0x34) |= 1;
        if (*(int *)(data + 0x178) != 0) return;
        flag = D_0013D50F_q + 1;
        if (flag[0xA] != 0) return;
        flag[0xA] = 1;
        func_0022EE28_q(1, 0, 0);
        func_L00_00264DB8_q(0x542D, -1);
        return;
    }
    case 15: {
        int i;
        char *m = 0;
        for (i = 0; i < 0x30; i++) {
            if (D_L06_0016A110[i] != 0) {
                m = D_L06_001679D0 + i * 0xA0;
                if (*(short *)(m + 0x86) == 0x12) {
                    func_L06_00317770_q(m);
                    break;
                }
            }
        }
        if (D_L06_0015F6A8_q == 2) {
            func_L06_002F6790_q(D_L06_00160058_q + (*(int *)(data + 0xFC) << 8));
            func_0020D678_q(moby);
        }
        break;
    }
    }
}
