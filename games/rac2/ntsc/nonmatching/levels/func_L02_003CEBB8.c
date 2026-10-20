/* from src/levels/shared/L02_003C43F0.c: not written this wave - decoded, no C attempt.
 * 184 bytes, 2 places. It is the 0x7FFF list walk the matched siblings use
 * (src/levels/01_oozla/L01_003CAFE0-g8.c, src/levels/04_barlow/L04_00422A00-g8.c):
 *     q = (u16 *)D_L02_001FC060[a0];        (table base 0x001FC060, index is the raw argument)
 *     base = *(u8 **)D_L02_001B359C;        (loaded with lui+lw, so nosda)
 *     for (;;) {
 *         e = (char *)(base + ((*q & 0x7FFF) << 8));
 *         if (e == 0) return 0;
 *         if (*(u8 *)(e + 0x20) == 0xD) return e;     (returns e in the class-test delay slot)
 *         ... continuation ...
 *     }
 * The gates before the walk are exact and cheap: (a0 & 0xFF) == 0xFF -> 0; the word at
 * 0x001B35B8 (= $gp+0x45C8) < a0 -> 0; q == 0 -> 0.
 * The wall is the continuation chain, which this cc1 duplicates: after the class test the
 * retail bytes test (a0 & 0xFF) == 0xFF again, then a register that is provably zero
 * (bnez on a register the prologue just cleared), then q == 0, then the two-part loop test
 * old value (s16)*(q-1) >= 0 (bgezl) OR q == *(u32*)&table[a0] (beql) - the second one
 * reloads table[index] through the same address register. Same shape in func_L02_003CD8C0
 * below. Whoever resumes should write the guards as one source-level condition and check
 * whether this cc1 reproduces the duplication, rather than transcribing the duplicated chain.
 */
