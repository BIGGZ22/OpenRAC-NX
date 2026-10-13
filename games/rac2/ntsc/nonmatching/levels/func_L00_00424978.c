/* NEAR MISS - 3 instructions, not a match.
   Last check.py line: func_L00_00424978: 0 of 27 places match
   (00_aranos_tutorial: instruction at +0x38 differs)
   Everything from 0x4249C0 to the end is byte-identical; the only difference is the
   pair at 0x4249B0:
     mine:   bne  $v0,$a2, +0xC ; lui  $a0,%hi(D_L00_001B1844)   (delay slot)
     retail: bnel $v0,$a2, +0xC ; lh   $v0,0x84($v0)             (delay slot)
   The reorg pass fills the delay slot of the `if (v == a0) return 1;` branch from the
   target block: the retail target block starts with a load (so the branch must be
   annulled, bnel), ours starts with the lui of the table address (safe, so a plain bne
   is enough). Variants tried and rejected: raw pointer arithmetic with (i << 5), array
   indexing D_L00_001B1844[i], explicit index locals declared before the table pointer,
   and two separate key locals - all produce the same bne/lui pair.

   Signature evidence: argument is a pointer (sw $a2,0x240($a3) stores it into the owner
   block); the key of each object is *(unsigned char *)(object->owner + 28) where owner is
   the field at +28 of a 32-byte entry of the table at global 0x1B1844 (D_L00_001B1844).
   Returns -1 while *(short *)(D_L00_001B8E10 + 134) != 0, 1 when the object is placed,
   0 otherwise. */

/* Measured partial layout; original type and game role are unknown. */
typedef struct Rac2Native_428d1e2cc4aa1f21_Keyed {
    unsigned char unknown_00[28];
    char *owner_1c;
} Rac2Native_428d1e2cc4aa1f21_Keyed;

extern char *D_L00_001B8E10;

extern Rac2Native_428d1e2cc4aa1f21_Keyed *D_L00_001B1844;

int func_L00_00424978(char *a0)
{
    char *p = D_L00_001B8E10;
    char *q;
    char *v;

    if (*(short *)(p + 134) != 0) {
        return -1;
    }
    q = *(char **)(p + 112);
    v = *(char **)(q + 576);
    if (v == 0) {
        *(char **)(q + 576) = a0;
        return 1;
    }
    if (v == a0) {
        return 1;
    }
    if (*(unsigned char *)(D_L00_001B1844[*(short *)(v + 132)].owner_1c + 28) <
        *(unsigned char *)(D_L00_001B1844[*(short *)(a0 + 132)].owner_1c + 28)) {
        *(char **)(q + 576) = a0;
        return 1;
    }
    return 0;
}
