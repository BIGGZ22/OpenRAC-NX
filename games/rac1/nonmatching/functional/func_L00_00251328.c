/* functional: high. corrected from nonmatching/shared/func_L00_00251328.c: the shared copy used `long`, which is 32 bits on the host, so its 64-bit word and shifts were wrong there. Retail loads the 64-bit word at m+0x38, keeps its low 32 bits, and ORs a<<32, b<<40 and c<<48 (each argument's low 32 bits shifted, so overlapping bits OR together exactly as here); the word is stored back whole. Arguments stay int, as the callers pass them. */
void func_L00_00251328(char *m, int a, int b, int c) {
    unsigned long long v = *(unsigned long long *)(m + 0x38);
    unsigned long long hi = ((unsigned long long)(unsigned int)a << 32)
                          | ((unsigned long long)(unsigned int)b << 40)
                          | ((unsigned long long)(unsigned int)c << 48);
    *(unsigned long long *)(m + 0x38) = (unsigned int)v | hi;
}
