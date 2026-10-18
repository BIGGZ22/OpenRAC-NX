typedef struct SND_Positional {
    int field00;
    char _pad04[0x04];
    unsigned int soundID;
    char _pad0C[0x04];
    struct SND_Positional *link;
} SND_Positional;

extern int SND_IsPlaying__FPC14SND_Positional(const SND_Positional *p);
extern void snd_StopSound(unsigned int soundID);

void SND_StopPositionalSound__FP14SND_Positionalb(SND_Positional *p, int flag) {
    if (!p) {
        return;
    }
    if (!SND_IsPlaying__FPC14SND_Positional(p)) {
        return;
    }
    if (p->link) {
        p->link->field00 = 0;
        p->link = 0;
    }
    if (p->soundID != 0xFFFFFFFFu) {
        snd_StopSound(p->soundID);
    } else {
        p->field00 = 6;
    }
}
