extern void snd_SetSFXGlobalReg(int reg, signed char value);

void SND_SetGlobalSoundRegister__FUiScUi(unsigned int reg, signed char value,
                                         unsigned int unused) {
    snd_SetSFXGlobalReg(reg, value);
}
