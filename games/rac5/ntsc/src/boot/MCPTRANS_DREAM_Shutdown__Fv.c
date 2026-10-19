extern unsigned char mcptrans_dream_active;
extern void *mcptrans_dream_sound;
extern void SND_StopPositionalSound__FP14SND_Positionalb(void *, int);

void MCPTRANS_DREAM_Shutdown__Fv(void) {
    if (mcptrans_dream_active != 0)
        mcptrans_dream_active = 0;
    if (mcptrans_dream_sound != 0) {
        SND_StopPositionalSound__FP14SND_Positionalb(mcptrans_dream_sound, 1);
        mcptrans_dream_sound = 0;
    }
}
