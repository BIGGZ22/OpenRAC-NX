#include "include_asm.h"

extern void func_00284EA8(void);

typedef long s64;

extern void func_002CBC58(s64);

extern float func_00283BE0_6(float b, float a);

extern float func_00282C48_8(float t);

extern void func_00283B98_10(float a, float c);

/* calls the measured no-argument callee */
void func_L00_002E5918(void) {
 func_00284EA8();
}

INCLUDE_ASM("asm/levels", func_L00_002E6348);

INCLUDE_ASM("asm/levels", func_L00_002E6400);

INCLUDE_ASM("asm/levels", func_L00_002E6440);

/* calls the measured callee with zero */
void func_L00_002E6448(void) {
 func_002CBC58(0);
}

INCLUDE_ASM("asm/levels", func_L00_002E6CF0);

INCLUDE_ASM("asm/levels", func_L00_002E6D40);

INCLUDE_ASM("asm/levels", func_L00_002E6DB8);

INCLUDE_ASM("asm/levels", func_L00_002E6E08);

INCLUDE_ASM("asm/levels", func_L00_002E6E80);

INCLUDE_ASM("asm/levels", func_L00_002E6EF8);

INCLUDE_ASM("asm/levels", func_L00_002E6F48);

INCLUDE_ASM("asm/levels", func_L00_002E73E0);

INCLUDE_ASM("asm/levels", func_L00_002E7F98);

INCLUDE_ASM("asm/levels", func_L00_002E8120);

INCLUDE_ASM("asm/levels", func_L00_002E8428);

INCLUDE_ASM("asm/levels", func_L00_002E8DD8);

INCLUDE_ASM("asm/levels", func_L00_002E9098);

INCLUDE_ASM("asm/levels", func_L00_002E90A8);

INCLUDE_ASM("asm/levels", func_L00_002E9100);

INCLUDE_ASM("asm/levels", func_L00_002E9120);

INCLUDE_ASM("asm/levels", func_L00_002E9140);

INCLUDE_ASM("asm/levels", func_L00_002E9210);

INCLUDE_ASM("asm/levels", func_L00_002E94B8);

INCLUDE_ASM("asm/levels", func_L00_002E95C0);

INCLUDE_ASM("asm/levels", func_L00_002E9AA0);

INCLUDE_ASM("asm/levels", func_L00_002EA318);

INCLUDE_ASM("asm/levels", func_L00_002EA600);

INCLUDE_ASM("asm/levels", func_L00_002EA660);

INCLUDE_ASM("asm/levels", func_L00_002EA668);

INCLUDE_ASM("asm/levels", func_L00_002EA708);

INCLUDE_ASM("asm/levels", func_L00_002EBE88);

INCLUDE_ASM("asm/levels", func_L00_002EC230);

INCLUDE_ASM("asm/levels", func_L00_002EC248);

INCLUDE_ASM("asm/levels", func_L00_002EC608);

INCLUDE_ASM("asm/levels", func_L00_002EC970);

INCLUDE_ASM("asm/levels", func_L00_002ECFE8);

INCLUDE_ASM("asm/levels", func_L00_002ED3F0);

void func_L00_002ED420(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_002ED7F8);

INCLUDE_ASM("asm/levels", func_L00_002ED808);

INCLUDE_ASM("asm/levels", func_L00_002EDC90);

INCLUDE_ASM("asm/levels", func_L00_002EDCB8);

INCLUDE_ASM("asm/levels", func_L00_002EDCC8);

INCLUDE_ASM("asm/levels", func_L00_002EDCF0);

INCLUDE_ASM("asm/levels", func_L00_002EDD00);

INCLUDE_ASM("asm/levels", func_L00_002EDD28);

INCLUDE_ASM("asm/levels", func_L00_002EDD38);

INCLUDE_ASM("asm/levels", func_L00_002EE050);

INCLUDE_ASM("asm/levels", func_L00_002EE058);

INCLUDE_ASM("asm/levels", func_L00_002EE078);

INCLUDE_ASM("asm/levels", func_L00_002EE0C0);

INCLUDE_ASM("asm/levels", func_L00_002EE290);

INCLUDE_ASM("asm/levels", func_L00_002EE340);

INCLUDE_ASM("asm/levels", func_L00_002EE4D8);

INCLUDE_ASM("asm/levels", func_L00_002EE660);

/* Takes three floats, computes a value from the second and first, negates the third when that value is not positive, clamps the third against that value and its negation (recomputing it in each branch), then calls a helper with the first and the clamped third. */
void func_L00_002EF548(float a, float b, float c)
{
    float t = func_00283BE0_6(b, a);

    if (t <= 0.0f)
        c = -c;
    if (func_00282C48_8(t) < c)
        c = func_00282C48_8(t);
    else if (c < -func_00282C48_8(t))
        c = -func_00282C48_8(t);
    func_00283B98_10(a, c);
}

INCLUDE_ASM("asm/levels", func_L00_002EF688);

INCLUDE_ASM("asm/levels", func_L00_002F0498);
