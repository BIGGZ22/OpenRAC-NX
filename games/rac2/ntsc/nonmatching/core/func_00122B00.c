/* from src/core/001213B8.c: 680 bytes against retail's 680 - same size, and
 * every instruction matches except one delay slot and the two instructions
 * around it (six differing words in all, three of them ordinary
 * relocations: the two unpack_d calls, the four __muldi3 calls and the
 * `thenan` %hi/%lo pair).
 *
 * libgcc.a dp-bit.o: dpmul, gcc/config/fp-bit.c under -DDP_FLOAT
 * -DUS_SOFTWARE_GOFAST, whose `#define multiply dpmul` names this member.
 * `_fpmul_parts` is `static __inline__` upstream and is inlined into dpmul's
 * one call site, which is why retail has no symbol between dpmul and dpdiv.
 * Everything is confirmed against retail: the 64x64-to-128 branch (four
 * __muldi3 calls), the DP parameters (IMPLICIT_1 1<<60, IMPLICIT_2 1<<61,
 * FRACHIGH 0x8000000000000000, GARDMASK 0xff, GARDMSB 0x80, GARDROUND 0x7f,
 * `normal_exp += 4`), the two normalize loops and the rounding block.
 *
 * The one difference is the delay slot of the rounding test:
 *
 *   retail  bnel $v1,$v0,+0x20 ; sd $s1,0x50($sp)      # store, annulled
 *   ours    bne  $v1,$v0,+0x20 ; addiu $v0,$zero,0x3   # constant
 *
 * so retail's merge point opens with `tmp->fraction = high` and ours with
 * the CLASS_NUMBER constant, and reorg fills the slot from that first
 * instruction (eager fill in retail's case, simple fill in ours). The
 * branch instructions are otherwise identical at the same address, and both
 * orderings are correct - this is not a source-shape difference that a
 * statement reorder reaches: swapping the two final stores produces a
 * byte-identical object (measured).
 *
 * Verified: tools/check.py func_00122B00 --diff: boot: 680 bytes, expected
 * 680, `instruction at +0x244 differs`.
 */

typedef struct {
    int kind;
    unsigned int sign;
    int exponent;
    unsigned long fraction;
} DoubleParts;

extern void func_00122760(double *, DoubleParts *);
extern double func_00122630(DoubleParts *);
extern DoubleParts D_00141810;   /* fp-bit.c's function-static `thenan` */

static __inline__ int double_parts_iszero(DoubleParts *p) { return p->kind == 2; }
static __inline__ int double_parts_isnan(DoubleParts *p) { return p->kind == 0 || p->kind == 1; }
static __inline__ int double_parts_isinf(DoubleParts *p) { return p->kind == 4; }

static __inline__ DoubleParts *dp_mul_parts(DoubleParts *a, DoubleParts *b,
                                            DoubleParts *tmp) {
    unsigned long long low = 0;
    unsigned long long high = 0;

    if (double_parts_isnan(a)) { a->sign = a->sign != b->sign; return a; }
    if (double_parts_isnan(b)) { b->sign = a->sign != b->sign; return b; }
    if (double_parts_isinf(a)) {
        if (double_parts_iszero(b)) return &D_00141810;
        a->sign = a->sign != b->sign; return a;
    }
    if (double_parts_isinf(b)) {
        if (double_parts_iszero(a)) return &D_00141810;
        b->sign = a->sign != b->sign; return b;
    }
    if (double_parts_iszero(a)) { a->sign = a->sign != b->sign; return a; }
    if (double_parts_iszero(b)) { b->sign = a->sign != b->sign; return b; }

    {
        unsigned long long nl = a->fraction & 0xffffffff;
        unsigned long long nh = a->fraction >> 32;
        unsigned long long ml = b->fraction & 0xffffffff;
        unsigned long long mh = b->fraction >> 32;
        unsigned long long pp_ll = ml * nl;
        unsigned long long pp_hl = mh * nl;
        unsigned long long pp_lh = ml * nh;
        unsigned long long pp_hh = mh * nh;
        unsigned long long res2 = 0;
        unsigned long long res0 = 0;
        unsigned long long ps_hh__ = pp_hl + pp_lh;

        if (ps_hh__ < pp_hl) res2 += 0x100000000LL;
        pp_hl = (ps_hh__ << 32) & 0xffffffff00000000LL;
        res0 = pp_ll + pp_hl;
        if (res0 < pp_ll) res2++;
        res2 += ((ps_hh__ >> 32) & 0xffffffffL) + pp_hh;
        high = res2;
        low = res0;
    }

    tmp->exponent = a->exponent + b->exponent;
    tmp->sign = a->sign != b->sign;
    tmp->exponent += 4;

    while (high >= (unsigned long long)(1LL << 61)) {
        tmp->exponent++;
        if (high & 1) { low >>= 1; low |= 0x8000000000000000LL; }
        high >>= 1;
    }
    while (high < (unsigned long long)(1LL << 60)) {
        tmp->exponent--;
        high <<= 1;
        if (low & 0x8000000000000000LL) high |= 1;
        low <<= 1;
    }
    if ((high & 0xff) == 0x80) {
        if (high & (1 << 8))
            high += 0x7f + 1;
        else if (low)
            high += 0x7f + 1;
    }
    tmp->fraction = high;
    tmp->kind = 3;
    return tmp;
}

double func_00122B00(double first, double second) {
    DoubleParts a, b, tmp;
    DoubleParts *res;

    func_00122760(&first, &a);
    func_00122760(&second, &b);
    res = dp_mul_parts(&a, &b, &tmp);
    return func_00122630(res);
}
