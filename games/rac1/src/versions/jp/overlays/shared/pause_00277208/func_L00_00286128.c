extern B_00284e50 D_J_L00_001BB410;
void jet_reset_camera(void *a0, void *a1)
{
    unsigned char *src;
    int i;
    int lvl;
    char *g;

    func_L00_001FF040((int)&D_J_L00_001BB410, &D_L00_001BA9B0, 0x404);
    lvl = D_0015EE84_m;
    for (src = D_L00_001BA9B0.f, i = 0; i < 0x7FF; i++, src++) {
        int v = *src;
        if ((unsigned char)v == 0) continue;
        if ((unsigned char)v != 1 && D_0014C150[lvl][(unsigned char)v - 2] != 0xFF) continue;
        D_J_L00_001BB410.f[i] = v;
        level_moby_killed[lvl][i >> 5] |= 1 << (i & 0x1F);
        D_L00_001BA860[i >> 5] |= 1 << (i & 0x1F);
    }
    g = (char *)D_L00_001BB5C0;
    *(int *)g = 1;
    qcopy(g + 0x10, a0);
    qcopy(g + 0x20, a1);
    {
        char *hero = D_0013F450;
        char *stat = D_0013E650;
        char *fb = D_001517D0;
        char *p = *(char **)(hero + 0x2080);
        signed char v = *(signed char *)(fb + 0x22);

        *(int *)(g + 0x30) = *(int *)((char *)p + 0x38);
        *(int *)(g + 0x34) = *(int *)((char *)p + 0x3C);
        *(int *)(g + 0x38) = *(int *)((char *)p + 0x80);
        *(int *)(g + 0x44) = *(unsigned char *)(hero + 0x20A4);
        *(int *)(g + 0x48) = (int)p;
        *(int *)(g + 0x4C) = *(short *)(hero + 0x22CC);
        *(int *)(g + 0x3C) = *(int *)(stat + 0x64);
        *(unsigned char *)(g + 0x40) = *(unsigned char *)(stat + 0x68);
        *(unsigned char *)(g + 0x41) = *(unsigned char *)(stat + 0x69);
        *(unsigned char *)(g + 0x42) = *(unsigned char *)(stat + 0x6A);
        if (v != -1) {
            *(int *)(g + 0xC54) = v;
        } else {
            *(int *)(g + 0xC54) = *(short *)(fb + 0x38);
        }
    }
}
