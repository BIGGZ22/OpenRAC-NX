void snow_plow_update(VehicleMoby_2CB800 *m) {
    VehicleVec_2CB800 old, delta;
    VehicleData_2CB800 *d=m->data;
    u8 state=m->state;
    VehicleSub_2CB800 *sub;
    VehicleTrigger_2CB800 *trigger;
    float zero; s32 izero;
    qcopy(&old,&m->pos);
    if (!d) return;
    sub=(VehicleSub_2CB800 *)d->motion;
    if (state==0) {
        if (sub->path_id!=-1) {
            s32 *path=D_L04_001B0930[sub->path_id];
            if (FastVecDist((char *)path+0x10,(char *)path+(*path<<4))<0.5f) --*path;
        } else {
            STUB_printf(D_L04_001DC110,m->cls,truncate_float_to_s32((float)((s32)((char *)m-(char *)MOBYS)>>8)));
        }
        m->shield=0x7F;
        m->alpha=0xFF;
        d->start_speed=d->speed;
        d->shield=m->shield;
    }
    izero=0; zero=*(float *)&izero;
    gunship_follow_path(m);
    if (m->cls!=0x1E4) {
        FastDecTimer(&d->phase_timer);
        switch (d->phase) {
        case 0: {
            s32 i;
            if (d->speed<d->start_speed) {
                d->speed+=D_0015EE70*20.0f;
                if (d->speed>d->start_speed) d->speed=d->start_speed;
            }
            for (i=0,trigger=d->paths;i<4;i++,trigger++) {
                if (d->phase_timer==0 && d->paths[i].id!=-1 && is_point_inside_clip_volume(&m->pos,d->paths[i].id)) {
                    qcopy(&d->start_pos,&m->pos);
                    d->selected=trigger->token;
                    d->phase=1;
                    d->start_angle=m->rot.f[2];
                    d->turn_velocity=0;
                    m->state=3;
                    d->moving_timer=scale_ticks(0x8CA0);
                    break;
                }
            }
            break;
        }
        case 1:
            MobySpringTurn(m,&d->turn_velocity,D_L04_0016016C[d->selected].angle,D_0015EE70*1.0471976f,D_0015EE70*3.1415927f,D_0015EE6C*3.1415927f);
            if (func_001F9D48(&m->pos,&D_L04_0016016C[d->selected].pos)<1.0f &&
                FastDiffRots(m->rot.f[2],D_L04_0016016C[d->selected].angle)<0.17453292f) {
                d->phase=2;
                d->phase_timer=scale_ticks(300);
            }
            FastVecSub(&delta,&D_L04_0016016C[d->selected].pos,&m->pos);
            func_L00_001FF4B0(&delta,&delta,D_0015EE6C*3.0f);
            FastVecAdd(&m->pos,&m->pos,&delta);
            break;
        case 2:
            if (d->phase_timer==0) { d->turn_velocity=zero; d->phase=3; }
            break;
        case 3: {
            float length,step;
            MobySpringTurn(m,&d->turn_velocity,d->start_angle,D_0015EE70*1.0471976f,D_0015EE70*3.1415927f,D_0015EE6C*3.1415927f);
            if (func_001F9D48(&m->pos,&d->start_pos)<1.0f && FastDiffRots(m->rot.f[2],d->start_angle)<0.034906585f) {
                d->phase=0;
                d->phase_timer=scale_ticks(600);
                d->speed=zero;
                d->turn_velocity=zero;
                d->moving_timer=0;
            }
            FastVecSub(&delta,&d->start_pos,&m->pos);
            length=FastVecLength(&delta);
            func_L00_001FF4B0(&delta,&delta,D_0015EE6C*3.0f>length ? length : D_0015EE6C*3.0f);
            FastVecAdd(&m->pos,&m->pos,&delta);
            break;
        }
        }
    }
    if (state==0) {
        switch ((s16)((u16)m->cls-0x1D2)) {
        case 0:case 14:case 19:case 20:case 28:case 29:case 32:case 89:
            AttachManipulator(m,0,d->matrix0);d->angle0=0;break;
        case 22:case 27:
            AttachManipulator(m,0,d->matrix0);d->angle0=0;
            AttachManipulator(m,1,d->matrix1);d->angle1=0;break;
        }
    } else {
        switch ((s16)((u16)m->cls-0x1D2)) {
        case 0:case 14:
            d->angle0=FastAddRots(d->angle0,D_0015EE6C*12.566371f);
            axis_rotation_2d48(d->matrix0+0x10,0,d->angle0);break;
        case 19:case 20:case 28:case 29:case 32:case 89:
            d->angle0=FastAddRots(d->angle0,D_0015EE6C*12.566371f);
            axis_rotation_2d48(d->matrix0+0x10,2,d->angle0);break;
        case 22:case 27:
            d->angle0=FastAddRots(d->angle0,D_0015EE6C*12.566371f);
            axis_rotation_2d48(d->matrix0+0x10,2,d->angle0);
            d->angle1=FastAddRots(d->angle1,D_0015EE6C*12.566371f);
            axis_rotation_2d48(d->matrix1+0x10,2,d->angle1);break;
        }
    }
    if (d->partner!=-1) {
        VehicleMoby_2CB800 *other=MOBYS+d->partner;
        other->shield=0x7F;
        switch (m->cls) {
        case 0x1E0:
            delta.quad=0;
            delta.f[0]=0.5f;delta.f[2]=-5.0f;
            func_001F9EC0(&delta,&delta,m->matrix);
            FastVecAdd(&other->pos,&m->pos,&delta);
            qcopy(&other->rot,&m->rot);
            other->rot.f[1]=FastAddRots(other->rot.f[1],1.5707964f);
            MobyAnimAdvance(other);func_L00_00251E30(other);func_0020EEE8(other);
            other->flags|=6;
            break;
        case 0x1EE:
            delta.quad=0;delta.f[2]=-2.0f;
            func_001F9EC0(&delta,&delta,m->matrix);
            FastVecAdd(&other->pos,&m->pos,&delta);
            qcopy(&other->rot,&m->rot);
            MobyAnimAdvance(other);func_L00_00251E30(other);func_0020EEE8(other);
            other->flags|=6;
            break;
        }
    }
    if (MobyGetHitMessage(m,0x210000,0)) {
        if (++D_0015EEF8>=10 && gSkillPoints.flag==0) {
            gSkillPoints.flag=1;
            PlayBankSound(1,0,0);ShowBanner(0x542D,-1);
        }
        switch ((s16)((u16)m->cls-0x1D2)) {
        case 19:case 20: {
            {
                VehicleQuery_2CB800 query;
                query.flags=0x810000;query.strength=1;query.owner=m;
                clear_u64_value(&query.vec);query.result=0;
                coll_sphere_mobys(&m->pos,4.0f,16,m,&query);
            }
        }
        case 0:case 14:case 22:case 24:case 28:case 29:case 32:case 89: {
            {
                VehicleArr_2CB800 none;
                delta.quad=0;delta.f[2]=0.75f;
                clear_u64_value(none);
                FastVecAdd(&delta,&delta,&m->pos);
                func_L00_0025F4A8(m,none,&delta,0.0f,0.0f,10,3,4,2,1,100000,3,-1,15,1,1,-1,0);
                if (m->cls==0x1F2 || m->cls==0x22B) Moby_PlaySound(0,0,(s32)m);
                else if (m->cls!=0x1E6) Moby_PlaySound(1,0,(s32)m);
                d->visibility_timer=scale_ticks(600);d->visibility_active=1;
            }
            break;
        }
        case 27: {
            {
                VehicleArr_2CB800 offset={0.0f,0.0f,0.75f};
                VehicleArr_2CB800 none;
                VehicleQuery_2CB800 query2;
                clear_u64_value(none);
                Moby_GetJointWorldPos(m,0,&delta);
                FastVecAdd(&delta,&delta,offset);
                SpawnExplosionFx(m,&delta,-1,1.0f,13.0f);
                Moby_GetJointWorldPos(m,1,&delta);
                FastVecAdd(&delta,&delta,offset);
                SpawnExplosionFx(m,&delta,-1,1.0f,13.0f);
                FastVecAdd(&delta,offset,&m->pos);
                func_L00_0025F4A8(m,none,&delta,0.0f,0.0f,20,5,4,2,1,100000,3,-1,15,1,1,-1,0);
                Moby_PlaySound(1,0,(s32)m);
                query2.flags=0x810000;query2.strength=2;query2.owner=m;
                clear_u64_value(&query2.vec);query2.result=0;
                coll_sphere_mobys(&m->pos,6.0f,16,m,&query2);
                d->visibility_timer=scale_ticks(600);d->visibility_active=1;
            }
            break;
        }
        }
        FastVecScale(&delta,m->matrix,D_0015EE60*0.075f);
        delta.f[2]+=D_0015EE60*0.08f;
        if (m->cls==0x1D2) {
            SpawnBreakFragment(m,0x63F,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            SpawnBreakFragment(m,0x641,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            SpawnBreakFragment(m,0x642,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
        }
        else if (m->cls==0x1E0) {
            SpawnBreakFragment(m,0x642,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            SpawnBreakFragment(m,0x643,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            SpawnBreakFragment(m,0x644,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
        }
        else if (m->cls==0x1E6) {
            s32 i;
            SpawnBreakFragment(m,0x645,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            for (i=0;i<6;i++) {
            SpawnBreakFragment(m,0x646,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            }
        }
        else if (m->cls==0x1EA) {
            SpawnBreakFragment(m,0x647,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            SpawnBreakFragment(m,0x648,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            SpawnBreakFragment(m,0x649,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
        }
        else if (m->cls==0x1EE) {
            SpawnBreakFragment(m,0x64D,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            SpawnBreakFragment(m,0x64E,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
        }
        else if (m->cls==0x1EF) {
            SpawnBreakFragment(m,0x64F,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            SpawnBreakFragment(m,0x650,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            SpawnBreakFragment(m,0x651,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            SpawnBreakFragment(m,0x658,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
        }
        else if (m->cls==0x1F2) {
            SpawnBreakFragment(m,0x652,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            SpawnBreakFragment(m,0x653,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            SpawnBreakFragment(m,0x654,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
        }
        else if (m->cls==0x1ED) {
            s32 j,k;
            SpawnBreakFragment(m,0x793,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            SpawnBreakFragment(m,0x794,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            for (j=0;j<3;j++) {
            SpawnBreakFragment(m,0x795,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            }
            for (k=0;k<3;k++) {
            SpawnBreakFragment(m,0x796,&m->pos,&m->rot,scale_ticks(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            }
        }
    }
    m->draw_alpha=0xFF;
    if (d->visibility_active) {
        if (func_001F9908(&d->visibility_timer) &&
            FastDiffRots(D_L04_00166E80.angle,func_L00_001FF860(m->pos.f[0]-D_L04_00166E80.x,m->pos.f[1]-D_L04_00166E80.y))>1.5707964f) {
            m->sort=*(s32 *)(m->metadata+0x10);
            m->flags&=0xFFBE;
            d->visibility_active=0;
            if (d->partner!=-1) { VehicleMoby_2CB800 *other=MOBYS+d->partner; other->flags&=0xFFBE; }
            m->flags|=0x1000;
        } else {
            m->sort=0;
            m->flags|=0x41;
            if (d->partner!=-1) { VehicleMoby_2CB800 *other=MOBYS+d->partner; other->flags|=0x41; }
            m->flags&=0xEFFF;
        }
    }
}
