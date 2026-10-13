/* functional: medium. Moby (gate/door) test: finds mobys inside a sphere over its box, clamps each one's position into the box (half sizes at data+8/+10) in the moby's local frame and, if the clamped point still touches that moby, calls func_L00_0025AD38 on it with flags 0x10001. Same function as the NTSC decomp's FUN_L13_002ed158. */
typedef struct { float v[4]; } Vec_2EE590 __attribute__((aligned(16)));
extern void func_001FA1F8_2EE590(void *, void *) __asm__("func_001FA1F8");
extern void func_001FA4A0_2EE590(void *, void *) __asm__("func_001FA4A0");
extern void func_001F9EE8_2EE590(void *, void *, void *) __asm__("func_001F9EE8");
extern int func_L00_001F2BE8_2EE590(float, void *, int, void *, void *) __asm__("func_L00_001F2BE8");
extern void func_L00_0025AD38_2EE590(void *, void *, int, void *, float) __asm__("func_L00_0025AD38");
extern void *func_00115248_2EE590(void *, const void *, unsigned int) __asm__("func_00115248");
extern char D_L13_00178280_2EE590[] __asm__("D_L13_00178280");

void func_L13_002EE590(void *mobyp, void *datap) {
    char *moby = (char *)mobyp;
    char *data = (char *)datap;
    float matrix[16];
    Vec_2EE590 qv;
    float *query = qv.v;
    float candidate_pos[4];
    Vec_2EE590 lv;
    float *local_pos = lv.v;
    float rotation_matrix[16];
    Vec_2EE590 pv;
    float *probe = pv.v;
    void **candidates;
    int count, hit_count, i, j;
    char *candidate;
    short max_size;
    float radius, limit, tolerance, height_limit, height_tolerance;
    int *hits = (int *)D_L13_00178280_2EE590;

    func_001FA1F8_2EE590(matrix, moby + 0x40);
    max_size = (*(short *)(data + 10) < *(short *)(data + 8)) ? *(short *)(data + 8) : *(short *)(data + 10);
    radius = (float)max_size * 0.5f;
    qv = (Vec_2EE590){{0.0f, 0.0f, 0.0f, 0.0f}};
    query[2] = radius;
    func_001F9EC0(query, query, matrix);
    func_001F9BD8(query, query, moby + 0x10);
    count = func_L00_001F2BE8_2EE590(radius, query, 0x10, moby, 0);
    if (count == 0) return;
    candidates = (void **)__builtin_alloca(count * 4);
    func_00115248_2EE590(candidates, D_L13_00178280_2EE590, count * 4);
    for (i = 0; i < count; i++) {
        candidate = (char *)candidates[i];
        if (candidate == 0 || (unsigned char)candidate[0x20] == 0xFE || (unsigned char)candidate[0x20] == 0xFD)
            continue;
        func_001F9BF0(candidate_pos, candidate + 0x10, moby + 0x10);
        func_001FA4A0_2EE590(rotation_matrix, matrix);
        func_001F9EE8_2EE590(local_pos, candidate_pos, rotation_matrix);
        pv = lv;
        limit = (float)*(short *)(data + 8) * 0.5f;
        tolerance = limit + 0.7f;
        if (probe[0] < -tolerance)
            probe[0] = (float)(-*(short *)(data + 8)) * 0.5f;
        else if (tolerance < probe[0])
            probe[0] = limit;
        height_limit = (float)*(short *)(data + 10) * 0.5f;
        height_tolerance = height_limit + 1.5f;
        if (probe[2] < -height_tolerance)
            probe[2] = (float)(-*(short *)(data + 10)) * 0.5f;
        else if (height_tolerance < probe[2])
            probe[2] = height_limit;
        probe[1] = 0.0f;
        func_001F9EC0(probe, probe, matrix);
        func_001F9BD8(probe, probe, moby + 0x10);
        hit_count = func_L00_001F2BE8_2EE590(0.7f, probe, 0x10, moby, 0);
        for (j = 0; j < hit_count; j++) {
            if ((char *)hits[j] == candidate)
                func_L00_0025AD38_2EE590(candidate, moby, 0x10001, probe, 1.0f);
        }
    }
}
