/* func_L00_002C3A40 (56 bytes, 27 places) -- NEAR MISS, not integrated.
   state: the whole function is `s0 = 0xC` before the call, `if (state == 2) s0 = 0`,
   `return s0`, but this cc1 if-converts the conditional assignment: every shape below
   compiles to 40-44 bytes with `xori/movz` instead of retail's
     002C3A4C  jal  func_2C3A78
     002C3A50  addiu $s0,$zero,0xC     <- in the jal delay slot
     002C3A58  addiu $v0,$zero,0x2
     002C3A5C  beql $v1,$v0, . + 4 + (0x1 << 2)
     002C3A60  daddu $s0,$zero,$zero   <- the conditional assignment in the slot
     002C3A64  daddu $v0,$s0,$zero
   Six shapes tried in one call, all 40 or 44 bytes: `int result = 0xC;` before the call
   with `if (func() == 2) result = 0;`, the if/else form, the ternary, the early-return
   form, a switch on the state, and the state latched into a local first. The SDK 2.9-ee
   profile (no movz patterns) is the obvious next idea but no level file uses it and the
   compiler is not provisioned in this worker's volume.
   Retail keeps the value in a callee-saved register across the call, which is what the
   source must force; `movz` means cc1 proved the constant rematerialisable. */
