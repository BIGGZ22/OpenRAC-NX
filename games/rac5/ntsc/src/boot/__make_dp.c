struct dp_decl {
    int sign;
    int class;
    int exp;
    long long frac;
};

extern long long __pack_d(struct dp_decl *x);

long long __make_dp(int sign, int class, int exp, long long frac) {
    struct dp_decl x;
    x.sign = sign;
    x.class = class;
    x.exp = exp;
    x.frac = frac;
    return __pack_d(&x);
}
