extern short D_J_L02_001620AC;
void SpawnTwoParticles(char *moby, void *arg) {
    float v0[4];
    float v1[4];
    float mat[4];
    float v2[4];
    float m[4];
    int i;
    FastVecSub(mat, moby + 0x10, arg);
    for (i = 0; i < 2; i++) {
        int a, b, c, d, e;
        char *p;
        Moby_GetJointWorldPos(moby, i, m);
        FastVecScale(v0, mat, *(float *)&D_L02_001620A4);
        FastVecScale(v1, mat, *(float *)&D_L02_001620A8);
        rand_vec(v2, 0.0f, *(float *)&D_L02_001620AC * D_0015EE6C);
        FastVecAdd(v1, v1, v2);
        v0[3] = random_float_between(*(float *)&D_L02_00162084, *(float *)&D_L02_00162088) / 1.5f;
        v1[3] = random_float_between(*(float *)&D_L02_0016208C, *(float *)&D_L02_00162090) / 1.5f;
        a = FastTweenColor(*(int *)&D_L02_001620B0, *(int *)&D_L02_001620B4, random_float_between(0.0f, 1.0f));
        b = FastTweenColor(*(int *)&D_L02_001620B8, *(int *)&D_J_L02_001620AC, random_float_between(0.0f, 1.0f));
        c = func_001FA898_r(func_001F9878((float)*(int *)&D_L02_00162074 * random_float_between(0.0f, 1.0f) + 1.0f));
        d = func_001FA898_r(func_001F9878((float)*(int *)&D_L02_00162078 * (random_float_between(-*(float *)&D_L02_00162080, *(float *)&D_L02_00162080) + 1.0f)));
        e = func_001FA898_r(func_001F9878((float)*(int *)&D_L02_0016207C * (random_float_between(-*(float *)&D_L02_00162080, *(float *)&D_L02_00162080) + 1.0f)));
        p = PartType02Spawn(m, v0, v1, a, b, c, d, e, -1);
        if (p != 0) {
            p[9] = func_001FA898_r(8.0f) + 0x60;
        }
    }
}
