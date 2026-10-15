/* Removed from src/ after the full build reported it: the placement accepts it but the linked program differs from retail (wave 8). Kept as a starting point. */
extern s32 func_4171F8(s32);

extern void func_4172A8(s32, void *);

extern s32 func_417238(s32);

s32 func_L11_00440C90(u8 *obj, u8 *arg) {
    u8 *state = *(u8 **)(obj + 0x68);
    s32 *slots = (s32 *)(state + 0x120);
    s32 index = *(s32 *)(state + 0x18C);
    f32 one;
    if (slots[index] == 0) {
        return 0;
    }
    if (func_4171F8(slots[index]) == 0) {
        return 0;
    }
    switch (*(s16 *)(arg + 0xAA)) {
        case 0xD98:
            one = 1.0f;
            *(s32 *)(state + 0x100) = *(s32 *)(state + 0x100) + 1;
            break;
        case 0xDCF:
            one = 1.0f;
            *(s32 *)(state + 0x104) = *(s32 *)(state + 0x104) + 1;
            break;
        case 0xDF4:
            one = 1.0f;
            *(s32 *)(state + 0x108) = *(s32 *)(state + 0x108) + 1;
            break;
        case 0xDF7:
            one = 1.0f;
            *(s32 *)(state + 0x10C) = *(s32 *)(state + 0x10C) + 1;
            break;
        case 0xDF9:
            if (func_417238(slots[*(s32 *)(state + 0x18C)]) != 0) {
                goto tail;
            }
            one = 1.0f;
            *(s32 *)(state + 0x110) = *(s32 *)(state + 0x110) + 1;
            break;
        case 0xD37:
            one = 1.0f;
            *(s32 *)(state + 0x114) = *(s32 *)(state + 0x114) + 1;
            break;
        default:
            goto tail;
    }
    *(f32 *)(*(u8 **)(arg + 0x68) + 0x20) = one;
tail:
    func_4172A8(slots[*(s32 *)(state + 0x18C)], arg);
    return 1;
}
