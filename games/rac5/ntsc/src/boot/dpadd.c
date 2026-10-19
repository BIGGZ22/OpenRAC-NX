/* dpadd - libgcc double addition through the unpacked fp_number_form. */

typedef struct {
    int class;
    int sign;
    int normal_exp;
    int pad;
    long long fraction;
    int pad2;
} fp_number_type;

extern void __unpack_d(double *a, fp_number_type *x);
extern fp_number_type *__add_d(fp_number_type *x, fp_number_type *y,
                               fp_number_type *z);
extern double __pack_d(fp_number_type *z);

double dpadd(double a, double b) {
    fp_number_type x, y, z;
    fp_number_type *r;

    __unpack_d(&a, &x);
    __unpack_d(&b, &y);
    r = __add_d(&x, &y, &z);
    return __pack_d(r);
}
