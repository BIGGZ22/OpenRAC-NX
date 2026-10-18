/* Boot-only match under the pinned compiler and complete load-image gate. */
extern int busy_state;

int MEMCARD_Busy__Fv(void) { return busy_state != 0; }
