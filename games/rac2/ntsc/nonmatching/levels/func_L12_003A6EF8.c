/* 0 of 1 places match - last line: 12_todano: instruction at +0x4 differs
   Hands the measured installer its own setup routine: unless the 0x21 byte is
   0xFF, copy the shared slot word into the 0x68 sub-object's first word and, when
   it changed, call func_2EA380 with the installer's entry. Retail opens with
   `daddu $a1,$a0,$zero` - the object lives in $a1 and the shared word in $v1,
   while $a0 is reused for the call's constant. Every spelling tried keeps the
   object in $a0 and the word in $a1 (the copy is gone), so the whole body shifts:
   the same 80 bytes with two registers swapped. Two spellings (a local copy, and
   re-reading the object for the store) give the same. Blocker: that argument
   copy - nothing in the body keeps the object live across the `lui $a0`. */
typedef unsigned char u8;
typedef int s32;

extern s32 D_L12_001B29C8 __attribute__((sda));
extern void func_2EA380(void *);
extern void func_L12_003A6E18(void);

void func_L12_003A6EF8(u8 *a0) {
    u8 *a1 = a0;
    s32 v;

    if (*(u8 *)(a1 + 0x21) == 0xFF)
        return;
    v = D_L12_001B29C8;
    if (v != 0) {
        s32 *p = *(s32 **)(*(u8 **)(a1 + 0x68) + 8);

        if (*p != v) {
            *p = v;
            func_2EA380((void *)func_L12_003A6E18);
        }
    }
}
