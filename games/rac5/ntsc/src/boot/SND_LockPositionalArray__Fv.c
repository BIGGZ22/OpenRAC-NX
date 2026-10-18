extern volatile unsigned char positional_sounds_locked;

void SND_LockPositionalArray__Fv(void) {
    if (positional_sounds_locked == 0) positional_sounds_locked = 1;
}
