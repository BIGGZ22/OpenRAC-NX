/* Exact at every place, but the build refuses it: the switch table travels with the
   function ("needs .rodata at its place, which is not C"), so it stays assembly. From
   the wave that matched the rest of its file. */

s32 func_L02_003F9B70(u32 a0) {
    switch (a0) {
    case 0:
        return 0x31;
    case 1:
        return 0x32;
    case 2:
        return 0x33;
    case 3:
        return 0x34;
    case 4:
        return 0x35;
    case 5:
        return 0x36;
    case 6:
        return 0x37;
    case 7:
        return 0x38;
    case 8:
        return 0x39;
    case 9:
        return 0x3A;
    case 10:
        return 0x3B;
    case 11:
        return 0x91;
    case 12:
        return 0x92;
    case 13:
        return 0x93;
    default:
        return -1;
    }
}
