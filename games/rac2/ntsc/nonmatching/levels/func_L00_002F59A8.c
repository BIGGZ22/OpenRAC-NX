/* 0 of 27 places match - last line: 00_aranos_tutorial: 144 bytes, expected 140
   Steps the shared small-data counters: bail when the 0x1B1BA6 byte is 0, call
   func_2F4490(D_L00_001B1B88[i], D_L00_001B1B98[i]) with i = the 0x1B1BA7 byte,
   bail when it returns 0, then decrement 0x1B1BA6 and set
   0x1B1BA7 = (0x1B1BA7 + 1) % 7. Note both globals are reached two ways in the
   same body: 0x1B1BA6 and the index read come out absolute (`lui 0x1B; lbu`), while
   the incrementing read of 0x1B1BA7 comes out of $gp (`lbu $v1,0x2BB7($gp)`), so
   the gp copy needs its own name (see below). Retail saves only $ra and keeps the
   frame at 0x10 - nothing survives the call. Measured: keeping the index or the
   next value in a local grows the frame to 0x20 with two saved registers (144-148
   bytes, the copy in a register); reading everything after the call is 144 bytes,
   one word over. Blocker: the post-call reload/schedule of the two counters. */
typedef unsigned char u8;
typedef short s16;
typedef int s32;

extern s32 func_2F4490(s32, s32);
extern s16 D_L00_001B1B88[];
extern s16 D_L00_001B1B98[];
extern u8 D_L00_001B1BA6;
extern u8 D_L00_001B1BA7;
extern u8 D_L00_001B1BA7_gp __attribute__((sda));

void func_L00_002F59A8(void)
{
    if (D_L00_001B1BA6 == 0)
        return;
    if (func_2F4490(D_L00_001B1B88[D_L00_001B1BA7],
                    D_L00_001B1B98[D_L00_001B1BA7]) == 0)
        return;
    D_L00_001B1BA6 = D_L00_001B1BA6 - 1;
    D_L00_001B1BA7 = (D_L00_001B1BA7_gp + 1) % 7;
}
