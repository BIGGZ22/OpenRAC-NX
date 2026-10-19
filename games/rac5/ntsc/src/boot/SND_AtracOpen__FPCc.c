extern char snd_str_l_vag[];
extern char snd_str_r_vag[];

extern int strlen(const char *s);
extern char *strcpy(char *dst, const char *src);
extern char *strcat(char *dst, const char *src);
extern int SND_FindVag__FPCc(const char *name);

int SND_AtracOpen__FPCc(const char *name) {
    char buf[0x40];
    int len = strlen(name);
    int left;
    int right;

    strcpy(buf, name);
    strcat(buf, snd_str_l_vag);
    left = SND_FindVag__FPCc(buf);
    buf[len] = 0;
    strcat(buf, snd_str_r_vag);
    right = SND_FindVag__FPCc(buf);
    if (left == -1) {
        return 0;
    }
    if (right != -1) {
        return left | (right << 16);
    }
    return 0;
}
