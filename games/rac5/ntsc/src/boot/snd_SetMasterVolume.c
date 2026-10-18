extern void snd_SendIOPCommandNoWait(int command, int length, void *data,
                                     int arg3, int arg4);

void snd_SetMasterVolume(int volume, int duration) {
    int args[2];
    args[0] = volume;
    args[1] = duration;
    snd_SendIOPCommandNoWait(9, 8, args, 0, 0);
}
