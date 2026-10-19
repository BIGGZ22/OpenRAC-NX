/* Candidate only: the full-program build and compiler profile are not qualified. */
typedef struct {
    int field00;
    char _pad04[0x74];
    unsigned char field78;
    char _pad79;
    unsigned char field7A;
    char _pad7B;
} ATRACInstance;

extern int g_VagWadEntries;
extern ATRACInstance g_AtracInstances[];
extern int SND_AtracOpen__FPCc(const char *name);
extern int SND_IsATRACPlaying__FP13ATRACInstance(ATRACInstance *p);

int SND_IsATRACPlaying__FPCcPi(const char *name, int id) {
    ATRACInstance *p;
    ATRACInstance *end;

    if (!g_VagWadEntries) {
        return 0;
    }
    if (!id) {
        id = SND_AtracOpen__FPCc(name);
        if (!id) {
            return 0;
        }
    }
    p = g_AtracInstances;
    end = p + 2;
    do {
        if (SND_IsATRACPlaying__FP13ATRACInstance(p)) {
            if (p->field00 == id) {
                return 1;
            }
        }
        p++;
    } while (p < end);
    return 0;
}
