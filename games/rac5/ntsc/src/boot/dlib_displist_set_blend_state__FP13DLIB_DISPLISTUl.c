/* Candidate only: the full-program build and compiler profile are not qualified. */
extern void dlib_displist_direct_begin__FP13DLIB_DISPLIST(void *list);
extern void dlib_displist_add_data_ul64__FP13DLIB_DISPLISTUlUl(void *list,
                                                              unsigned long value,
                                                              unsigned long attr);
extern void dlib_displist_direct_end__FP13DLIB_DISPLIST(void *list);

void dlib_displist_set_blend_state__FP13DLIB_DISPLISTUl(void *list,
                                                        unsigned long state) {
    dlib_displist_direct_begin__FP13DLIB_DISPLIST(list);
    dlib_displist_add_data_ul64__FP13DLIB_DISPLISTUlUl(list, state, 0x42);
    dlib_displist_direct_end__FP13DLIB_DISPLIST(list);
}
