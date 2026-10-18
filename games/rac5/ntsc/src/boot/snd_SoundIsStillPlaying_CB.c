extern void snd_SendIOPCommandNoWait(int command, int length, void *data,
                                     int arg3, int arg4);

void snd_SoundIsStillPlaying_CB(int sound, int arg2, int arg3) {
    snd_SendIOPCommandNoWait(0x19, 4, &sound, arg2, arg3);
}
