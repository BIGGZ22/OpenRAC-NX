/* func_L23_00418160 (652 bytes, 1 place: 23:00418160) -- NEAR MISS, not integrated.
   This C compiles to 632 bytes (retail 652) and needs the -g8 sibling (seven $gp words:
   D_L23_001B164C, 163C, 1640, 15A8, 15AC, 15B0, 15B4); check.py reports 0 of 1 places
   ("632 bytes, expected 652").
   The body is mapped: the state at +0x68, the object table entry chosen by the counter in
   D_L23_001B164C, the 0xC/0x10 type test selecting D_L23_001B1640 or 163C, the vector chain
   (func_0029FCC0 -> D_L23_001B3800, func_0029FC20 -> a -16.0f scaled helper, func_00418650 on a
   copy, the 2.2f scale), the three 128-bit copies of the sub-object at +0xC0/+0xD0/+0xE0 (skipped
   for type 2), the func_L23_004256F0 call with its 16 arguments (the 0x2F00/1/-1/1 words on the
   stack, 1.7f/1.9f/0.5f/D_L23_001B15A8 in $f12..$f15) stored to D_L23_001B37F0 when it was zero,
   func_002A7750(obj, 1, 0), func_002E5CF8(4, 0, arg0) and the counter update `(n + 1) % 25`.
   What differs:
   - every one of the six 128-bit copies: retail materialises both addresses and copies through $v0
     (`addiu $a0,$s5,0xC0 ; addiu $v1,$s3,0xC0 ; lq $v0,0($v1) ; sq $v0,0($a0)`, and for the
     third one `lq $v0,0($a2)` with $a2 = arg0+0xE0 also the second argument of the later
     func_00282D30, and for the fourth `sq $v0,0($s6)` with $s6 = obj+0x10 also the third argument of
     the later func_00282CC0); this cc1 folds the offsets into lq/sq, which is most of the
     twenty missing bytes;
   - the callee-saved assignment follows from that: retail keeps arg0 in $s3, the state in $s2, the
     object in $s5, &A in $s4, &D_L23_001B3800 in $s7, obj+0x10 in $s6 (set in the delay slot of
     the func_00418650 call), &C in $s0, &D in $s1; here arg0 and the state are $s4/$s3.
   A compiler that keeps TImode quad addresses in registers would give the retail size (measured on
   a private cc1: 652 bytes). */

typedef int TI128 __attribute__((mode(TI)));

extern float D_L23_001B15B4 __attribute__((sda));

extern TI128 D_L23_001B3800 __attribute__((sda));

extern char D_L23_001B3810[16] __attribute__((sda));

extern int D_L23_001B37F0;

extern char *D_L23_00259560[];

extern void func_0029FCC0(char *, int, char *);

extern void func_0029FC20(char *, int, char *);

extern void func_00282D30(char *, char *, float);

extern void func_00282CC0(char *, char *, char *);

extern void func_00282CF0(char *, char *, char *);

extern int D_L23_001B164C __attribute__((sda));

extern int D_L23_001B163C __attribute__((sda));

extern int D_L23_001B1640 __attribute__((sda));

extern float D_L23_001B15A8 __attribute__((sda));

extern short D_L23_001B15AC __attribute__((sda));

extern short D_L23_001B15B0 __attribute__((sda));

extern void func_L23_00418650(char *, float);

extern void func_002A7750(char *, int, int);

extern int func_002E5CF8(int, int, char *);

extern void *func_L23_004256F0(char *obj, char *a1, char *a2, unsigned char a3, unsigned char a4, short a5,
                               short a6, int a7, float f1, float f2, float f3, float f4, int a9, int a10,
                               int a11, int a12);

void func_L23_00418160(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char *obj = D_L23_00259560[D_L23_001B164C];
    char *p16 = obj + 0x10;
    float B[4];
    float A[4];
    float M[16];
    float D[4];
    float C[4];
    int type;

    type = *(unsigned char *)(arg0 + 0x20);
    if (type == 0xC || type == 0x10)
        *(int *)(state + 0x598) = D_L23_001B1640;
    else
        *(int *)(state + 0x598) = D_L23_001B163C;
    *(int *)(state + 0x59C) = (int)obj;
    *(int *)(state + 0x590) = *(int *)(state + 0x590) - 1;
    func_0029FCC0(arg0, *(int *)(state + 0x594) + 0x1E, (char *)A);
    D_L23_001B3800 = *(TI128 *)A;
    func_0029FC20(arg0, *(int *)(state + 0x594) + 0x1E, (char *)M);
    func_00282D30((char *)B, (char *)(M + 8), -16.0f);
    *(TI128 *)C = *(TI128 *)A;
    func_L23_00418650((char *)C, 0.0f);
    func_00282CF0((char *)D, (char *)C, state + 0x3E0);
    func_00282D30((char *)D, (char *)D, 2.2f);
    func_00282CC0((char *)B, (char *)B, (char *)D);
    func_00282CC0((char *)A, (char *)A, (char *)B);
    if (*(unsigned char *)(obj + 0x20) != 2) {
        *(TI128 *)(obj + 0xC0) = *(TI128 *)(arg0 + 0xC0);
        *(TI128 *)(obj + 0xD0) = *(TI128 *)(arg0 + 0xD0);
        *(TI128 *)(obj + 0xE0) = *(TI128 *)(arg0 + 0xE0);
        *(TI128 *)p16 = *(TI128 *)A;
    }
    func_00282D30(D_L23_001B3810, arg0 + 0xE0, D_L23_001B15B4);
    func_00282CC0(D_L23_001B3810, D_L23_001B3810, p16);
    if (D_L23_001B37F0 == 0)
        D_L23_001B37F0 = (int)func_L23_004256F0((char *)&D_L23_001B3800, arg0, D_L23_001B3810, 0x32, 0x46,
                                                D_L23_001B15AC, D_L23_001B15B0, 0xF7F0F, 1.7f, 1.9f, 0.5f,
                                                D_L23_001B15A8, 0x2F00, 1, -1, 1);
    func_002A7750(obj, 1, 0);
    func_002E5CF8(4, 0, arg0);
    D_L23_001B164C = (D_L23_001B164C + 1) % 25;
}
