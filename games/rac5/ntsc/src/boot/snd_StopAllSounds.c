extern void snd_SendIOPCommandNoWait(int command, int length, void *data,
                                     int arg3, int arg4);

void snd_StopAllSounds(void) {
    snd_SendIOPCommandNoWait(0x18, 0, 0, 0, 0);
}
