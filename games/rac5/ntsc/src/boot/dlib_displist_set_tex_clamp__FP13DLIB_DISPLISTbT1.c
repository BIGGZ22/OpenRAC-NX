/* dlib_displist_set_tex_clamp__FP13DLIB_DISPLISTbT1 0x01E94128..0x01E94188 */
typedef struct {
    char pad0[4];
    unsigned char *cur;
} DLIB_DISPLIST;

extern void dlib_displist_direct_begin__FP13DLIB_DISPLIST(DLIB_DISPLIST *displist);
extern void dlib_displist_add_data_ul64__FP13DLIB_DISPLISTUlUl(DLIB_DISPLIST *displist,
                                                               unsigned long first,
                                                               unsigned long second);
extern void dlib_displist_direct_end__FP13DLIB_DISPLIST(DLIB_DISPLIST *displist);

void dlib_displist_set_tex_clamp__FP13DLIB_DISPLISTbT1(DLIB_DISPLIST *displist,
                                                       unsigned long mode,
                                                       unsigned long clamp) {
    dlib_displist_direct_begin__FP13DLIB_DISPLIST(displist);
    dlib_displist_add_data_ul64__FP13DLIB_DISPLISTUlUl(
        displist, (mode & 0xFF) | (clamp & 0xFF) << 2, 8);
    dlib_displist_direct_end__FP13DLIB_DISPLIST(displist);
}
