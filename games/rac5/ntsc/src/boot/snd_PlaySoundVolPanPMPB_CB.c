/* snd_PlaySoundVolPanPMPB_CB 0x01E94E70..0x01E94ED0 */
extern void snd_SendIOPCommandNoWait(int command, int length, void *data,
                                     int arg3, int arg4);

void snd_PlaySoundVolPanPMPB_CB(int sound, int volume, int pan, int pitch,
                                int pitchMod, int pitchBend, int pmpb,
                                int callback) {
    int params[6];

    params[0] = sound;
    params[1] = volume;
    params[2] = pan;
    params[3] = pitch;
    params[4] = pitchMod;
    params[5] = pitchBend;
    snd_SendIOPCommandNoWait(0x11, 0x18, params, pmpb, callback);
}
