/* Exact at every place, but the build refuses it: the switch table travels with the
   function ("needs .rodata at its place, which is not C"), so it stays assembly. From
   the wave that matched the rest of its file. */

s32 func_L02_003FE1B0(u32 a0) {
    switch (a0) {
    case 0:
        return 0x2F48;
    case 1:
        return 0x2F49;
    case 2:
        return 0x2F4A;
    case 3:
        return 0x2F4B;
    case 4:
        return 0x2F4C;
    case 5:
        return 0x2F4D;
    case 6:
        return 0x2F4E;
    case 7:
        return 0x2F4F;
    case 8:
        return 0x2F6E;
    case 9:
        return 0x2F50;
    case 10:
        return 0x2F51;
    case 11:
        return 0x3240;
    case 12:
        return 0x3241;
    case 13:
        return 0x3242;
    default:
        return 0x2F48;
    }
}
