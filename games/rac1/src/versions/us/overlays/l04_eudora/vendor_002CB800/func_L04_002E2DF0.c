void suck_cannon_update(Moby *moby) {
    PortalVars_002E2DF0 *vars = (PortalVars_002E2DF0 *)moby->pvars;
    Exit_002E2DF0 *exit;

    moby->rot.z = add_rot_002E2DF0(moby->rot.z, D_0015EE6C_002E2DF0 * 1.5707964f);
    if (D_L04_0015F6A8_002E2DF0 == 2)
        moby->flags |= 0x41;
    else if (moby->flags & 1)
        moby->flags &= ~0x41;
    switch (moby->state) {
    case 0:
        func_L00_002D80A0_002E2DF0(moby);
        if (*(D_0013D5EB_002E2DF0 + 0xE)) {
            remove_moby_002E2DF0(moby);
            break;
        }
        moby->state = 1;
        moby->pos.z += 1.0f;
        break;
    case 1:
        func_L04_002E30E0_002E2DF0(moby);
        if (dist_002E2DF0(&moby->pos, D_0013F4D0_002E2DF0) < 3.0f) {
            moby->flags |= 0x41;
            func_L00_00299B68_002E2DF0(2);
            moby->state = 2;
        }
        break;
    case 2:
        if (D_L04_0015F6A8_002E2DF0 == 2)
            break;
        if (!*(D_0013D5CA_002E2DF0 + 7))
            func_L00_00264DB8_002E2DF0(0xFA7, -1);
        else
            func_L00_00264DB8_002E2DF0(0x53DF, -1);
        D_L04_0015F720_002E2DF0 = 0xB4;
        func_L00_002618D8_002E2DF0(9, 1);
        if (vars->exitMoby != -1) {
            exit = &portal_mobys_002E2DF0[vars->exitMoby];
            func_L00_00286128_002E2DF0(&exit->pos, &exit->rot);
        } else {
            func_001E9730_002E2DF0(D_L04_001DCA60_002E2DF0, *(s16 *)&moby->save_id);
        }
        moby->state = 3;
        func_0020BFC8_002E2DF0(0, -1);
        break;
    case 3:
        remove_moby_002E2DF0(moby);
        break;
    }
}
