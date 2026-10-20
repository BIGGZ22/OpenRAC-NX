/* 0 of 8 places match - last line: 03_endako: 164 bytes, expected 172
   Row lookup keyed on the sub-object: bail (0) when the 0x5E half of the state
   word equals the shared counter minus one, then read the 16-byte record at
   0x002D2AB0 + (state 0x62 half) * 16. A record kind of 0 or less gives 0; kind
   1 or 2 compares the second argument with the word at record + (state 0x64 half
   + 1) * 4; kind 3 (only when the 0x64 half is 0) compares it with the words at
   record+4 and record+8; anything else gives 0.
   Measured: the form below is 164 bytes, the flat `row` pointer form 156, against
   retail's 172. Retail keeps `a0` as the record byte offset and re-derives
   `a3 + a0` twice at the end (`addu $v0,$a3,$a0` then `addu $a0,$a3,$a0`), which
   is what the two spellings here fold away. Blocker: that rematerialised
   `record + offset` pair and the `blez`/`bnel` layout of the kind dispatch. */
typedef char *ptr;
typedef unsigned char u8;
typedef short s16;
typedef int s32;

extern s32 D_L03_001B0B50 __attribute__((sda));
extern s32 D_L03_002D2AB0[];

s32 func_L03_003E3750(char *a0, s32 a1)
{
    char *a2 = *(char **)(a0 + 0x68);
    char *row;
    s32 off;
    s32 kind;

    if (*(s16 *)(a2 + 0x5E) == D_L03_001B0B50 - 1)
        return 0;
    off = *(s16 *)(a2 + 0x62) * 16;
    row = (char *)D_L03_002D2AB0 + off;
    kind = *(s32 *)row;
    if (kind <= 0)
        return 0;
    if (kind < 3)
        return a1 == *(s32 *)((char *)row + (*(s16 *)(a2 + 0x64) + 1) * 4);
    if (kind == 3) {
        if (*(s16 *)(a2 + 0x64) != 0)
            return 0;
        if (a1 == *(s32 *)((char *)D_L03_002D2AB0 + off + 4))
            return 1;
        if (a1 == *(s32 *)((char *)D_L03_002D2AB0 + off + 8))
            return 1;
    }
    return 0;
}
