/* func_L00_003ACCF8, 104 bytes, 27 placements.
   Nearest: size exact, 25 of 26 instructions identical.
   Last line of tools/check.py: "00_aranos_tutorial: instruction at +0x3c differs"
   The only difference is the position of `mtc1 $zero,$f12` against
   `lw $a2,0x7B8($gp)`: retail emits the float constant first, this source has
   the scheduler emit the global load first.  Naming the constant in a local
   float does not move it.  Needs the -g8 sibling file ($gp globals). */

extern char D_L00_0023BB78[];
extern char D_L00_0023B710[];
extern int D_L00_001AF7A8 __attribute__((sda));

extern void func_2EC608(void *, int);

extern void func_2EC970(void *, void *, int, int, float, float);

extern void func_L00_003ACC88();

void func_L00_003ACCF8(char *a0)
{
    char local[16];
    int *p = *(int **)(a0 + 0x68);

    func_L00_003ACC88(a0, local);
    func_2EC608(local, D_L00_001AF7A8);
    func_2EC970(local, D_L00_0023B710 + p[11] * 64, D_L00_001AF7A8, 4, 0.0f, 0.25f);
}
