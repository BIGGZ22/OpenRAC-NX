void ToxicCrabL10_Update(void *mv) {
    Moby *m = mv;
    V_2E52B0 vA;
    V_2E52B0 vB;
    V_2E52B0 W;
    V_2E52B0 X;
    V_2E52B0 dst;
    int A;
    float B;
    float fx;
    D_2E52B0 *d = (D_2E52B0 *)m->pvars;
    Hit_2E52B0 *r;
    Moby *q;
    int v21;

    if (((Hero *)(D_0013E633_2E52B0 + 0xE1D))->items[0].item_id == 0x15) {
        if (((D_2E52B0 *)((Hero *)(D_0013E633_2E52B0 + 0xE1D))->items[0].moby->pvars)->f10 == m) {
            func_L10_002E14F8_2E52B0(m);
            if (m->state == 1) m->state = 2;
        }
    }
    if (d->f38 != 0) {
        func_L10_002E14F8_2E52B0(m);
        d->f1AC = func_001FA898_2E52B0(func_001F9878_2E52B0(func_002140F8_2E52B0(180.0f, 240.0f)));
        d->f38 = 0;
    }
    func_001F9908_2E52B0(&d->f1AC);
    A = 1;
    B = 0.0f;
    r = func_L00_0025B478_2E52B0(m, 0xB30000, 0);
    v21 = func_L00_0025B4D0_2E52B0(m, r, &d->f20, 0, &A, &B, 0, 4);
    func_001F9938_2E52B0(&d->h26);
    if ((unsigned int)A >= 2 && m->state != 0xB && m->state != 0xC) {
        if (D_0015EE84_m == 10 && r != 0 && r->f20 != 0 && r->f20->oclass == 0x31A) {
            if (d->f1AC != 0 && ((Flags_2E52B0 *)(D_0013D50F_2E52B0 + 1))->b11 == 0) {
                ((Flags_2E52B0 *)(D_0013D50F_2E52B0 + 1))->b11 = 1;
                func_0022EE28_2E52B0(1, 0, 0);
                func_L00_00264DB8_2E52B0(0x542D, -1);
            }
        }
        if (d->h26 != 0 && r->b28 == 0) {
            v21 = 11;
            if (B < 3.0f) B = 0.0f;
        }
        if (((Hero *)(D_0013E633_2E52B0 + 0xE1D))->state.current == 32
            || (((Hero *)(D_0013E633_2E52B0 + 0xE1D))->state.prev == 32
                && ((Hero *)(D_0013E633_2E52B0 + 0xE1D))->state_timer < func_001F9850_2E52B0(30))) {
            if (2.0f < B) B = 2.0f;
        }
        if (r->h2A == m->oclass) {
            B = 0.0f;
            v21 = 11;
        }
        if ((unsigned int)(m->prev_seq - 6) < 2) {
            if (((Hero *)(D_0013E633_2E52B0 + 0xE1D))->vehicle_mode == 1
                && r->f20 == ((Hero *)(D_0013E633_2E52B0 + 0xE1D))->moby) {
                B = 0.0f;
                v21 = 11;
            }
        }
        if (r->h2A == 0x47) {
            *(u128_2E52B0 *)&vA = *(u128_2E52B0 *)(D_0013E633_2E52B0 + 0xE9D);
            *(u128_2E52B0 *)&vB = *(u128_2E52B0 *)((V_2E52B0 *)&m->pos);
            vA.z += 1.0f;
            vB.z += 1.0f;
            if (func_L00_001EFFF0_2E52B0(&vA, &vB, 2, m, 0) != 0) {
                v21 = 11;
                B = 0.0f;
            }
        } else if ((q = r->f20) != 0 && q->oclass != 0x359) {
            *(u128_2E52B0 *)&vA = *(u128_2E52B0 *)((V_2E52B0 *)&q->pos);
            *(u128_2E52B0 *)&vB = *(u128_2E52B0 *)((V_2E52B0 *)&m->pos);
            vB.z += 1.0f;
            if (r->f20->oclass == 0xBA || r->f20->oclass == 0) vA.z += 0.25f;
            if (func_L00_001EFFF0_2E52B0(&vA, &vB, 0x12, m, 0) != 0) {
                func_001F9BF0_2E52B0(&dst, &vB, &vA);
                *(u128_2E52B0 *)&W = *(u128_2E52B0 *)&dst;
                func_L00_001FF4B0_2E52B0(&W, &W, 1.0f);
                func_L00_001FF4B0_2E52B0(&X, &D_L10_00174380_2E52B0, 1.0f);
                if (0.5f < func_001F9B88_2E52B0(func_001F9C78_2E52B0(&W, &X))) {
                    B = 0.0f;
                    v21 = 11;
                }
            }
        }
        d->f20 -= B;
        if (d->f20 <= 0.0f) v21 = 1;
        func_L10_002E14F8_2E52B0(m);
        d->f90 = func_001FA898_2E52B0(768.0f);
        d->f98 = 0.75f;
        d->f94 = 9;
        d->bAD = 0;
        fx = func_L00_001FF860_2E52B0(r->pos.x, r->pos.y);
        switch (v21) {
        case 3:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            m->state = 10;
            d->h26 = func_001F9850_2E52B0(60);
            d->f80 = D_L10_00161EAC_2E52B0 * D_0015EE70_2E52B0;
            func_L03_00251A58_2E52B0(d->a70, 0.5f, 0.5f);
            d->fC0 = 8.0f;
            d->fC4 = 10.0f;
            *(u128_2E52B0 *)&vA = *(u128_2E52B0 *)&r->pos;
            func_L00_0025BBA0_2E52B0(&vA, &fx, &d->f88, &d->f8C);
            func_L00_0025D5B0_2E52B0(m, d->a70, 9, 1, 0, fx);
            d->b67 = 0xFA;
            func_L00_0025E4B0_2E52B0(m, d->a60);
            break;
        case 4:
        case 5: {
            float s;
            m->state = 10;
            d->h26 = func_001F9850_2E52B0(60);
            d->f80 = D_L10_00161EAC_2E52B0 * D_0015EE70_2E52B0;
            s = ((Hero *)(D_0013E633_2E52B0 + 0xE1D))->vehicle_mode == 1 ? 1.0f : 3.0f;
            func_L03_00251A58_2E52B0(d->a70, s, 0.5f);
            d->fC0 = 3.5f;
            d->fC4 = 7.0f;
            *(u128_2E52B0 *)&vA = *(u128_2E52B0 *)&r->pos;
            func_L00_0025BBA0_2E52B0(&vA, &fx, &d->f88, &d->f8C);
            func_L00_0025D5B0_2E52B0(m, d->a70, 9, 1, 0, func_L00_001FF860_2E52B0(r->pos.x, r->pos.y));
            d->b67 = 0xFA;
            func_L00_0025E4B0_2E52B0(m, d->a60);
            break;
        }
        case 1:
        case 2:
            m->state = 11;
            m->flags &= 0xEFFF;
            break;
        case 0:
        case 11:
            break;
        }
    }
    m->hit_slot = 0xFF;
    func_L00_0025E590_2E52B0(m, d->a60);
    if ((unsigned char)(m->state - 10) < 3) {
        d->f1D4 = 0;
        return;
    }
    if (d->f1AC != 0) {
        d->f1A0 = D_L10_00161EC0_2E52B0 + 6.0f;
    } else if (m->state == 1) {
        if (m == ((Moby *)((Hero *)(D_0013E633_2E52B0 + 0xE1D))->unk240) || m == ((Moby *)((Hero *)(D_0013E633_2E52B0 + 0xE1D))->coll_hit_moby)) {
            d->f1A0 = 6.0f;
        } else {
            d->f1A0 = 0.0f;
        }
        if (((Hero *)(D_0013E633_2E52B0 + 0xE1D))->vehicle_mode == 1) func_L10_002E14F8_2E52B0(m);
    } else {
        d->f1A0 = D_L10_00161EC0_2E52B0;
    }
    if (d->f1AC != 0) {
        func_L00_00260D30_2E52B0(m, &d->a120, d->f1A0);
        if (d->f1A0 < func_001F9D48_2E52B0(&d->a170, &d->a120) || 3.0f < func_001F9B88_2E52B0(m->pos.z - d->a120.z)) {
            d->f164 = 2;
        }
    } else if (d->f1A0 < func_001F9D48_2E52B0(&d->a170, &d->a120) || 3.0f < func_001F9B88_2E52B0(m->pos.z - d->a120.z)) {
        func_L00_00260D30_2E52B0(m, &d->a120, d->f1A0);
        d->f164 = 2;
    } else {
        func_L00_00260FB0_2E52B0(d->f1A0, m, &d->a120, 0, 0, D_L10_001B0C30_2E52B0[d->f1B0]->nodes,
            D_L10_001B0C30_2E52B0[d->f1B0]->count);
    }
    if (m->state != 1 && d->f164 == 2) {
        d->f1D4++;
        if (func_001F9850_2E52B0(300) < d->f1D4) {
            if (1.0f < func_001F9D10_2E52B0(&m->pos, &d->a170)) {
                m->state = 8;
                if (m->prev_seq != 4) func_00213DE0_2E52B0(m, 4, 0, func_001F9850_2E52B0(12));
            } else if (((Hero *)(D_0013E633_2E52B0 + 0xE1D))->vehicle_mode != 1) {
                m->state = 1;
                if (m->prev_seq != 0) func_00213DE0_2E52B0(m, 0, 0, func_001F9850_2E52B0(20));
            }
        }
    } else {
        d->f1D4 = 0;
    }
    if (d->f160 == 0) d->f160 = ((Hero *)(D_0013E633_2E52B0 + 0xE1D))->moby;
}
