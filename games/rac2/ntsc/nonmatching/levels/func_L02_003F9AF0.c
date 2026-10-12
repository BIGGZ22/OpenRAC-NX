/* Exact at every place, but the build refuses it: the switch table travels with the
   function ("needs .rodata at its place, which is not C"), so it stays assembly. From
   the wave that matched the rest of its file. */

s32 func_L02_003F9AF0(u32 a0) {
    switch (a0) {
    case 0:
        return 0;
    case 1:
        return 1;
    case 2:
        return 2;
    case 3:
        return 3;
    case 4:
        return 4;
    case 5:
        return 5;
    case 6:
        return 6;
    case 7:
        return 7;
    case 8:
        return 8;
    case 9:
        return 9;
    case 10:
    case 11:
    case 12:
    case 13:
    default:
        return -1;
    }
}
