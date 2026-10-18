extern unsigned char system_frame_started; /* 0x01EF5714 */

void SYSTEM_BeginFrame__Fb(unsigned char mode) {
    (void)mode;
    system_frame_started = 1;
}
