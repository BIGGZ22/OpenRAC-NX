struct sp_decl {
    int sign;
    int class;
    int exp;
    int frac;
};

extern int __pack_f(struct sp_decl *x);

int __make_fp(int sign, int class, int exp, int frac) {
    struct sp_decl x;
    x.sign = sign;
    x.class = class;
    x.exp = exp;
    x.frac = frac;
    return __pack_f(&x);
}
