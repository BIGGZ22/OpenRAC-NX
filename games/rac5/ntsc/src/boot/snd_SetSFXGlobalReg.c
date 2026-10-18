extern void snd_SendIOPCommandNoWait(int command, int length, void *data,
                                     int arg3, int arg4);

void snd_SetSFXGlobalReg(int reg, signed char value) {
    int args[2];
    args[0] = reg;
    args[1] = value;
    snd_SendIOPCommandNoWait(0x67, 8, args, 0, 0);
}
