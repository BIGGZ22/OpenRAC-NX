/* Candidate only: the full-program build and compiler profile are not qualified. */
void SND_ResetSoundTimeStamps__FPUi(unsigned int *p) {
    unsigned int n;
    unsigned int i;

    if (*p != 0xAABBCCDDu) {
        return;
    }
    p++;
    n = *p;
    if ((int)n >= 0) {
        return;
    }
    p++;
    n &= 0x7FFFFFFFu;
    for (i = 0; i < n; i++) {
        unsigned int v = *p;
        if (v) {
            ((unsigned int *)v)[9] = 0;
        }
        p++;
    }
}
