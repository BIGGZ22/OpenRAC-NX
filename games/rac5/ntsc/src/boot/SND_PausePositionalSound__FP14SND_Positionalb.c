/* Candidate only: the full-program build and compiler profile are not qualified. */
typedef struct SND_Positional {
    int field00;
    char _pad04[0x04];
    unsigned int soundID;
    char _pad0C[0x04];
    struct SND_Positional *link;
    char _pad14[0x08];
    unsigned char paused;
} SND_Positional;

extern int SND_IsPlaying__FPC14SND_Positional(const SND_Positional *p);
extern void snd_PauseSound(unsigned int soundID);
extern void snd_ContinueSound(unsigned int soundID);

void SND_PausePositionalSound__FP14SND_Positionalb(SND_Positional *p, int paused) {
    if (!SND_IsPlaying__FPC14SND_Positional(p)) {
        return;
    }
    p->paused = paused;
    if (p->soundID == 0xFFFFFFFFu) {
        return;
    }
    if (paused) {
        snd_PauseSound(p->soundID);
    } else {
        snd_ContinueSound(p->soundID);
    }
}
