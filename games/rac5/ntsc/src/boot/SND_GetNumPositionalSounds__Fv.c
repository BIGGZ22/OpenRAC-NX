extern volatile unsigned char positional_sounds_locked;

int SND_GetNumPositionalSounds__Fv(void) {
    return positional_sounds_locked ? 0x40 : 0;
}
