/* 0 of 1 places match - last line: 22_dobbo_orbit: 176 bytes, expected 160
   Candidate is four instructions over.  Arms the object's private block the first time, then
   asks the shared helper for the resident record and only fires the engine call when the
   record's two gate words are both clear, parking the block's mode word at 0x708 either way.
   Retail reaches the record pointer with a plain absolute `lui $v0,0x19 / lw $s0,-0x4958($v0)`
   (0x0018B6A8) where this unit's own declaration of that address is `sda`, so the body below
   reaches it under a second name marked `nosda` - that part is right.  The extra four
   instructions are in the two guards and the early returns, which retail folds into the
   epilogue with `ld $s0,0x0($sp)` in each `beql`/`bnel` delay slot (the annulled-slot idiom
   again, three times here).  Unblocking needs the guard chain written so this cc1 annuls the
   branches, or accepts the per-guard frame restore. */

typedef int s32;
typedef unsigned char u8;

extern s32 func_2F36F0(s32);
extern void func_2F9E20(s32, s32);
extern s32 D_L22_0018B6A8_nosda __asm__("D_L22_0018B6A8") __attribute__((nosda));

void func_L22_00415A48(s32 a0) {
    s32 s1 = *(s32 *)(a0 + 0x68);
    s32 s0;

    if (*(u8 *)(a0 + 0x20) == 0) {
        *(u8 *)(a0 + 0x20) = 1;
        *(u8 *)(a0 + 0x30) = 0xFF;
        *(s32 *)s1 = 0x708;
    }
    if (*(u8 *)(a0 + 0x20) != 1)
        return;
    s0 = D_L22_0018B6A8_nosda;
    if (func_2F36F0(s1) == 0)
        return;
    if (*(s32 *)(s0 + 8) != 0)
        return;
    if (*(s32 *)(s0 + 0xC) != 0)
        return;
    func_2F9E20(0x9FD, 0x54);
    *(s32 *)s1 = 0x708;
}
