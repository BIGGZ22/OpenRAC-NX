void update_bomber_tank_damage_and_aim(Moby *m) {
    Data_2D5FA8 *d = (Data_2D5FA8 *)m->pvars;
    Hit_2D5FA8 *t;
    Hit_2D5FA8 *u;
    Hit_2D5FA8 *c;
    float vec[4];
    int hit;
    float damage = 0.0f;
    int one;

    t = find_2D5FA8(m, 0x330000, 0);
    if (d->partA != 0) {
        u = find_2D5FA8(d->partA, 0x210000, 0);
    } else {
        u = 0;
    }
    if (d->partB != 0) {
        c = find_2D5FA8(d->partB, 0x210000, 0);
    } else {
        c = 0;
    }
    t = pick_2D5FA8(t, u, c, 0);
    if (t != 0 && t->moby != 0) {
        short h = t->moby->oclass;
        if (h == 0x1A9 || h == 0x47) t = 0;
    }
    damage_2D5FA8(m, t, &d->health, 0, &hit, &damage, 0, 4);
    one = 1;
    if (hit != one && m->state != 0) {
        d->health -= damage;
        if (damage != 0.0f) event_2D5FA8(4, 0, m);
        if (d->health > 0.0f) {
            if (damage < 4.0f) {
                d->tint = 0x78;
                if ((unsigned short)(t->moby->oclass - 0xB0) < 2) {
                    d->timer = ticks_2D5FA8(0x3C);
                } else {
                    d->timer = ticks_2D5FA8(0x1E);
                }
            } else if (damage < 6.0f) {
                d->tint = 0xC8;
                d->timer = ticks_2D5FA8(0x1E);
            } else {
                d->tint = 0xE6;
                d->timer = ticks_2D5FA8(0x1E);
            }
            if (m->state == 0xC && m->prev_seq != 1) {
                animate_2D5FA8(m, 1, 0, 0);
            }
        } else {
            int dead = 0x63;
            if (m->state != dead) {
                qcopy(vec, &m->pos);
                vec[2] += 3.0f;
                d->tint = 0xFA;
                zero_2D5FA8(d->f120);
                burst_2D5FA8(m, d->f40, vec, 0.0f, 0.0f, 20, 8, 20, 4.0f, 2.0f, 9.0f, 1.0f, -1, 15.0f, 1, one, -1, 0);
                spawn_2D5FA8(m, 0x6C0, (float *)&m->pos, (float *)&m->rot, 0, 0, 0.0f, D_L08_0015F660_2D5FA8, D_L08_0015F660_2D5FA8, D_L08_0015F660_2D5FA8);
                spawn_2D5FA8(m, 0x6C1, (float *)&m->pos, (float *)&m->rot, 0, 0, 0.0f, D_L08_0015F660_2D5FA8, D_L08_0015F660_2D5FA8, D_L08_0015F660_2D5FA8);
                spawn_2D5FA8(m, 0x6C2, (float *)&m->pos, (float *)&m->rot, 0, 0, 0.0f, D_L08_0015F660_2D5FA8, D_L08_0015F660_2D5FA8, D_L08_0015F660_2D5FA8);
                spawn_2D5FA8(m, 0x6C3, (float *)&m->pos, (float *)&m->rot, 0, 0, 0.0f, D_L08_0015F660_2D5FA8, D_L08_0015F660_2D5FA8, D_L08_0015F660_2D5FA8);
                spawn_2D5FA8(m, 0x6C4, (float *)&m->pos, (float *)&m->rot, 0, 0, 0.0f, D_L08_0015F660_2D5FA8, D_L08_0015F660_2D5FA8, D_L08_0015F660_2D5FA8);
                if (m->prev_seq != one) animate_2D5FA8(m, 1, 0, 3);
                m->state = dead;
                d->health = 6.0f;
                d->flags &= 0xFFFD;
            } else {
                if (D_0015EE84_2B94B0 == 8) {
                    int k = d->part_id;
                    if (k != 0 && t != 0 && t->moby != 0 && t->moby->oclass == 0x661) {
                        if (k == one) D_L08_0015F694_2D5FA8 = one;
                        if (k == 2) D_L08_0015F698_2D5FA8 = one;
                        if (k == 3) D_L08_0015F69C_2D5FA8 = one;
                        if (k == 4) D_L08_0015F6A0_2D5FA8 = one;
                        if (D_L08_0015F694_2D5FA8 != 0 && D_L08_0015F698_2D5FA8 != 0) {
                            unsigned char *flags = D_0013D50F_2D5FA8 + 1;
                            if (flags[0xD] == 0) {
                                flags[0xD] = one;
                                sound_2D5FA8(1, 0, 0);
                                voice_2D5FA8(0x53D6, -1);
                            }
                        }
                    }
                }
                qcopy(vec, &m->pos);
                vec[2] += 1.0f;
                explode_2D5FA8(m, 0, -1);
                event_2D5FA8(2, 0, m);
                burst_2D5FA8(m, d->f40, vec, 0.0f, 0.0f, 20, 8, 20, 4.0f, 2.0f, 9.0f, 1.0f, -1, 15.0f, 1, 1, -1, 0);
                spawn_2D5FA8(m, 0x6FE, (float *)&m->pos, (float *)&m->rot, 0, 0, 0.0f, D_L08_0015F660_2D5FA8, D_L08_0015F660_2D5FA8, D_L08_0015F660_2D5FA8);
                spawn_2D5FA8(m, 0x6FE, (float *)&m->pos, (float *)&m->rot, 0, 0, 0.0f, D_L08_0015F660_2D5FA8, D_L08_0015F660_2D5FA8, D_L08_0015F660_2D5FA8);
                spawn_2D5FA8(m, 0x6FF, (float *)&m->pos, (float *)&m->rot, 0, 0, 0.0f, D_L08_0015F660_2D5FA8, D_L08_0015F660_2D5FA8, D_L08_0015F660_2D5FA8);
                spawn_2D5FA8(m, 0x6FF, (float *)&m->pos, (float *)&m->rot, 0, 0, 0.0f, D_L08_0015F660_2D5FA8, D_L08_0015F660_2D5FA8, D_L08_0015F660_2D5FA8);
                spawn_2D5FA8(m, 0x6FF, (float *)&m->pos, (float *)&m->rot, 0, 0, 0.0f, D_L08_0015F660_2D5FA8, D_L08_0015F660_2D5FA8, D_L08_0015F660_2D5FA8);
                if (d->partA != 0) delete_2D5FA8(d->partA);
                if (d->partB != 0) delete_2D5FA8(d->partB);
                delete_2D5FA8(m);
                return;
            }
        }
        effects_start_2D5FA8(m, &d->anim0);
    }

    m->hit_slot = 0xFF;
    if (d->partA != 0) d->partA->hit_slot = 0xFF;
    if (d->partB != 0) d->partB->hit_slot = 0xFF;

    follow_2D5FA8(100.0f, m, d->target, 0, 0, D_L08_001B0FB0_2D5FA8[d->path]->list, D_L08_001B0FB0_2D5FA8[d->path]->count);
    if (d->fF0 == 0) d->fF0 = *(int *)(D_0013E633_2D5FA8 + 0x2E9D);
    if (d->mode != 2) {
        float yaw, len, pitch;
        subtract_2D5FA8(vec, d->target, (float *)&m->pos);
        vec[2] -= 3.0f;
        yaw = angle_2D5FA8(vec[0], vec[1]);
        len = length_2D5FA8(vec);
        pitch = -angle_2D5FA8(len, vec[2]);
        if (0.34906585f < pitch) {
            pitch = 0.34906585f;
        } else if (pitch < -0.7853982f) {
            pitch = -0.7853982f;
        }
        yaw = angle_delta_2D5FA8(yaw, m->rot.z);
        approach_2D5FA8(&d->yaw, yaw, &d->yaw_speed, D_0015EE70_2B94B0 * 12.566371f, D_0015EE70_2B94B0 * 12.566371f, D_0015EE6C_2D5FA8 * 9.424778f);
        approach_2D5FA8(&d->pitch, pitch, &d->pitch_speed, D_0015EE70_2B94B0 * 12.566371f, D_0015EE70_2B94B0 * 12.566371f, D_0015EE6C_2D5FA8 * 9.424778f);
    } else {
        approach_2D5FA8(&d->yaw, 0.0f, &d->yaw_speed, D_0015EE70_2B94B0 * 12.566371f, D_0015EE70_2B94B0 * 12.566371f, D_0015EE6C_2D5FA8 * 9.424778f);
        approach_2D5FA8(&d->pitch, 0.0f, &d->pitch_speed, D_0015EE70_2B94B0 * 12.566371f, D_0015EE70_2B94B0 * 12.566371f, D_0015EE6C_2D5FA8 * 9.424778f);
    }
    rotation_2D5FA8(vec, 1, d->pitch);
    rotation_2D5FA8(d->f110, 2, d->yaw);
    matmul_2D5FA8(d->f110, vec, d->f110);
    {
        short a0 = d->anim0;
        short a2 = d->anim2;
        effects_update_2D5FA8(m, &d->anim0);
        if (d->partA != 0) {
            d->anim0 = a0;
            d->anim2 = a2;
            effects_update_2D5FA8(d->partA, &d->anim0);
        }
        if (d->partB != 0) {
            d->anim0 = a0;
            d->anim2 = a2;
            effects_update_2D5FA8(d->partB, &d->anim0);
        }
    }
}
