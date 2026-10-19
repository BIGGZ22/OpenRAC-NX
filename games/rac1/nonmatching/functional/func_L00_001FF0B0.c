/* functional: high. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Copies 16-byte blocks from src to dst, one lq/sq pair per pass, do-while on dst != end, so it always copies at least once; retail's two nops and the src-increment placement are scheduling only and do not change reads, writes or order. */
typedef int u128_FF0B0 __attribute__((mode(TI)));

/* Copies 16-byte blocks from src to dst until dst reaches end. */
void func_L00_001FF0B0(char *dst, char *src, char *end) {
    do {
        *(u128_FF0B0 *)dst = *(u128_FF0B0 *)src;
        src += 0x10;
        dst += 0x10;
    } while (dst != end);
}
