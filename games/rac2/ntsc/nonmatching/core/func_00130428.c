/* NEAR MISS - candidate 680 bytes, retail's exact size (wave 4, 15 October 2026).

   func_0012C4C8 in the SDK's libmpeg (the reference-image resize half of
   _sequenceHeader): called once the sequence header has set the decoder's
   sizes, it recomputes the macroblock counts and, when the frame size actually
   changed, reserves the three frame buffers and re-initialises the reference
   images. Transcribed from the reconstruction Lombyte carries as
   src/assembly/sdk/library/sequenceheader.c (MIT), func_0012C4C8, with the
   decoder state reached through mp->sys at +0x40 as that source does.
   The 0x180 * width * height >> 8 product is the R5900's three-operand
   `mult rd,rs,rt` (rabbitizer prints two operands), which is why the source's
   left-to-right parenthesisation has to be kept.
   Profile: sdk -O2 -G2 -ffunction-sections (the frame is `addiu $sp,$sp,-0xF0`
   with `sd` in 16-byte slots; `sdk -O2 -g2 -gstabs` gives the same bytes).

   Wave 3 recorded 676 bytes, four short, with the residual in the frame-size
   check. Wave 4 found the four bytes: the `var_2_53 = mpeg + 0x528;` inside the
   inner `if` is dead (the `sp40 + 0x68` assignment below overwrites it before
   any use), and this cc1 was still registering its value. Removing that dead
   assignment gives retail's exact size and moves the first difference from
   +0xb8 to +0xb4. What is left is one seven-instruction region, and everything
   after it realigns byte for byte:

     retail : bne $s7,$v1,skip ; addiu $v0,$fp,0x528 ; lw $v0,0x4($a1)
              beq $s6,$v0,end  ; addiu $v0,$fp,0x528 ; sw $s6,0x4($a1) ...
     here   : bnel $s7,$v1,skip ; sw $s6,0x4($a1)   ; lw $v0,0x4($a1)
              beq $s6,$v0,end  ; ld $ra,0xE0($sp)    ; sw $s6,0x4($a1)
              addiu $v0,$fp,0x528 ...

   Two related choices, both compiler-side:

   - Rematerialisation. Retail computes `mpeg + 0x528` in BOTH branch delay
     slots, in $v0 - the register the height load then clobbers, which is why
     the fall-through path needs its own copy. This cc1 computes it once, in
     $a0 (a register nothing clobbers), leaves it live to the merge and stores
     it there; that costs one instruction fewer, and it is the four bytes wave 3
     was missing.
   - The shared epilogue: with the address no longer occupying the second delay
     slot, this cc1 sinks `ld $ra,0xE0($sp)` into it and the shared epilogue
     comes out one instruction short, cancelling the gain.

   Levers tried and measured in wave 4: the `__asm__` alias on one address (the
   lever the brief proposed for this wall) cannot apply here - the body holds no
   symbol address at all, every address is `$fp + const` with $fp holding the
   `mp->sys` pointer loaded from arg0+0x40, so there is no second name to give
   the same symbol; and no symbol means `mips_check_split`/`-mno-split-addresses`
   has nothing to bite on either. The retained candidate is the wave-3
   transcription with that one dead assignment dropped. */
extern void func_0012FB70(void *arg0);

extern int func_0012FB80(void *arg0, void *arg1, unsigned int arg2, unsigned int arg3);

extern void func_001306D0(void *arg0, void *arg1, void *arg2, void *arg3, void *arg4,
                          void *arg5, void *arg6, void *arg7, void *arg8, int arg9,
                          int arg10, int arg11, int arg12, int arg13);

extern void func_001302E0(void *arg0, int arg1, int arg2);

