extern float D_L00_001B15B8 __attribute__((sda));
extern char D_L00_001B9040[];
extern char D_00189F70[];
extern int D_001A7BAC;
extern float D_L00_001A8904;
extern int D_L00_001A8FF0;
extern void func_00282CC0(char *, char *, char *);
extern float func_00283B98(float, float);
extern void func_L00_004208C8(char *, float *);
extern void func_L00_004224D0(char *);
extern void func_L00_00421C50(char *);
extern void func_L00_00426340(int);
extern void func_L00_00422268(char *);

void func_L00_004263B8(char *object)
{
    char local[16];
    char *hero = D_00189F70;
    char *self = object;
    char *parameters = D_L00_001B9040;
    float value = (&D_L00_001B15B8)[D_001A7BAC];
    float first;

    D_L00_001A8904 = value;
    func_00282CC0(local, hero, parameters);
    first = D_L00_001A8FF0 != 0 ? 0.0f : *(float *)(hero + 12);
    *(float *)(local + 12) = func_00283B98(first, *(float *)(parameters + 12));
    func_L00_004208C8(self, (float *)local);
    func_L00_004224D0(self);
    func_L00_00421C50(self);
    func_L00_00426340((int)self);
    func_L00_00422268(self);
}
