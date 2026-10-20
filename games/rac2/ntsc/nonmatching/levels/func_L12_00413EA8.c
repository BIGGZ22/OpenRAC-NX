/* 0 of 1 places match - last line: 12_todano: 120 bytes, expected 124
   Raises the measured flag on every record of the object's own table: walks
   D_L12_001FB960[index] to its terminator and, for each record whose 0x20 byte is
   not negative, sets the byte at (0x68-word)+0x2EA when the record's 0xAA half is
   0xF31. Same walk as func_L12_003E1AA8 (which matches) with this id and field.
   Retail rematerialises the table base (`lui $a0,0x1B; lw $a0,0x2E9C($a0)`) at
   the top of every iteration - the byte store inside the loop is why it cannot be
   hoisted - and keeps a `nop` after the two hoisted constants (`0xF31`, `1`).
   Writing `base = D_L12_001B2E9C_2;` before the loop gives 128 bytes (hoisted
   load); referring to the extern inside the loop gives 120: every instruction
   present, one word short of retail's 124 and the constants scheduled
   differently. Blocker: that missing scheduling `nop`. */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char s8;
typedef short s16;
typedef int s32;

extern u16 *D_L12_001FB960[];
extern s32 D_L12_001B2E9C_2;

void func_L12_00413EA8(s32 unused, s32 index) {
    s16 *p;
    s16 *q;

    if (index == 0xFF)
        return;
    q = (s16 *)D_L12_001FB960[index];
    if (q == 0)
        return;
    p = q;
    do {
        u32 v = (*(u16 *)p & 0x7FFF) << 8;

        if (*(s8 *)(D_L12_001B2E9C_2 + v + 0x20) >= 0) {
            if (*(s16 *)(D_L12_001B2E9C_2 + v + 0xAA) == 0xF31)
                *(u8 *)(*(u32 *)(D_L12_001B2E9C_2 + v + 0x68) + 0x2EA) = 1;
        }
    } while (*p++ >= 0);
}
