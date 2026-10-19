/* Candidate only: the full-program build and compiler profile are not qualified. */
extern volatile unsigned char positional_lock;

void SND_UnlockPositionalArray__Fv(void) {
    if (positional_lock) positional_lock = 0;
}
