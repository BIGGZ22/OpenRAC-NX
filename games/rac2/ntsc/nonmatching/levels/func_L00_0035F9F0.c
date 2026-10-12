/* near miss: 4 of 6 retail instructions (retail 24 bytes, this 16). Missing the dsll32/dsra32 sign extension of the int parameter before bnel. */
void func_L00_0035F9F0(int a0, int a1)
{
    if ((long long)a1 != 0)
        *(int *)a1 = a0;
}
