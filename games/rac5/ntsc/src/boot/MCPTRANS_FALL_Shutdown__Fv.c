extern unsigned char mcptrans_fall_active;
extern void *mcptrans_fall_sound;
extern void SND_StopPositionalSound__FP14SND_Positionalb(void *, int);

void MCPTRANS_FALL_Shutdown__Fv(void) {
    if (mcptrans_fall_active != 0)
        mcptrans_fall_active = 0;
    if (mcptrans_fall_sound != 0) {
        SND_StopPositionalSound__FP14SND_Positionalb(mcptrans_fall_sound, 1);
        mcptrans_fall_sound = 0;
    }
}
