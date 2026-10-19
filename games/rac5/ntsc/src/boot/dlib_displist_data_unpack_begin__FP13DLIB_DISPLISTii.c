/* dlib_displist_data_unpack_begin__FP13DLIB_DISPLISTii 0x01E94280..0x01E942C8 */
typedef struct {
    unsigned long long lo, hi;
} __attribute__((packed)) QW;

typedef struct {
    char pad0[8];
    char *begin;
} DLIB_DISPLIST;

extern QW g_cntTag; /* 0x01FD3360 */

extern void dlib_displist_cnt_unpack_begin__FP13DLIB_DISPLISTii(
    DLIB_DISPLIST *displist, int high, int low);

void dlib_displist_data_unpack_begin__FP13DLIB_DISPLISTii(DLIB_DISPLIST *displist,
                                                          int high, int low) {
    *(QW *)displist->begin = g_cntTag;
    dlib_displist_cnt_unpack_begin__FP13DLIB_DISPLISTii(displist, high, low);
}
