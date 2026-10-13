typedef unsigned int ti_word __attribute__((mode(TI)));
typedef struct { char pad0[0x10]; ti_word copy; char pad1[0x48]; char *state; } TaboraObject;
extern float func_L08_003D8290(void *, float, float);

void func_L08_003D82F0(void *raw) {
    TaboraObject *object = (TaboraObject *)raw;
    char *state = object->state;
    float value = *(float *)(state + 0x3E8);
    ti_word copy = object->copy;
    *(float *)(state + 0x434) = func_L08_003D8290(&copy, value + 3.0f, value - 20.0f);
}
