/* The active display list pointer lives at 0x01F78BC4. */
extern void *display_list_current;
extern void dlib_displist_set_texture_state__FP13DLIB_DISPLISTiUlUlUlUl(
    void *displist, int mode, unsigned long a, unsigned long b,
    unsigned long c, unsigned long d);

void UTIL_SetupTextureState__Fv(void) {
    dlib_displist_set_texture_state__FP13DLIB_DISPLISTiUlUlUlUl(
        display_list_current, 1, 0, 1, 0x44, (unsigned long)0xFE00 << 23);
}
