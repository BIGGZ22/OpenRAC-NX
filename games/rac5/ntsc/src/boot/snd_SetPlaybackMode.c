/* snd_SetPlaybackMode 0x01E94E00..0x01E94E30 */
extern void snd_SendIOPCommandNoWait(int command, int length, void *data,
                                     int arg3, int arg4);

void snd_SetPlaybackMode(int mode) {
    snd_SendIOPCommandNoWait(0xB, 4, &mode, 0, 0);
}
