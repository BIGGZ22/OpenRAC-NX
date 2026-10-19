/* w11/b01 — HIT_fopen_internal__FPCcT0
 * Reconstructed from the retail disassembly.  Mode strings at 0x01F1F548:
 * "r+","w+","a+","r","w","a" (g_mode* aliases; callers pass "rb"); per-fd
 * flags at 0x01F7E638 (g_text) and 0x01F7E5F8 (g_eof).
 */
extern int strncasecmp(const char *a, const char *b, unsigned int n);
extern char *strchr(const char *s, int c);
extern int higIoOpenInternal__FPCciUib(const char *name, int mode,
                                       unsigned int flags, int blocking);

extern const char g_modeRT[];
extern const char g_modeWT[];
extern const char g_modeAT[];
extern const char g_modeR[];
extern const char g_modeW[];
extern const char g_modeA[];

extern unsigned char g_hitTextMode[];   /* 0x01F7E638 */
extern unsigned char g_hitBinMode[];    /* 0x01F7E5F8 */

int HIT_fopen_internal__FPCcT0(const char *filename, const char *mode) {
    int m = 1;
    int fd;

    if (!strncasecmp(mode, g_modeRT, 2)) {
        m = 3;
    } else if (!strncasecmp(mode, g_modeWT, 2)) {
        m = 0x603;
    } else if (!strncasecmp(mode, g_modeAT, 2)) {
        m = 0x303;
    } else if (!strncasecmp(mode, g_modeR, 1)) {
        m = 1;
    } else if (!strncasecmp(mode, g_modeW, 1)) {
        m = 0x602;
    } else if (!strncasecmp(mode, g_modeA, 1)) {
        m = 0x302;
    }
    fd = higIoOpenInternal__FPCciUib(filename, m, 0, 0);
    if (fd < 0)
        return 0;
    if (strchr(mode, 't')) {
        g_hitTextMode[fd] = 1;
    } else {
        g_hitTextMode[fd] = 0;
    }
    g_hitBinMode[fd] = 0;
    return fd + 1;
}
