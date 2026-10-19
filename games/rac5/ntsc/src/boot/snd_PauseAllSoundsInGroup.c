extern void snd_SendIOPCommandNoWait(int command, int length, void *data,
                                     int arg3, int arg4);

void snd_PauseAllSoundsInGroup(int group) {
    snd_SendIOPCommandNoWait(0x16, 4, &group, 0, 0);
}
