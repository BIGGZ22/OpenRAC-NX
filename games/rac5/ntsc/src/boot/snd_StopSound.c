/* snd_StopSound 0x01E94ED0..0x01E94F00 */
extern void snd_SendIOPCommandNoWait(int command, int length, void *data,
                                     int arg3, int arg4);

void snd_StopSound(int sound) {
    snd_SendIOPCommandNoWait(0x15, 4, &sound, 0, 0);
}
