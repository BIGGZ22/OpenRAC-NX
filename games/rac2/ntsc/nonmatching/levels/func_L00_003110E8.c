/* func_L00_003110E8 - NON MATCHEE.
   The shared copy of the level's "rebuild the path mesh" helper.  The candidate is
   the exact size (336 bytes) and the whole body matches in shape - the prologue, the
   call to func_00282A88, the sum chain for out->f06, the five short stores, the
   guarded memcpy and the scatter loop - but eight words differ, all of them a
   register choice:

   - the two pointer temporaries are swapped: retail loads the table pointer into
     $v1 and the second pointer into $v0, this candidate loads the table pointer
     into $v0 and the second one into $v1.  That also moves the `addu` that
     forms the table address (retail `addu $v1,$v1,$a1`, candidate
     `addu $a1,$a1,$v0`) and the two `lh` that read the record's shorts
     (retail $v0/$a0, candidate $v0/$v1);
   - the memcpy's two arguments are in the other two registers (retail keeps the
     destination offset in $a3 and the source in $t0, this candidate keeps the
     source in $a3 and the offset in $t0).

   The source order of the two byte reads, the pointer types (`char *` against
   `int *`/`int **`), the way the table is indexed (`a0->f18[a1 + 18]` against
   byte arithmetic) and the declaration order of the temporaries were all tried
   and none of them moves the two pointer temporaries onto the retail registers.

   What the body does: it clears `((n0 + 3) * 8) & 0xFF0` bytes of the output
   with func_00282A88, writes five shorts derived from the record and from the two
   byte counts, copies `record->f06 * 16 - n3 * 8` bytes with func_00282AB0,
   scatters `n3` 64-bit slots from the record into the output through the byte
   table at `*(int *)(a0->f18 + 0x1C) + 4`, makes sure the slot at
   `out->f06 * 16` is not zero, and calls func_0011AEA0(0).

   Measured with `python3 tools/check.py func_L00_003110E8 --diff`: the size is
   exact and the first difference is at +0x30.
*/

extern void func_00282A88(void *buf, int size);

extern void func_00282AB0(void *target, void *base, int len);

extern int func_0011AEA0(int code);

/* The record the table entry points at: three shorts and an array of 64-bit slots. */
typedef struct L110E8Rec {
    char          pad00[0x06];
    short         f06;
    char          pad08[2];
    short         f0A;
    char          pad0C[2];
    short         f0E;
    unsigned long long slots[1];
} L110E8Rec;

/* What the function writes: five shorts and the same array of 64-bit slots. */
typedef struct L110E8Out {
    char          pad00[0x06];
    short         f06;
    short         f08;
    short         f0A;
    short         f0C;
    short         f0E;
    unsigned long long slots[1];
} L110E8Out;

/* The object the two table pointers come from. */
typedef struct L110E8Src {
    char          pad00[0x14];
    char         *f14;
    char         *f18;
} L110E8Src;

void func_L00_003110E8(L110E8Src *a0, int a1, int a2, L110E8Out *a3)
{
    char *p14 = a0->f14;
    unsigned char n3 = (unsigned char)a0->f18[8];
    unsigned char n0 = (unsigned char)p14[8];
    int *q = *(int **)(a0->f18 + a1 * 4 + 0x48);
    L110E8Rec *rec = (L110E8Rec *)(void *)(*(int *)((char *)q + a2 * 4 + 0x1C));
    unsigned short *uf = (unsigned short *)rec;
    unsigned short uhA;
    unsigned short uhE;
    L110E8Out *out = a3;
    short hA;
    short hE;
    char *msrc;
    int v;
    int len;

    func_00282A88(out, ((n0 + 3) * 8) & 0xFF0);

    hA = rec->f0A;
    hE = rec->f0E;
    out->f08 = (short)(n0 * 8);
    v = n0 + hA;
    v += hE;
    v += 1;
    out->f06 = (short)(v >> 1);
    uhA = uf[5];
    out->f0A = uhA;
    out->f0C = (short)((uhA + n0) * 8);
    uhE = uf[7];
    out->f0E = uhE;

    len = rec->f06 * 16 - n3 * 8;
    msrc = (char *)rec + n3 * 8 + 0x10;
    if (len != 0)
        func_00282AB0((char *)out + n0 * 8 + 0x10, msrc, len);

    {
        unsigned char *table = (unsigned char *)(*(int *)(a0->f18 + 0x1C)) + 4;
        int i;
        unsigned long long *src = (unsigned long long *)((char *)rec + 0x10);

        if (n3 != 0) {
            i = n3;

            while (i != 0) {
                *(unsigned long long *)((char *)out + table[0] * 8 + 0x10) = src[0];
                table++;
                src++;
                i--;
            }
        }
    }
    {
        unsigned long long *p = (unsigned long long *)((char *)out + out->f06 * 16);

        if (*p == 0)
            *p = 1;
    }
    func_0011AEA0(0);
}
