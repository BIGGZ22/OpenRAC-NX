/* Candidate only: the full-program build and compiler profile are not qualified. */
typedef struct {
    unsigned int w[4];
} DLIB_GIFTAG;

extern DLIB_GIFTAG g_cntTag;
extern DLIB_GIFTAG g_dlib_tag_template;
extern void dlib_displist_cnt_giftag_begin__FP13DLIB_DISPLISTG11DLIB_GIFTAG(
    void *list, DLIB_GIFTAG *tag);

void dlib_displist_direct_begin__FP13DLIB_DISPLIST(int *list) {
    unsigned int *q;

    *(DLIB_GIFTAG *)list[2] = g_cntTag;
    q = (unsigned int *)list[1];
    q[3] = 0x50000000;
    q[0] = 0;
    q[1] = 0;
    q[2] = 0;
    list[4] = (int)&q[3];
    list[1] += 0x10;
    dlib_displist_cnt_giftag_begin__FP13DLIB_DISPLISTG11DLIB_GIFTAG(list,
                                                                    &g_dlib_tag_template);
}
