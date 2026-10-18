extern volatile int display_vcount; /* 0x01EF57B4 */

int sceDisplayGetVcount__Fv(void) {
    return display_vcount;
}
