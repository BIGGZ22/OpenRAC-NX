extern int transition_bank;
extern void snd_UnloadBank(int bank);
extern int snd_FlushSoundCommands(void);
extern void DelayThread(int usec);

int GLOBALWAD_ReleaseTransitionBank__Fv(void) {
    if (transition_bank) {
        snd_UnloadBank(transition_bank);
        while (snd_FlushSoundCommands()) {
            DelayThread(800);
        }
        transition_bank = 0;
    }
    return 1;
}
