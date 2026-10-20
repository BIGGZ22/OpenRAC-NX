/* Not attempted: the body is MMI-family (the brief routes MMI to a near miss without
   budget). Retail at 00:00387BB0, 248 bytes, 27 places:
     prologue: frame 0xB0, saves $s1,$s2,$s3,$s4,$s0,$ra,$f20 (slots 0x70..0xA0)
     +40  lw  $v0,0x68($s2); jal func_37DBF8($v0->0x48)      ($s2 = a0)
     +60  lq  $v0,0x0($s4)   $s4 = a1, a 16-byte input
     +64  sq  $v0,0x0($s1)   $s1 = sp+0x30
     +68  lq  $v0,0x0($s1)
     +72  sq  $v0,0x0($sp)
     +76  .word 0x700014A9   opcode 0x1C (R5900 MMI group, rabbitizer: INVALID) - identify
     +80  f20 = 1.0f
     +88  $s0 = sp+0x10
     +92  sq  0x40($sp)
     +96  func_2EE220(sp+0x10, sp, sp+0x40)      (func_00282DD8)
     +124 func_2EE3D0(sp+0x10, sp+0x10, 1.0f)    (func_00282F20)
     +140 func_2EE220(sp+0x20, sp+0x10, sp)
     +152 func_31EBB8($s3->0x10, sp)
     +172 func_311580($s2, 0, sp+0x50)
     +188 func_2EE5B0(sp+0x30, $s4, $s2+0xC0)    (func_2EE5B0)
     +204 func_2EE108(sp+0x30, sp+0x30, sp+0x50) (func_00282CC0)
   The seven 16-byte stack slots hold vectors built by these calls; the two lq/sq pairs
   and the MMI word at +0x4C are what block a C transcription today. To unblock: identify
   the 0x1C-group instruction (a TI-mode expression may reach it) and re-derive the vector
   aliasing; no attempt is claimed. */
