/* func_L05_003CFC48 (168 o) - best attempt, not a match.
   Structure and size are exact (168 bytes, same instructions); the residual is pure
   register allocation: retail keeps the D_L05_00221D10 base in v1 and the loaded value
   in v0, this source allocates them the other way round, and the two loads at +0x10/+0x14
   are taken in the opposite order.  'D[i] = D[i] + 1' vs 'D[i]++' changes nothing.
   verified with: python3 tools/check.py func_L05_003CFC48 --diff (instruction at +0x38 differs)
*/

void func_L05_003CFC48(char *object)
{
    int index = func_002ED8A0(1);

    if (index < 0 || D_L05_002A1800[index].f10 == D_L05_002A1800[index].f14) {
        func_L05_0041C118(object);
        D_L05_00221D10[23] = D_L05_00221D10[23] + 1;
        D_L05_00221D10[28] = D_L05_00221D10[28] + 1;
    } else {
        func_L05_0041D498(object);
        D_L05_00221D10[25] = D_L05_00221D10[25] + 1;
        D_L05_00221D10[28] = D_L05_00221D10[28] + 1;
    }
}
