/* snd_PlayVAGStreamByLocEx_CB 0x01E959D0..0x01E95A40 */
extern void snd_SendIOPCommandNoWait(int command, int length, void *data,
                                     int arg3, long long arg4);

void snd_PlayVAGStreamByLocEx_CB(int a0, int a1, int a2, int a3, int a4, int a5,
                                 int a6, int a7, int a8, int a9, int a10,
                                 long long a11) {
    int params[8];

    params[0] = a0;
    params[1] = a1;
    params[2] = (a4 << 16) | (a2 & 0xFFFF);
    params[3] = (a5 << 16) | (a3 & 0xFFFF);
    params[4] = a6;
    params[5] = a7;
    params[6] = a8;
    params[7] = a9;
    snd_SendIOPCommandNoWait(0x2C, 0x20, params, a10, a11);
}
