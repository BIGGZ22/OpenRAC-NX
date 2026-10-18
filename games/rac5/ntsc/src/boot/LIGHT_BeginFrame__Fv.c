/* The active display list pointer lives at 0x01F78BC4 and holds a frame counter
   at +0x24; LIGHT keeps three scrubbed copies of that counter. */
extern void *display_list_current;
extern int light_frame_a; /* 0x01EF58AC */
extern int light_frame_b; /* 0x01EF58B0 */
extern int light_frame_c; /* 0x01EF58B4 */

struct DisplayListFrame {
    char pad00[0x24];
    int counter; /* 0x24 */
};

void LIGHT_BeginFrame__Fv(void) {
    struct DisplayListFrame *displist = display_list_current;
    int step = 0x28000;
    displist->counter = displist->counter - step;
    light_frame_a = displist->counter;
    light_frame_b = displist->counter;
    light_frame_c = step;
}
