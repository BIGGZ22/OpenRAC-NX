/* functional: high. Same reads and writes as the assembly: a do-while copy of 32-bit words from src to dst, stopping when dst equals end, leaving both pointers advanced. Retail's register choice ($3) and its hand-placed nops do not change behaviour. */
void func_L00_001FF088(int *dst, int *src, int *end) {
    int v;
    do {
        v = *src;
        src++;
        *dst = v;
        dst++;
    } while (dst != end);
}
