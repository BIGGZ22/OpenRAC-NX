/* snd_SetSoundVolPan 0x01E95058..0x01E950C8 */
extern void snd_SendIOPCommandNoWait(int command, int length, void *data,
                                     int arg3, int arg4);

void snd_SetSoundVolPan(int sound, int volume, int pan) {
    int params[3];

    params[0] = sound;
    params[1] = volume;
    params[2] = pan;
    snd_SendIOPCommandNoWait(0x1B, 0xC, params, 0, 0);
}
