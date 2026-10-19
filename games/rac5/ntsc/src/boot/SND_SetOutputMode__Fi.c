/* Candidate only: the full-program build and compiler profile are not qualified. */
extern int g_soundOutputMode;
extern void snd_SetPlaybackMode(int mode);

void SND_SetOutputMode__Fi(int mode) {
    if (g_soundOutputMode == mode) {
        return;
    }
    switch (mode) {
    case 0:
        snd_SetPlaybackMode(1);
        break;
    case 1:
        snd_SetPlaybackMode(2);
        break;
    case 2:
        snd_SetPlaybackMode(0);
        break;
    }
    g_soundOutputMode = mode;
}
