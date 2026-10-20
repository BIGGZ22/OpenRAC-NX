/* The catalogue row starts eight bytes early in every place (level 06_notak
   0x003E1040, one of the two places): +0x0 `addiu $sp,$sp,0x20` and +0x4 `nop`
   are the previous function's return delay slot and its alignment padding, not
   this body. The real entry is row+8 (0x003E1048), size 96:
     addiu $sp,$sp,-0x10 ; addiu $v1,$zero,0x18 ; sd $ra,0x0($sp)
     lw $v0,0x68($a0) ; lw $v0,0x0($v0) ; bne $v0,$v1,+0x1c
     addiu $v1,$v0,-0x1 ; addiu $a0,$zero,0x8 ; addiu $a1,$zero,0x1243
     jal func_366BD0 ; addiu $a2,$zero,0x1 ; b +0x2c ; ld $ra,0x0($sp)
     addiu $a0,$zero,0x8 ; lui $v0,0x2A ; sll $v1,$v1,2 ; addiu $v0,$v0,-0x3520
     addiu $a2,$zero,0x1 ; addu $v1,$v1,$v0 ; jal func_366BD0
     lw $a1,0x0($v1) ; ld $ra,0x0($sp) ; jr $ra ; addiu $sp,$sp,0x10
   i.e. a message-id switch: 0x18 picks the fixed id 0x1243, anything else indexes
   a 0x2A-based table at 0x003D3C0C by (state-1)*4 and passes that value. The row
   must be split in tools/levels.py (start +8, size 96) before any C can match it.
   The C body below is what a maintainer should check against the real entry:
     void f(char *a0) { s32 v = *(s32 *)(*(char **)(a0 + 0x68));
                        if (v == 0x18) m(8, 0x1243, 1);
                        else m(8, D_003D3C0C[v - 1], 1); }
   Not written this wave: a row that cannot match cannot be committed as C. */
