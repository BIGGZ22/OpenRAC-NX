extern int g_SoundsPaused;
extern void SND_PausePositionalSounds__Fb(int pause);
extern void SND_PauseAtrac__Fbi(int pause, int group);
extern void snd_PauseAllSoundsInGroup(unsigned int group);
extern int snd_FlushSoundCommands(void);
extern void DelayThread(unsigned int micros);

void SND_PauseSoundsImmediate__Fv(void) {
    if (g_SoundsPaused == 0) {
        SND_PausePositionalSounds__Fb(1);
        SND_PauseAtrac__Fbi(1, -1);
        g_SoundsPaused = 0;
    }
    snd_PauseAllSoundsInGroup(0xFFFFFFFF);
    while (snd_FlushSoundCommands() != 0) {
        DelayThread(0x1388);
    }
}
