extern unsigned char wide_screen_from_memcard;

int MEMCARD_GetWideScreenFromMemcard__Fv(void) {
    return wide_screen_from_memcard & 1;
}
