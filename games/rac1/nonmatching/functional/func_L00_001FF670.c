/* functional: high. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Copies the 16-byte vector at src to dst (lq from src, sq to dst), no return value. */
typedef int u128_FF670 __attribute__((mode(TI)));

void func_L00_001FF670(char *dst, char *src) {
    *(u128_FF670 *)dst = *(u128_FF670 *)src;
}
