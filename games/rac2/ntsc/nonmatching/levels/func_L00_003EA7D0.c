/* DRAFT w49 — best attempt, DOES NOT MATCH: 140 bytes expected, 140 bytes produced,
   a single instruction in the wrong place (see note).
   last check.py line: func_L00_003EA7D0: 0 of 27 places match
   00_aranos_tutorial: instruction at +0x34 differs

   The address 0x1B2BA0 must be a SYMBOL (the retail uses `addiu` for the low
   half, ori would be emitted for a literal constant). Every symbolic form
   tried (scalar or array, `char*` or `unsigned int`) drops the address into a
   separate register ($s0/$s1) and grows the frame to 144/148/152 bytes; the
   literal keeps 140 bytes but emits `ori` AND schedules `lui $a1,0x1B` one
   step too early in the FIRST call only (the second call is identical to the
   retail one). Remaining to solve: place the lui/addiu pair of the 2nd
   argument in the argument register with no intermediate register.

   The rest of the body (prologue, beql+delay slot, calls 1 and 3) is identical
   to the retail word for word. */
void func_L00_0037B508(unsigned int a, unsigned long b);

extern void func_L00_003EA8C8(char *a, unsigned int b, unsigned int c, float d);

void func_L00_003EA7D0(char *p)
{
    if (*(unsigned char *)(p + 0x31) == 0) {
        return;
    }
    func_L00_0037B508(0x47, 0x53001UL);
    func_L00_003EA8C8(p, (unsigned int)0x1B2BA0, 0x7F10307F, 0.25f);
    func_L00_003EA8C8(p, (unsigned int)0x1B2BA0, 0x3F08183F, 0.5f);
    func_L00_0037B508(0x47, 0x5360bUL);
}
