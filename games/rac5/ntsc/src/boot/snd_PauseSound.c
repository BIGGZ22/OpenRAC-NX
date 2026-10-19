extern void snd_SendIOPCommandNoWait(int command, int length, void *data,
                                     int arg3, int arg4);

void snd_PauseSound(int sound) {
    snd_SendIOPCommandNoWait(0x13, 4, &sound, 0, 0);
}
