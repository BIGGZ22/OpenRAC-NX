/* Candidate only: the full-program build and compiler profile are not qualified. */
extern int SND_IsAllAtracPaused__Fv(void);
extern int SND_PauseAtrac__Fbi(int pause, int id);
extern void SND_UpdateATRAC__Fv(void);
extern void snd_FlushSoundCommands(void);
extern void DelayThread(int usec);
extern void snd_StreamSafeCheckCDIdle(int mode);

int SND_PauseAndSyncATRAC__Fi(int id) {
    if (SND_IsAllAtracPaused__Fv()) {
        return 1;
    }
    if (!SND_PauseAtrac__Fbi(1, id)) {
        return 0;
    }
    while (!SND_IsAllAtracPaused__Fv()) {
        SND_UpdateATRAC__Fv();
        snd_FlushSoundCommands();
        DelayThread(5000);
    }
    snd_StreamSafeCheckCDIdle(1);
    return 1;
}
