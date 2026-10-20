/* Near miss from src/levels/11_joba/L11_002DE9E0.c: the build refuses this body at its
   place — instruction at +0x8 differs. Kept as a starting point;
   not linked, not counted.
   Wave 9 (p1) re-measured it: 72 bytes, every instruction and every call matches, and
   one instruction sits one slot too late.  Retail copies the incoming pointer into $s1
   between the two callee-saved stores, this cc1 copies it after them:
     00340504  sd   $s1,0x18($sp)  |  sd   $s1,0x18($sp)
     00340508  daddu $s1,$a0,$zero |  sd   $s0,0x10($sp)
     0034050C  sd   $s0,0x10($sp)  |  daddu $s1,$a0,$zero
   The body is the twin of the matched func_L02_00333438
   (src/levels/shared/L02_002B5F50.c), which hands the shared helper its second
   argument; here the helper gets `a0 + 0x10` instead, so the two pointers live in
   $s1 and $s0 across the call.  Tried (all three give the same bytes): the plain
   form above, a local for `a0 + 0x10`, and a local for `a0` plus one for `a0 + 0x10`.
   What is left is a lever that puts the parameter copy between the two stores. */

/* Standalone declarations recovered from src/levels/11_joba/L11_002DE9E0.c. */
extern void func_00282CF0(void *, void *, void *);

extern void func_L02_00333480(char *, char *, void *);

void func_L11_00340500(char *a0, char *a1) {
    char buf[0x10];

    func_00282CF0(buf, a1, a0 + 0x10);
    func_L02_00333480(a0, a0 + 0x10, buf);
}
