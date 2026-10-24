void JetFighterUpdateL13(Jet47 *m) {
    JetData47 *d;
    JVec47 impulse;
    int state,initial_state;
    if (((JetPlayer47*)(D_0013E633+0xE1D))->state==0x72) return;
    d=m->data; d->hit_class=0;
    func_001F9908((int*)&d->timer);
    initial_state=m->state;
    if ((unsigned int)(initial_state-6)>=2 && (unsigned char)initial_state!=1 && (unsigned char)initial_state!=2) {
        JetHit47 *hit=(JetHit47*)MobyGetHitMessage(m,0x30001,0);
        if (hit && m->state==4) {
            if (((JetPlayer47*)(D_0013E633+0xE1D))->health>hit->damage) {
                ((JetPlayer47*)(D_0013E633+0xE1D))->health-=hit->damage;
                if (hit->source && (unsigned int)(hit->source->class_id-0x52)<2) d->hit_class=(short)hit->source->class_id;
                if (hit->flags&1) {
                    float weight,yaw_delta,pitch_delta;
                    jet_camera47.flash=1.0f; jet_camera47.ticks=scale_ticks(4);
                    weight=hit->damage*0.05f;
                    BuildSphericalOffset(impulse,d->speed,d->angles[2],-d->angles[1]);
                    func_L00_001FF548(hit->velocity,hit->velocity,D_0015EE6C*3.0f);
                    FastVecAdd(m->position,m->position,hit->velocity);
                    func_L00_001FF610(impulse,impulse,hit->velocity);
                    yaw_delta=FastSubRots(func_L00_001FF860(impulse[0],impulse[1]),d->angles[2]);
                    d->angles[2]=FastAddRots(d->angles[2],yaw_delta*(D_0015EE60*0.1f)*weight);
                    pitch_delta=FastSubRots(func_L00_001FF860(func_001F9CE8(impulse),impulse[2]),d->angles[1]);
                    d->angles[1]=FastAddRots(d->angles[1],pitch_delta*(D_0015EE60*0.1f)*weight);
                }
            } else {
                BuildSphericalOffset(impulse,d->speed,d->angles[2],-d->angles[1]);
                if (hit->source && (unsigned int)(hit->source->class_id-0x52)<2) d->hit_class=(short)hit->source->class_id+10000;
                jet_camera47.flash=0.2f; jet_camera47.ticks=scale_ticks(5);
                ((JetPlayer47*)(D_0013E633+0xE1D))->health=0.0f; m->flags|=0x41;
                func_L00_0025F4A8(m,impulse,0,0.0f,0.0f,10,3,16,4.0f,2.0f,9.0f,6,1.0f,15.0f,1,1,-1,0);
                m->animation=0; d->death_timer=scale_ticks(240); m->state=6;
            }
        }
    }
    m->status=0xFF;
    if (((JetPlayer47*)(D_0013E633+0xE1D))->craft==m) ((JetPlayer47*)(D_0013E633+0xE1D))->health_meter=(int)(((JetPlayer47*)(D_0013E633+0xE1D))->health*0.00390625f*200.0f);
    switch(m->state) {
    case 0: {
        float limit;
        int *palette;
        JetTexture47 *texture_table;
        unsigned int palette_data;
        int texture_index,initialized;
        m->state=1; d->animation_frame=0; m->scale=m->model->scale;
        *(float*)((char*)d+0x20)=d->angles[0]; *(float*)((char*)d+0x24)=d->angles[1];
        limit=func_L00_001FF860(1.0f,D_L13_0016CE70); limit+=limit;
        d->counterC0=0; d->hit_class=0; *(float*)((char*)d+0xAC)=limit; *(float*)((char*)d+0xB0)=limit;
        qcopy(d->angles,m->rotation); qcopy(d->spawn_rotation,m->rotation); qcopy(d->spawn_position,m->position);
        texture_index=d->texture_id;
        texture_table=D_L13_0016D500;
        d->blend=0.0f;
        texture_index+=40;
        palette_data=D_L13_0015F520;
        initialized=jet_palette_ready47;
        palette=(int*)(palette_data+(texture_table[texture_index].palette<<4));
        jet_palette47=palette;
        if (!initialized) __builtin_memcpy(D_L13_001CC0F0,palette,1024);
        jet_palette_ready47=1; d->wait_timer=1; d->energy=1;
        if (m->persistent!=0xFF && (D_0014171B+0xAA35)[m->persistent+(D_0015EE84<<4)]==0xFF) m->state=9;
        if (d->group_a>0) jet_group_state(d->group_a,0,0,0);
        if (d->group_b>0) jet_group_state(d->group_b,0,0,0);
        jet_group_switch(D_L13_001CC068,1,1,-1); jet_group_switch(&D_L13_001614D0,1,-1,-1); jet_group_flag(0x102,0);
        d->lock_index=-1; d->start_timer=scale_ticks(575);
        break;
    }
    case 1: {
        short enter=0;
        if (m->persistent!=0xFF && (D_0014171B+0xAA35)[m->persistent+(D_0015EE84<<4)]==0xFF) m->state=9;
        else if (d->trigger!=-1 && is_point_inside_clip_volume(D_0013E633+0xE9D,d->trigger)) { d->blend=0.99f; D_L13_0015F4FC=0.99f; enter=1; }
        else if (jet_distance(m,((JetPlayer47*)(D_0013E633+0xE1D))->moby)<2.0f && ((JetPlayer47*)(D_0013E633+0xE1D))->state!=50 && ((JetPlayer47*)(D_0013E633+0xE1D))->state!=29 && !((JetPlayer47*)(D_0013E633+0xE1D))->busy) {
            int pressed=try_set_help_message(7,0x543B)!=0;
            if (*(int*)(D_0013A5E0+0x2604)&0x10) enter=pressed;
        }
        if (enter) {
            float limit;
            HeroEnterMoveState(50,1); m->scale=m->model->scale;
            *(float*)((char*)d+0x20)=d->angles[0]; *(float*)((char*)d+0x24)=d->angles[1];
            limit=func_L00_001FF860(1.0f,D_L13_0016CE70); limit+=limit;
            d->counterC0=0; d->hit_class=0; *(float*)((char*)d+0xAC)=limit; *(float*)((char*)d+0xB0)=limit;
            qcopy(d->angles,m->rotation); d->wait_timer=1; d->energy=1; m->state=2; d->lock_index=-1;
        } else if (d->blend!=0.0f) { ApproachValue(&d->blend,0.0f,D_0015EE6C*4.0f); D_L13_0015F4FC=d->blend; }
        jet_animate((unsigned char*)m,1.5f);
        break;
    }
    case 2:
        ApproachValue(&d->blend,1.0f,D_0015EE6C*4.0f); D_L13_0015F4FC=d->blend;
        if (d->blend==1.0f) {
            if (d->entry_path!=-1) {
                JetPath47 *path=&D_L13_0016016C[d->entry_path];
                Hero_ResetToPosition(path->position,path->rotation,50,1);
                {
                    int index=d->entry_path;
                    JetPath47 *paths=D_L13_0016016C;
                    qcopy(m->position,paths[index].position); qcopy(m->rotation,paths[index].rotation);
                }
            }
            CameraScript(m->position,m->rotation,0,0,0);
            d->missile_index=0; d->counter6C=0; d->counter70=0; d->missile_timer=0; d->speed=D_0015EE60*0.1f; d->gun_timer=0;
            m->alpha=m->render_flags=0xFF; *(int*)((char*)((JetPlayer47*)(D_0013E633+0xE1D))->moby+0x98)=-1;
            qcopy(d->previous_rotation,m->rotation);
            d->angle_a=0; d->angle_b=0; d->angle_c=0; d->angle_d=0; d->angle_e=0; d->angle_f=0; ((JetPlayer47*)(D_0013E633+0xE1D))->craft=m; { unsigned short cls=m->class_id; ((JetPlayer47*)(D_0013E633+0xE1D))->health=255.0f; ((JetPlayer47*)(D_0013E633+0xE1D))->ammo=10; ((JetPlayer47*)(D_0013E633+0xE1D))->pad160E=1; ((JetPlayer47*)(D_0013E633+0xE1D))->ammo2=20; ((JetPlayer47*)(D_0013E633+0xE1D))->class_id=cls; ((JetPlayer47*)(D_0013E633+0xE1D))->ammo3=20; d->hud_previous=255; d->hud_health=0; ((JetPlayer47*)(D_0013E633+0xE1D))->ammo4=3; ((JetPlayer47*)(D_0013E633+0xE1D))->ammo5=3; ((JetPlayer47*)(D_0013E633+0xE1D))->ammo6=3; ((JetPlayer47*)(D_0013E633+0xE1D))->pad160F=0; ((JetPlayer47*)(D_0013E633+0xE1D))->max_health=256.0f; ((JetPlayer47*)(D_0013E633+0xE1D))->health_scale=100.0f; }
            BuildSphericalOffset(d,D_0015EE60*0.1f,d->angles[2],-d->angles[1]);
            if (d->group_a>0) jet_group_state(d->group_a,1,1,1);
            if (d->group_b>0) jet_group_state(d->group_b,1,1,1);
            jet_group_switch(D_L13_001CC068,0,0,-1); jet_group_switch(&D_L13_001614D0,0,-1,-1); jet_group_flag(0x102,0);
            m->position[2]+=3.0f; m->scale=m->model->scale*0.25f; ((JetPlayer47*)(D_0013E633+0xE1D))->in_craft=1;
            if (d->engine_sound==-1) d->engine_sound=Moby_PlaySound(0,4,(int)m);
            D_L13_0015F6E8=2; PlayerFighterFlight(m,d); jet_camera_mode(); MusicRequestTrack(2,4); m->state=4;
        }
        break;
    case 4: {
        unsigned char *audio;
        if (d->blend!=0.0f) { ApproachValue(&d->blend,0.0f,D_0015EE6C*4.0f); D_L13_0015F4FC=d->blend; }
        force_help_message(6,0); D_L13_0015F6E8=2;
        audio=D_0013E633+0x1D+d->engine_sound*0x70;
        if (*(Jet47**)(audio+0x88)!=m || !audio[0x74]) { d->engine_sound=-1; d->engine_sound=Moby_PlaySound(0,4,(int)m); }
        PlayerFighterFlight(m,d);
        if (((JetPlayer47*)(D_0013E633+0xE1D))->pad160F&1) m->state=5;
        if (d->energy<=0) { d->wait_timer=scale_ticks(1200); m->state=8; }
        break;
    }
    case 5:
        ((JetPlayer47*)(D_0013E633+0xE1D))->in_craft=0; *(int*)((char*)((JetPlayer47*)(D_0013E633+0xE1D))->moby+0x98)=0;
        HeroEnterMoveState(0,1); CameraScript2(0); Hud_SetBarValueById(((JetPlayer47*)(D_0013E633+0xE1D))->weapon,0);
        qcopy(m->position,d->spawn_position); qcopy(m->rotation,d->spawn_rotation);
        m->scale=m->model->scale; func_0020EEE8(m);
        if (d->group_a>0) jet_group_state(d->group_a,0,0,0);
        if (d->group_b>0) jet_group_state(d->group_b,0,0,0);
        jet_group_switch(D_L13_001CC068,1,1,-1); jet_group_switch(&D_L13_001614D0,1,-1,-1); jet_group_flag(0x102,0);
        if (d->exit_path!=-1) { JetPath47 *path=&D_L13_0016016C[d->exit_path]; Hero_ResetToPosition(path->position,path->rotation,0,1); }
        else Hero_ResetToPosition(d->spawn_position,D_L13_0016016C[d->destination_path].rotation,0,1);
        {
            int id=d->engine_sound;
            unsigned char *audio;
            if (id!=-1) { audio=D_0013E633+0x1D+id*0x70; if (*(Jet47**)(audio+0x88)==m && audio[0x74]) Sound_StopSlot(id); }
            d->engine_sound=-1;
            id=d->weapon_sound;
            if (id!=-1) { audio=D_0013E633+0x1D+id*0x70; if (*(Jet47**)(audio+0x88)==m && audio[0x74]) Sound_StopSlot(id); }
            d->weapon_sound=-1;
        }
        MusicRequestTrack(0,5); d->blend=0.99f; D_L13_0015F4FC=0.99f; m->state=1;
        break;
    case 6:
        D_L13_0015F6E8=2;
        {
            int id=d->engine_sound;
            unsigned char *audio;
            if (id!=-1) { audio=D_0013E633+0x1D+id*0x70; if (*(Jet47**)(audio+0x88)==m && audio[0x74]) Sound_StopSlot(id); }
            d->engine_sound=-1;
            id=d->weapon_sound;
            if (id!=-1) { audio=D_0013E633+0x1D+id*0x70; if (*(Jet47**)(audio+0x88)==m && audio[0x74]) Sound_StopSlot(id); }
            d->weapon_sound=-1;
        }
        m->state=7; break;
    case 7: {
        JetTransition47 *transition;
        D_L13_0015F6E8=2;
        if (FastDecTimer(&d->death_timer)) {
            transition=(JetTransition47*)(D_0014171B+0x100B5);
            if (!transition->pending) {
            int active=transition->active;
            if (active==-1) {
                Hero_OnKilled(); CameraScript2(0);
                qcopy(m->position,d->spawn_position); qcopy(m->rotation,d->spawn_rotation); func_0020EEE8(m);
                {
            int id=d->engine_sound;
            unsigned char *audio;
            if (id!=active) { audio=D_0013E633+0x1D+id*0x70; if (*(Jet47**)(audio+0x88)==m && audio[0x74]) Sound_StopSlot(id); }
            d->engine_sound=-1;
            id=d->weapon_sound;
            if (id!=-1) { audio=D_0013E633+0x1D+id*0x70; if (*(Jet47**)(audio+0x88)==m && audio[0x74]) Sound_StopSlot(id); }
            d->weapon_sound=-1;
        }
                m->state=0;
            }
        }
        }
        break;
    }
    case 8:
        ((JetPlayer47*)(D_0013E633+0xE1D))->in_craft=0; *(int*)((char*)((JetPlayer47*)(D_0013E633+0xE1D))->moby+0x98)=0; HeroEnterMoveState(0,1); CameraScript2(0);
        if (d->escape_path==-1) { STUB_printf(D_L13_001F4760); qcopy(m->position,d->spawn_position); qcopy(m->rotation,d->spawn_rotation); }
        else { int index=d->escape_path; JetPath47 *paths=D_L13_0016016C; qcopy(m->position,paths[index].position); qcopy(m->rotation,paths[index].rotation); m->position[2]+=0.57318f; }
        m->scale=m->model->scale; func_0020EEE8(m);
        if (d->destination_path==-1) STUB_printf(D_L13_001F4798);
        else {
            JetPath47 *path;
            if (d->group_a>0) jet_group_state(d->group_a,0,0,0);
            if (d->group_b>0) jet_group_state(d->group_b,0,0,0);
            jet_group_switch(D_L13_001CC068,1,1,-1); jet_group_switch(&D_L13_001614D0,1,-1,-1); jet_group_flag(0x102,0);
            path=&D_L13_0016016C[d->destination_path]; Hero_ResetToPosition(path->position,path->rotation,0,1);
        }
        if (m->persistent!=0xFF) jet_clear_persistent(m->persistent);
        {
            int id=d->engine_sound;
            unsigned char *audio;
            if (id!=-1) { audio=D_0013E633+0x1D+id*0x70; if (*(Jet47**)(audio+0x88)==m && audio[0x74]) Sound_StopSlot(id); }
            d->engine_sound=-1;
            id=d->weapon_sound;
            if (id!=-1) { audio=D_0013E633+0x1D+id*0x70; if (*(Jet47**)(audio+0x88)==m && audio[0x74]) Sound_StopSlot(id); }
            d->weapon_sound=-1;
        }
        jet_reset_camera(D_0013E633+0xE9D,D_0013E633+0xEAD); m->state=9;
        break;
    case 9: DeleteMoby(m); return;
    case 10: break;
    default: break;
    }
    if (m->position[0]<15.0f) m->position[0]=15.0f;
    if (m->position[0]>1008.0f) m->position[0]=1008.0f;
    if (m->position[1]<15.0f) m->position[1]=15.0f;
    if (m->position[1]>1008.0f) m->position[1]=1008.0f;
    if (m->position[2]<15.0f) m->position[2]=15.0f;
    if (m->position[2]>1008.0f) m->position[2]=1008.0f;
    state=m->state;
    if ((unsigned char)state==4) { qcopy_nc(D_0013E633+0xE9D,m->position); qcopy_nc(D_0013E633+0xEAD,d->angles); }
    if ((unsigned int)(state-3)<2 || (unsigned char)state==8) spawn_fighter_jet_joint_particles(m,(char*)d);
}
