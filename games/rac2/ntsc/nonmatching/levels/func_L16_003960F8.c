/* 0 of 2 places match - last line: 16_snivelak: instruction at +0x0 differs
   Not C, no compile: `0x78620000` and `0x7FA20000` are MMI (lq/sq family), and the
   scan below them is built from `beql` pairs that put a load in the annulled slot.
   Two independent walls; ASM_FUNC candidate. */
