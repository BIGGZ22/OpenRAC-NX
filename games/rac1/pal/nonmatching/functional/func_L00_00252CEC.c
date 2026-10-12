/* functional: high. VU0 vf0 is fixed at (0,0,0,1), so vf1=(1,0,0,0), vf2=(0,1,0,0), vf3=vmr32(vf0)=(0,0,1,0) and the stored vf0=(0,0,0,1) are constants; written to the scratchpad quads at 0x70000000..0x70000030 as float bit patterns. */
void func_L00_00252CEC(void) {
    *(volatile int *)0x70000000 = 0x3F800000;
    *(volatile int *)0x70000004 = 0;
    *(volatile int *)0x70000008 = 0;
    *(volatile int *)0x7000000C = 0;
    *(volatile int *)0x70000010 = 0;
    *(volatile int *)0x70000014 = 0x3F800000;
    *(volatile int *)0x70000018 = 0;
    *(volatile int *)0x7000001C = 0;
    *(volatile int *)0x70000020 = 0;
    *(volatile int *)0x70000024 = 0;
    *(volatile int *)0x70000028 = 0x3F800000;
    *(volatile int *)0x7000002C = 0;
    *(volatile int *)0x70000030 = 0;
    *(volatile int *)0x70000034 = 0;
    *(volatile int *)0x70000038 = 0;
    *(volatile int *)0x7000003C = 0x3F800000;
}
