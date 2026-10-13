/* func_L00_003B0AB0, 108 bytes, 27 placements.
   Nearest: 26 of 27 instructions byte-identical and size exact (108).
   Last line of tools/check.py: "00_aranos_tutorial: instruction at +0x0 differs"
   Remaining difference: the byte index travels in $a3 and the loaded tag in $a2
   (retail: index in $a2, tag in $v0), which flips `bnel $v0,$a2` into
   `bnel $a2,$a3`.  Writing the comparison as `c == s->f17` instead moves the
   index to $a2 and the tag to $v0 (registers right) but flips the branch back
   to `bnel $a2,$v0`; the two spellings trade one difference for the other. */

typedef struct SUB_3B0AB0 {
    char pad0[0x17];
    unsigned char f17;
} SUB_3B0AB0;

typedef struct OBJ_3B0AB0 {
    long f0;
    char pad0[0x20 - 8];
    SUB_3B0AB0 s20;
} OBJ_3B0AB0;

extern char D_L00_0023BB78[];

extern void func_32FC10();

void func_L00_003B0AB0(int idx)
{
    unsigned char c = idx;
    char *base = D_L00_0023BB78;
    char *at = base + 0x10;
    char *slot = at + (unsigned char)idx * 0x1C;
    OBJ_3B0AB0 *obj = *(OBJ_3B0AB0 **)slot;

    if (obj != 0) {
        SUB_3B0AB0 *s = &obj->s20;

        if ((obj->f0 & 0x80FF) == 0x51)
            if (c == s->f17) {
                func_32FC10(obj, s, c);
                *(OBJ_3B0AB0 **)slot = 0;
            }
    }
}
