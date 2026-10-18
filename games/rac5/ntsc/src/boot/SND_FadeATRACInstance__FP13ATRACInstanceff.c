typedef struct {
    char pad0[0x54];
    float field54;
    float field58;
    int field5C;
    float field60;
    int field64;
    float field68;
} ATRACInstance;

void SND_FadeATRACInstance__FP13ATRACInstanceff(ATRACInstance *instance,
                                                float start, float duration) {
    instance->field5C = 0;
    instance->field60 = start;
    instance->field54 = instance->field68;
    instance->field58 = duration;
}
