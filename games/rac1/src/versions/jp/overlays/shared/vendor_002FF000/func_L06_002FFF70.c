void func_L06_002FFF70(char *m) {
    extern unsigned char D_0013D510[];
    char *d;
    char *t;
    char *p;
    int *e;
    int r;
    int k;
    int bits;
    float vv[4];
    float vec2[4];
    int hit;
    float dt;
    float f;
    float lo, hi, spd, off, pull;

    d = *(char **)(m + 0x78);
    *(float *)(m + 0x2C) = *(float *)(*(char **)(m + 0x24) + 0x24) * *(float *)&D_L06_00162038;
    update_blarg_space_commando_beam(m);
    if (*(int *)(d + 0x38) != 0) {
        *(int *)(d + 0x188) = truncate_float_to_s32(func_001F9878(random_float_between(180.0f, 240.0f)));
        *(int *)(d + 0x38) = 0;
    }
    func_001F9908((s32 *)(d + 0x188));
    *(int *)&dt = 0;
    t = MobyGetHitMessage(m, 0x330000, 0);
    r = ComputeHitResponse(m, t, d + 0x20, 0, &hit, &dt, 0, 4);
    if (hit != 1 && (unsigned char)m[0x20] != 0x10) {
        if (*(int *)&D_0015EE84 == 0xA && t != 0 && *(char **)(t + 0x20) != 0 &&
            *(short *)(*(char **)(t + 0x20) + 0xA6) == 0x31A && gSkillPoints[0x11] == 0) {
            gSkillPoints[0x11] = 1;
            PlayBankSound(1, 0, 0);
            ShowBanner(0x542D, -1);
        }
        f = *(float *)(d + 0x20) - dt;
        *(float *)(d + 0x20) = f;
        if (f <= 0.0f) r = 1;
        *(int *)(d + 0x80) = 0x1F4;
        *(float *)(d + 0x88) = 0.5f;
        switch (r) {
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        {
            char *q;
            int n;
            float b, a;
            m[0x20] = 0xF;
            q = d + 0x60;
            n = scale_ticks(0x3C);
            a = *(float *)&D_L06_00162040 * D_0015EE6C;
            b = *(float *)&D_L06_00162044 * D_0015EE6C;
            *(short *)(d + 0x26) = n;
            *(float *)(d + 0x7C) = b;
            *(float *)(d + 0x78) = a;
            *(float *)(d + 0xB0) = 8.0f;
            *(float *)(d + 0xB4) = 15.0f;
            InitKnockback(m, q, func_L00_001FF860(*(float *)(t + 0x10), *(float *)(t + 0x14)), 7, 1, 0);
            *(unsigned char *)(d + 0x117) = 0xFA;
            break;
        }
        case 1:
        case 2:
        {
            char *q;
            float a, b;
            m[0x20] = 0x10;
            q = d + 0x60;
            a = *(float *)&D_L06_00162048 * D_0015EE6C;
            b = *(float *)&D_L06_0016204C * D_0015EE6C;
            *(unsigned short *)(m + 0x34) &= 0xEFFF;
            *(float *)(d + 0x78) = a;
            *(float *)(d + 0x7C) = b;
            *(float *)(d + 0xB0) = 9.0f;
            *(float *)(d + 0xB4) = 18.0f;
            bits = 0;
            lo = *(float *)&bits;
            bits = 0x3E4CCCCD;
            hi = *(float *)&bits;
            a = func_L00_001FF860(*(float *)(t + 0x10), *(float *)(t + 0x14));
            bits = 0x40400000;
            off = *(float *)&bits;
            InitKnockback(m, q, a, 9, 1, 0);
            bits = 0x40E00000;
            pull = *(float *)&bits;
            for (k = 0; k < 10; k++) {
                float ang;
                qcopy(vv, m + 0x10);
                vv[2] = vv[2] + 2.5f;
                vv[0] = vv[0] + randf_sym(lo, hi);
                vv[1] = vv[1] + randf_sym(lo, hi);
                vv[2] = vv[2] + randf_sym(lo, hi);
                ang = random_angle_radians();
                spd = random_float_between(off, 5.7f) * D_0015EE6C;
                vec2[0] = FastCos(ang) * spd;
                vec2[1] = FastSin(ang) * spd;
                vec2[2] = lo;
                vec2[0] = vec2[0] + FastCos(func_L00_001FF860(*(float *)(t + 0x10), *(float *)(t + 0x14))) * (D_0015EE6C * pull);
                vec2[1] = vec2[1] + FastSin(func_L00_001FF860(*(float *)(t + 0x10), *(float *)(t + 0x14))) * (D_0015EE6C * pull);
                vec2[2] = random_float_between(off, 8.5f) * D_0015EE6C;
                p = SpawnRandomDebris((int)m, (char *)vv, (char *)vec2);
                if (p != 0) {
                    *(float *)(p + 0x2C) = *(float *)(p + 0x2C) * 0.4f;
                }
            }
            *(unsigned char *)(d + 0x117) = 0xFA;
            SetDeathBits(m, 0, -1);
            break;
        }
        case 0:
        case 11:
        default:
            break;
        }
        RerollEffectCounter(m, (short *)(d + 0x110));
    }
    *(unsigned char *)(m + 0xA4) = 0xFF;
    StepColorFade(m, d + 0x110);
    if (*(int *)(d + 0x188) != 0) {
        *(float *)(d + 0x198) = *(float *)&D_L06_0016203C + 6.0f;
    } else {
        *(float *)(d + 0x198) = *(float *)&D_L06_0016203C;
    }
    {
    char *q = d + 0x120;
    e = D_L06_001B0FB0[*(int *)(d + 0x180)];
    if (FindNearestEligibleMoby(*(float *)(d + 0x198), m, q, 0, 0, e + 4, *e) != 2) {
        if (*(float *)(d + 0x198) < func_001F9D48(d + 0x170, d + 0x120)) {
            *(int *)(d + 0x164) = 2;
        } else if (3.0f < FastAbsF(*(float *)(m + 0x18) - *(float *)(d + 0x128))) {
            *(int *)(d + 0x164) = 2;
        }
    }
    }
    if (*(int *)(d + 0x160) == 0) {
        *(int *)(d + 0x160) = *(int *)(D_0013E633 + 0x2E9D);
    }
}
