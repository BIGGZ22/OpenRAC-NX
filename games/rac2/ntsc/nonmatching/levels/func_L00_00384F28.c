/* 0 of 27 places match. Scans a strided object array and calls a virtual method
   on the entries of one class, skipping two flag bytes and one reserved object.
   Retail at 00:00384F28, 276 bytes:
     +4  $v1 = *(int *)0x1AC0C0            (lui 0x1B, lw -0x3F40)
     +36 $s0 = *(int *)0x1B1EA0 ; $v0 = *(int *)0x1B1EA4
     +56 if ((unsigned)$s0 >= $v0) return
     +64 $s3 = *(short *)($v1 + 0xAA)      the reference class
     +72 constants: $s2 = 0xFE, $s1 = 0xFD, $s5 = 0x383B90 (a routine address), $s4 = 0x5D8
     loop (+88):
       +92  $v0 = *(int *)0x1B1EA4         (gp-relative site, -0x2F30/0x2EB4($gp))
       +96  if (*(short *)($s0 + 0xAA) != $s3) next
       +104 $v1 = *(u16 *)($s0 + 0xAA) ; $v0 = *(u8 *)($s0 + 0x20)
       +112 if ($v0 == 0xFE) next ; +120 if ($v0 == 0xFD) next
       +128 if ($s0 == *(int *)0x1AC0C0) next
       +136 call *(void **)($s0 + 0x64)($s0)
       +156 second arm: sign-extended class compare against 0x5D8, the same two flags,
            and a compare of the method pointer against 0x383B90 before the call
       next: $s0 += 0x100 ; +232 loop while $s0 < *(int *)0x1B1EA4
   Shape to try next: a while over `object += 0x100`, the two flag bytes as 0xFE/0xFD
   compares on a byte field at +0x20 (see func_L00_003AB618 in the same pool for the
   idiom), the class at +0xAA and the method pointer at +0x64. The four data addresses
   (0x1AC0C0, 0x1B1EA0, 0x1B1EA4) differ per level and are read both absolutely and
   gp-relative, so they take sda ref_ symbols - the same lever func_L00_0037F7E8 needed. */
