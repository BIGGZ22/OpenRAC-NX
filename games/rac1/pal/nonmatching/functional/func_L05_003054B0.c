/* functional: medium. Pulls position toward the nearest point of a path polyline (D_L05_001B0CB0[path], 16-byte nodes, count in word 0, node.w at +0x1C marks active segments) when it lies within reach in the xy plane; writes the pulled point to out and returns the smallest xy distance seen. */
extern char *D_L05_001B0CB0_54B0[] __asm__("D_L05_001B0CB0");
extern void func_001F9BF0_54B0(void *, void *, void *) __asm__("func_001F9BF0");
extern void func_001F9BD8_54B0(void *, void *, void *) __asm__("func_001F9BD8");
extern void func_L00_001FF4B0_54B0(void *, void *, float) __asm__("func_L00_001FF4B0");
extern void func_001F9CA0_54B0(void *, void *, void *) __asm__("func_001F9CA0");
extern float func_001F9B88_54B0(float) __asm__("func_001F9B88");
extern float func_001F9CB8_54B0(void *) __asm__("func_001F9CB8");
extern float func_001F9C78_54B0(void *, void *) __asm__("func_001F9C78");

f32 func_L05_003054B0(s32 path_index, void *position, void *out, f32 reach) {
    f32 segment[4] __attribute__((aligned(16)));
    f32 result[4] __attribute__((aligned(16)));
    f32 from_point[4] __attribute__((aligned(16)));
    f32 normal[4] __attribute__((aligned(16)));
    f32 projection[4] __attribute__((aligned(16)));
    f32 cross[4] __attribute__((aligned(16)));
    f32 offset[4] __attribute__((aligned(16)));
    f32 best;
    f32 separation;
    f32 along;
    f32 length;
    s32 changed;
    s32 i;
    char *path;

    changed = 0;
    best = 512.0f;
    qcopy(from_point, position);
    for (i = 0; i < *(s32 *)D_L05_001B0CB0_54B0[path_index] - 1; i++) {
        path = D_L05_001B0CB0_54B0[path_index];
        if (*(f32 *)(path + i * 16 + 0x1C) == 0.0f && *(f32 *)(path + (i + 1) * 16 + 0x1C) == 0.0f)
            continue;
        func_001F9BF0_54B0(segment, from_point, path + i * 16 + 0x10);
        segment[2] = 0.0f;
        func_001F9BF0_54B0(normal, path + i * 16 + 0x20, path + i * 16 + 0x10);
        normal[2] = 0.0f;
        func_L00_001FF4B0_54B0(projection, normal, 1.0f);
        func_001F9CA0_54B0(cross, segment, projection);
        separation = func_001F9B88_54B0(cross[2]);
        if (separation < best)
            best = separation;
        if (reach < separation)
            continue;
        length = func_001F9CB8_54B0(normal);
        along = func_001F9C78_54B0(segment, projection);
        if (length < along || along < 0.0f) {
            if (!(func_001F9CB8_54B0(segment) < reach))
                continue;
            changed = 1;
            func_L00_001FF4B0_54B0(result, segment, reach);
        } else {
            changed = 1;
            func_L00_001FF4B0_54B0(offset, projection, along);
            func_001F9BF0_54B0(result, segment, offset);
            func_L00_001FF4B0_54B0(result, result, reach);
            func_001F9BD8_54B0(result, result, offset);
        }
        func_001F9BD8_54B0(from_point, result, D_L05_001B0CB0_54B0[path_index] + i * 16 + 0x10);
        from_point[2] = ((f32 *)position)[2];
    }
    if (changed)
        qcopy(out, from_point);
    return best;
}
