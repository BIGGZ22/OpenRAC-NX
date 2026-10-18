typedef struct {
    char pad0[0x7B];
    unsigned char field7B;
} ATRACInstance;

extern void SND_FadeATRACInstance__FP13ATRACInstanceff(ATRACInstance *instance,
                                                       float start, float duration);

void SND_FadeAndStopATRACInstance__FP13ATRACInstanceff(ATRACInstance *instance,
                                                       float start, float duration) {
    SND_FadeATRACInstance__FP13ATRACInstanceff(instance, start, duration);
    instance->field7B = 1;
}
