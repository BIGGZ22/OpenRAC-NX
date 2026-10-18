typedef struct {
    char _bytes[0x58];
} SND_Positional;

extern SND_Positional gSounds[];
extern int g_SoundsPaused;
extern void SND_PausePositionalSound__FP14SND_Positionalb(SND_Positional *p, int paused);

void SND_PausePositionalSounds__Fb(int paused) {
    SND_Positional *p;
    SND_Positional *end;

    g_SoundsPaused = paused;
    p = gSounds;
    end = p + 64;
    do {
        SND_PausePositionalSound__FP14SND_Positionalb(p, paused);
        p++;
    } while ((int)p < (int)end);
}
