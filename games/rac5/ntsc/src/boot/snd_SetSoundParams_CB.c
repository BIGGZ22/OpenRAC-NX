/* snd_SetSoundParams_CB 0x01E950E8..0x01E95130 */
extern void snd_SendIOPCommandNoWait(int command, int length, void *data,
                                     int arg3, int arg4);

void snd_SetSoundParams_CB(int sound, int a1, int a2, int a3, int a4, int a5,
                           int pmpb, int callback) {
    int params[6];

    params[0] = sound;
    params[1] = a1;
    params[2] = a2;
    params[3] = a3;
    params[4] = a4;
    params[5] = a5;
    snd_SendIOPCommandNoWait(0x21, 0x18, params, pmpb, callback);
}
