/* func_L23_0041B6D8 (540 bytes, 1 place: 23:0041B6D8) -- NEAR MISS, not integrated.
   This C compiles to 536 bytes (retail 540) and needs the -g8 sibling (the float D_L23_001B1564 is a
   $gp access); check.py reports 0 of 1 places ("536 bytes, expected 540").
   Everything else matches: the control flow, the four vector helpers, the 0.95f / 0.0f tests, the
   neg.s tail and the shared func_00283BE0 call. What differs:
   - the one 128-bit copy `C = M2[4..7]` (the second vector of the matrix filled by func_0029FC20):
     retail materialises both addresses, then copies through $v0 -- `addiu $s0,$sp,0x60 ;
     addiu $v1,$sp,0x80 ; lq $v0,0($v1) ; sq $v0,0($s0)` -- and keeps $s0 for the two calls that
     follow; this cc1 folds the offsets into the quad instructions (`lq $v0,0x80($sp)`,
     `sq $v0,0x60($sp)`), which is the four missing bytes. Retail's pair is also adjacent and
     right behind the call; here the scheduler splits it.
   - the call func_00282F20(A, A, 1.0f): retail sets `daddu $a1,$s3,$zero ; daddu $a0,$s3,$zero ;
     jal ; mov.s $f12,$f20` (the float in the slot), this cc1 `mov.s $f12,$f20 ; daddu $a1 ; jal ;
     daddu $a0` (five spellings of the call, a float-first alias prototype and a `float one` local
     measured: same bytes, or worse).
   A compiler that kept TImode memory addresses in plain registers for this unit would give the
   retail size (measured on a private cc1: 540 bytes, only the func_00282F20 argument order left). */

typedef int TI128 __attribute__((mode(TI)));

extern float D_L23_001B1564 __attribute__((sda));

extern void func_0029FC20(char *, int, char *);

extern void func_0029F978(char *, int, char *);

extern void func_00283898(void *, int, float);

extern float func_00283BE0(float, float);

extern void func_00282CF0(char *, char *, char *);

extern void func_00282CC0(char *, char *, char *);

extern void func_00282F20(char *, char *, float);

extern float func_00282DB0(char *, char *);

extern void func_00282DD8(char *, char *, char *);

extern void func_L23_00418650(char *, float);

void func_L23_0041B6D8(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    float M0[16];
    float B[4];
    float A[4];
    float C[4];
    float M2[16];
    float D[4];
    float E[4];
    float f;

    func_0029FC20(arg0, 0x22, (char *)M0);
    if (*(int *)(state + 0x604) != 0) {
        if (*(unsigned char *)(state + 0x431) == 0)
            func_0029F978(arg0, 0x22, state + 0x430);
        func_00283898(state + 0x440, 2, *(float *)(state + 0x570));
        *(float *)(state + 0x570) = func_00283BE0(*(float *)(state + 0x570), D_L23_001B1564 * 0.00029088825f);
    } else if (*(int *)(state + 0x60C) != -1) {
        func_00282CF0((char *)A, state + 0x3F0, state + 0x3E0);
        func_00282F20((char *)A, (char *)A, 10.0f);
        func_00282CC0((char *)B, (char *)A, arg0 + 0x10);
        func_L23_00418650((char *)B, 0.0f);
        func_00282CF0((char *)A, (char *)B, state + 0x3E0);
        func_00282F20((char *)A, (char *)A, 1.0f);
        func_0029FC20(arg0, *(int *)(state + 0x60C) + 1, (char *)M2);
        *(TI128 *)C = *(TI128 *)(M2 + 4);
        if (func_00282DB0((char *)C, (char *)A) < 0.95f) {
            func_00282DD8((char *)D, (char *)C, (char *)A);
            func_00282F20((char *)E, state + 0x410, 1.0f);
            if (0.0f < func_00282DB0((char *)D, (char *)E))
                f = D_L23_001B1564;
            else
                f = -D_L23_001B1564;
            if (*(unsigned char *)(state + 0x431) == 0)
                func_0029F978(arg0, 0x22, state + 0x430);
            func_00283898(state + 0x440, 2, *(float *)(state + 0x570));
            *(float *)(state + 0x570) = func_00283BE0(*(float *)(state + 0x570), -(f * 0.00029088825f));
        }
    }
}
