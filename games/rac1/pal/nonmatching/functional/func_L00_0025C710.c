/* functional: high. Copies a and b to the stack first (as retail does), then out[i] = func_L00_0025C700(a[i], b[i], t) for the four floats, in order; behaviour is exact, only the register choice differs. */
typedef int V128Interp __attribute__((mode(TI)));
extern float func_L00_0025C700(float a, float b, float t);

void func_L00_0025C710(void *out_, void *a_, void *b_, float t) {
    float *out = (float *)out_;
    V128Interp va = *(V128Interp *)a_;
    V128Interp vb = *(V128Interp *)b_;
    float *pa = (float *)&va;
    float *pb = (float *)&vb;
    {float value = func_L00_0025C700(pa[0], pb[0], t); out[0] = value;}
    {float value = func_L00_0025C700(pa[1], pb[1], t); out[1] = value;}
    {float value = func_L00_0025C700(pa[2], pb[2], t); out[2] = value;}
    {float value = func_L00_0025C700(pa[3], pb[3], t); out[3] = value;}
}
