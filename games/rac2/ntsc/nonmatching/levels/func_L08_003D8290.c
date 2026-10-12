typedef unsigned int ti_word __attribute__((mode(TI)));
extern int ref_L08_003D8290_00(void *, void *, int, int, int);
extern float D_L08_001BFF68;

float func_L08_003D8290(void *raw, float first, float second) {
    char scratch[0x20];
    ti_word *input = (ti_word *)raw;
    *(ti_word *)scratch = *input;
    *(ti_word *)(scratch + 0x10) = *input;
    *(float *)(scratch + 8) = second;
    *(float *)(scratch + 0x18) = first;
    if (ref_L08_003D8290_00(scratch + 0x10, scratch, 2, 0, 0)) return D_L08_001BFF68;
    return 0.0f;
}
