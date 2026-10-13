/* Exact at every place, but the build refuses it: the switch table travels with the
   function ("needs .rodata at its place, which is not C"), so it stays assembly. From
   the wave that matched the rest of its file. */

void func_L11_004433D8(u8 *object) {
    D_L11_001B32E8[0] = 1;
    if (func_3006B8(D_L11_001BA8C0, &D_L11_001B1530) < 0.00999999978f) {
        u8 state = *(u8 *)(object + 0x20);
        if (state == 4) {
            func_32C9E0(object, 7, -1);
        } else if (state == 5) {
            func_32C9E0(object, 6, -1);
        }
        func_488268(1, 0);
        func_4881A8(&D_L11_001B1530);
        func_488200(&D_L11_001B1540);
    }
}
