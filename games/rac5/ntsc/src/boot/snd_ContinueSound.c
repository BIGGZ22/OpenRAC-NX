extern void snd_SendIOPCommandNoWait(int command, int length, void *data,
                                     int arg3, int arg4);

void snd_ContinueSound(int sound) {
    snd_SendIOPCommandNoWait(0x14, 4, &sound, 0, 0);
}
