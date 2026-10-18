/* Candidate only: the full-program build and compiler profile are not qualified. */
typedef struct {
    int field00;
    char _pad04[0x74];
    unsigned char field78;
    char _pad79;
    unsigned char field7A;
    char _pad7B;
} ATRACInstance;

extern ATRACInstance g_AtracInstances[];
extern int SND_IsATRACPlaying__FP13ATRACInstance(ATRACInstance *p);

void SND_StopAllATRAC__Fv(void) {
    ATRACInstance *p;
    ATRACInstance *end;

    p = g_AtracInstances;
    end = p + 2;
    do {
        if (SND_IsATRACPlaying__FP13ATRACInstance(p)) {
            p->field7A = 1;
        }
        p++;
    } while (p < end);
}
