/* Candidate only: the full-program build and compiler profile are not qualified. */
typedef struct {
    char _pad00[0x50];
    int field50;
    int field54;
} DLIB_DISPLIST_DB;

extern void dlib_displist_send_dma__FP13DLIB_DISPLISTi(int list, int wait);
extern void dlib_displist_clear__FP13DLIB_DISPLIST(int list);

void dlib_displist_db_send_dma__FP16DLIB_DISPLIST_DB(DLIB_DISPLIST_DB *db) {
    dlib_displist_send_dma__FP13DLIB_DISPLISTi(db->field54, 1);
    db->field50 = 1 - db->field50;
    db->field54 = (int)db + 0x28 * db->field50;
    dlib_displist_clear__FP13DLIB_DISPLIST(db->field54);
}
