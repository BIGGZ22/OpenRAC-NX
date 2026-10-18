typedef struct {
    char pad[0x70];
    int playing;
} ATRACInstance;

int SND_IsATRACPlaying__FP13ATRACInstance(ATRACInstance *instance) {
    return instance->playing != 0;
}
