/* func_L00_003B5210 (180 bytes, 27 places) -- NEAR MISS, not integrated.  Needs the
   -G8 unit (the two angle constants are lwc1 off $gp, gp+0xA10/0xA14); the three
   absolute pointers (0x001B28B0, 0x001B28B8, 0x0023D9E0) stay lui+addiu, so their
   declarations carry no sda.
   Four 0x50-strided records: for each, when the first short of the 0x001B28B0
   array is non-zero, scales a rand by 0x001B28D8, converts it and hands the record
   to func_L00_003B52C8 with the two $gp angles and 0.2f.
   The candidate is 176 bytes against 180 and matches instruction for instruction
   including the source/destination mapping (s1 = the short array, s2 = the record,
   s3 = the second array) once the three pointers are declared r, p, q - the
   declaration order decides the s-registers here.  The missing word is the
   annulled slot: retail's `beql $v0,$zero` duplicates the p += 0x50 into its
   delay slot (executed only on the skipped path), this cc1 emits a plain `beqz`
   and hoists q++ into the slot instead; measured on three loop spellings
   (if-block then increments, comma increment clause, continue form). */
extern short D_L00_001B28B0[];
extern short D_L00_001B28B8[];
extern float D_L00_001B28D8;
extern char D_L00_0023D9E0[];
extern float D_L00_001AFA00 __attribute__((sda));
extern float D_L00_001AFA04 __attribute__((sda));
extern float func_00283CE0(int);
extern int func_00283CF0(float);
extern void func_L00_003B52C8(void *, int, int, int, float, float, float);

void func_L00_003B5210(void)
{
    short *r = D_L00_001B28B8;
    char *p = D_L00_0023D9E0;
    short *q = D_L00_001B28B0;
    int i;

    for (i = 3; i >= 0; i--) {
        if (*q != 0) {
            int v = func_00283CF0(func_00283CE0(*r) * D_L00_001B28D8);

            func_L00_003B52C8(p, v, v, 5, D_L00_001AFA00, D_L00_001AFA04, 0.2f);
        }
        p += 0x50;
        r++;
        q++;
    }
}