void func_00130428(void *arg0) {
    unsigned char *sp30;
    unsigned char *sp34;
    unsigned char *sp38;
    unsigned char *sp3C;
    unsigned char *sp40;
    unsigned char *var_2_53;
    int chroma_height;
    int height;
    int width;
    int temp_6_16;
    int var_2_40;
    unsigned int temp_16_77;
    int temp_22_48;
    int temp_23_51;
    int temp_18_75;
    unsigned char *temp_17_63;
    unsigned char *temp_19_67;
    unsigned char *temp_20_71;
    unsigned char *temp_21_73;
    unsigned char *mpeg;

    mpeg = *(unsigned char **)((unsigned char *)arg0 + 0x40);
    temp_6_16 = *(int *)(mpeg + 0x848);
    if (temp_6_16 == 0) {
        *(int *)(mpeg + 0x174) = 3;
        *(int *)(mpeg + 0x13C) = 1;
        *(int *)(mpeg + 0x140) = 1;
        *(int *)(mpeg + 0x188) = 1;
        *(int *)(mpeg + 0x17C) = 1;
        *(int *)(mpeg + 0x144) = 5;
    }
    *(int *)(mpeg + 0x12C) = (*(int *)(mpeg + 0x124) + 0xF) >> 4;
    if (temp_6_16 != 0) {
        if (*(int *)(mpeg + 0x13C) == 0) {
            var_2_40 = ((*(int *)(mpeg + 0x128) + 0x1F) >> 5) * 2;
        } else {
            goto block_6;
        }
    } else {
    block_6:
        var_2_40 = (*(int *)(mpeg + 0x128) + 0xF) >> 4;
    }
    *(int *)(mpeg + 0x130) = var_2_40;
    temp_22_48 = var_2_40 << 4;
    temp_23_51 = *(int *)(mpeg + 0x12C) << 4;
    if (temp_23_51 == *(int *)((unsigned char *)arg0 + 0x0)) {
        if (temp_22_48 == *(int *)((unsigned char *)arg0 + 0x4)) {
            return;
        }
    }
    sp40 = mpeg + 0x4C0;
    var_2_53 = sp40 + 0x68;
    *(int *)((unsigned char *)arg0 + 0x0) = temp_23_51;
    *(int *)((unsigned char *)arg0 + 0x4) = temp_22_48;
    temp_17_63 = mpeg + 0x108;
    sp30 = mpeg + 0x320;
    temp_19_67 = mpeg + 0x1E8;
    sp34 = mpeg + 0x388;
    temp_20_71 = mpeg + 0x250;
    temp_21_73 = mpeg + 0x2B8;
    sp38 = mpeg + 0x3F0;
    temp_16_77 = ((0x180U * (unsigned int)temp_23_51) * (unsigned int)temp_22_48) >> 8;
    sp3C = mpeg + 0x458;
    width = temp_23_51;
    height = temp_22_48;
    func_0012FB70(temp_17_63);
    *(int *)(mpeg + 0xFC) = func_0012FB80(mpeg, temp_17_63, temp_16_77, 0x40);
    *(int *)(mpeg + 0x100) = func_0012FB80(mpeg, temp_17_63, temp_16_77, 0x40);
    *(int *)(mpeg + 0x104) = func_0012FB80(mpeg, temp_17_63, temp_16_77, 0x40);
    func_001306D0(temp_19_67, temp_20_71, temp_21_73, sp30, sp34, sp38, sp3C, sp40, var_2_53,
                  *(int *)(mpeg + 0xFC), *(int *)(mpeg + 0x100), *(int *)(mpeg + 0x104), width,
                  height);
    func_001302E0(temp_19_67, width, height);
    func_001302E0(temp_20_71, width, height);
    func_001302E0(temp_21_73, width, height);
    temp_18_75 = height >> 1;
    chroma_height = temp_18_75;
    func_001302E0(sp30, width, chroma_height);
    func_001302E0(sp34, width, chroma_height);
    func_001302E0(sp38, width, chroma_height);
    func_001302E0(sp3C, width, chroma_height);
    func_001302E0(sp40, width, chroma_height);
    func_001302E0(var_2_53, width, chroma_height);
}
