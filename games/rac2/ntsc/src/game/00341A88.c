

extern void func_00335E38(void *, float, float, float, float);

extern void func_00336920(void *, int, int);

extern int *func_00335E20(void *);

extern void func_00336828(void *);

extern float D_L01_001AE420 __attribute__((sda));

extern float D_L01_001AE424;

extern float D_L01_001AE428 __attribute__((sda));

struct Obj {
    char pad[380];
    int h380;
    char pad2[56];
    int tbl440[3];
    int a452;
    int *p456;
    int count460;
    char pad3[72];
    float *fp536;
    char pad4[224];
    int mode764;
};

void func_00341A88(struct Obj *o)
{
    int i;

    for (i = 0; i < o->count460; i++) {
        func_00335E38(&o->h380, o->fp536[0] + D_L01_001AE420,
                o->fp536[1] + D_L01_001AE424 + D_L01_001AE428 * (float)i, 0.0f, 0.0f);
        if (o->mode764 == 2) {
            int v = o->p456[i];

            if (v == 5)
                func_00336920(&o->h380, 60062, 5);
            else
                func_00336920(&o->h380, v, 0);
        } else if (o->mode764 == 0) {
            int w = o->p456[i];

            if (w == 6)
                func_00336920(&o->h380, 60055, 0);
            else
                func_00336920(&o->h380, o->a452, w);
        } else {
            func_00336920(&o->h380, o->a452, o->p456[i]);
        }
        {
            int *r = func_00335E20(&o->h380);
            int t = o->tbl440[o->mode764] ^ i;
            int alt = 0x70FFFEED;

            *r = t ? 0x60F0F0B0 : alt;
        }
        func_00336828(&o->h380);
    }
}
