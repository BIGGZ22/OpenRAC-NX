extern int g_SoundsPaused;
extern void SND_PausePositionalSounds__Fb(int pause);
extern void SND_PauseAtrac__Fbi(int pause, int group);
extern int snd_FlushSoundCommands(void);
extern void DelayThread(unsigned int micros);

void SND_ContinueSoundsImmediate__Fv(void) {
    if (g_SoundsPaused == 0) {
        SND_PausePositionalSounds__Fb(0);
        SND_PauseAtrac__Fbi(0, -1);
    }
    while (snd_FlushSoundCommands() != 0) {
        DelayThread(0x1388);
    }
}
