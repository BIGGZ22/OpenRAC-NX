/* NEAR MISS — 168 bytes emitted, 164 expected (one instruction too many).
 * last check.py line: "14_aranos_prison: 168 bytes, expected 164"
 *
 * Everything matches except the constant 1: retail materialises it ONCE
 *   00425C58 addiu $t0,$zero,0x1
 *   00425C88 sb    $t0,0x31($v1)      ; the field write
 *   00425C8C jal   func_32BC08        ; ... and $t0 is also argument 5
 * whereas this compiler rematerialises it for the field write (`addiu $v0,$zero,0x1;
 * sb $v0,0x31($v1)`), costing one word. Tried: the literal 1 in both places, a shared
 * `int one = 1;` local, and both statement orders (store-then-call and call-then-store).
 * The early-return path carrying `daddu $v0,$v1,$zero` in the bne delay slot is reproduced,
 * as is the multiple reload of *(s0+0x120) between stores.
 */
extern char *func_317A28();
extern void func_32BC08();

void *func_L14_00425C08(char *a0)
{
    char *s0;
    char *v1;
    int one;

    one = 1;
    s0 = *(char **)(a0 + 0x68);
    v1 = *(char **)(s0 + 0x120);
    if (v1 != 0 && *(unsigned char *)(v1 + 0x20) != 0xFE &&
        *(unsigned char *)(v1 + 0x20) != 0xFD)
        return v1;
    v1 = func_317A28(0x1142);
    *(char **)(s0 + 0x120) = v1;
    if (v1 != 0) {
        *(unsigned short *)(*(char **)(s0 + 0x120) + 0x32) =
            *(unsigned short *)(a0 + 0x32);
        *(unsigned char *)(*(char **)(s0 + 0x120) + 0x30) =
            *(unsigned char *)(a0 + 0x30);
        *(long long *)(*(char **)(s0 + 0x120) + 0x38) =
            *(long long *)(a0 + 0x38);
        func_32BC08(a0, *(char **)(s0 + 0x120), 2, 0, one);
        *(unsigned char *)(*(char **)(s0 + 0x120) + 0x31) = one;
    }
    return *(void **)(s0 + 0x120);
}
