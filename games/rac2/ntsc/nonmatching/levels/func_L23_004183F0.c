/* func_L23_004183F0 (608 bytes, 1 place: 23:004183F0) -- NEAR MISS, not integrated.
   This C compiles to 608 bytes, the retail size, and needs the -g8 sibling ($gp float
   D_L23_001B15B4; D_L23_001B3800/3810 are 16 bytes and stay absolute); check.py reports
   "instruction at +0x48 differs", 0 of 1 places.
   Only the 128-bit copy `D_L23_001B3800 = v1` differs. Retail:
     lui $v1,0x1B ; addiu $v1,$v1,0x3800 ; lq $v0,0($s0) ; sq $v0,0($v1) ; lwc1 $f12,... 
   (the pair is adjacent, right behind the func_0029FCC0 call, and its source is $s0, the pointer
   kept for the calls below). This cc1 emits `lq $v0,0x10($sp)` first (offset folded into the quad
   load), then the address pair and the argument setup, and the `sq` last: six words.
   Everything else is exact, and these spellings are what gets it there (each measured against the
   alternative):
   - the four absolute globals are volatile only so the loop optimiser does not hoist the load of
     D_L23_001B36B0 above the counting loop (retail loads it after the loop; non-volatile: 26 words
     differ);
   - the counting loop is `p = table; i = 24; do { i--; ...; p++; } while (i >= 0)`; a for loop is
     8 bytes over;
   - `ok = !(flags & 1); if (ok && call(...))` gives retail's `xori ; andi ; beqz`; the inline
     `!(flags & 1) &&` is worse;
   - `if (best == 0) {...} else if (dist < bestDist) {...}`; one `best == 0 || dist < bestDist`
     is 12 bytes short;
   - func_00282948 is declared int (void: two more words differ).
   A compiler that keeps TImode quad addresses in registers (and treats the pair as one unit) would
   close the residue. */

typedef int TI128 __attribute__((mode(TI)));

extern float D_L23_001B15B4 __attribute__((sda));

extern TI128 D_L23_001B3800 __attribute__((sda));

extern char D_L23_001B3810[16] __attribute__((sda));

extern volatile int D_L23_001B37F0;

extern volatile int D_L23_001B37F4;

extern volatile int D_L23_001B36AC;

extern volatile int D_L23_001B36B0;

extern char *D_L23_00259560[];

extern void func_0029FCC0(char *, int, char *);

extern void func_0029FC20(char *, int, char *);

extern void func_00282D30(char *, char *, float);

extern void func_00282CC0(char *, char *, char *);

extern int func_00282948(char *);

extern void func_0029F338(int);

extern int func_L23_003C19E0(char *, float *);

extern void func_L23_003C1AB0(char *);

extern void func_L23_00418160(char *);

void func_L23_004183F0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    float v0[4];
    float v1[4];
    float m[16];
    float dist;
    char *best;
    float bestDist;
    char **p;
    int i;
    int j;
    int a;
    int b;
    int ok;

    if (*(int *)(state + 0x59C) != 0) {
        func_0029FCC0(arg0, *(int *)(state + 0x594) + 0x1E, (char *)v1);
        D_L23_001B3800 = *(TI128 *)v1;
        func_00282D30(D_L23_001B3810, arg0 + 0xE0, D_L23_001B15B4);
        func_00282CC0(D_L23_001B3810, D_L23_001B3810, *(char **)(state + 0x59C) + 0x10);
        func_0029FC20(arg0, *(int *)(state + 0x594) + 0x1E, (char *)m);
        func_00282D30((char *)v0, (char *)(m + 8), -16.0f);
        func_00282CC0((char *)v1, (char *)v1, (char *)v0);
        func_00282948(state + 0x598);
        if (*(int *)(state + 0x598) == 0) {
            *(int *)(state + 0x59C) = 0;
            *(int *)(state + 0x594) = (*(int *)(state + 0x594) + 1) % 4;
            a = D_L23_001B37F0;
            if (a != 0) {
                func_0029F338(a);
                D_L23_001B37F0 = 0;
            }
            b = D_L23_001B37F4;
            if (b != 0) {
                func_0029F338(b);
                D_L23_001B37F4 = 0;
            }
        }
    }
    D_L23_001B36AC = 0;
    p = D_L23_00259560;
    i = 24;
    do {
        i--;
        if (!(*(unsigned short *)(*p + 0x34) & 1))
            D_L23_001B36AC = D_L23_001B36AC + 1;
        p++;
    } while (i >= 0);
    best = 0;
    bestDist = 3000.0f;
    if (D_L23_001B36B0 == 0) {
        for (j = 0; j < 25; j++) {
            char *obj = D_L23_00259560[j];

            ok = !(*(unsigned short *)(obj + 0x34) & 1);
            if (ok && func_L23_003C19E0(obj, &dist) != 0) {
                if (best == 0) {
                    best = obj;
                    bestDist = dist;
                } else if (dist < bestDist) {
                    best = obj;
                    bestDist = dist;
                }
            }
        }
    }
    if (best != 0)
        func_L23_003C1AB0(best);
    if (*(int *)(state + 0x590) != 0 && *(int *)(state + 0x598) <= 0)
        func_L23_00418160(arg0);
}
