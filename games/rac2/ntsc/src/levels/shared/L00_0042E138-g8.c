/* The level geometry block the model is bound to: the data the model is set up from lives in
   the level's literal pool and is read through $gp. */
extern int func_00336E40(int value);

extern int *func_00336DA8(int size, int pool);

extern void func_0033ABD8(void *object, int value, const void *data);

extern int func_00288FB8(int text);

extern void func_0033B050(void *object, int first, int second, int third);

extern void func_0033B030(void *object, float x0, float y0, float z0, float x1, float y1, float z1);

extern void func_0033B0A8(void *object, float value);

extern int func_L00_00433198(char *object, int value);

extern int D_L00_001AE1C8 __attribute__((sda));

void func_L00_00433070(char *object, int value)
{
    char *child;
    int *holder;
    int first;
    int second;

    *(int *)object = value;
    if (value != 0) {
        holder = (int *)func_00336DA8(16, func_00336E40(value));
        *(int **)(object + 0x2AC) = holder;
        ((int *) holder)[1] = 0;
        ((int *) holder)[2] = 0;
        ((int *) holder)[3] = 0;
        ((int *) holder)[0] = 0;
    }
    child = object + 8;
    ((float *) *(int *)(object + 0x2AC))[0] = 255.0f;
    ((float *) *(int *)(object + 0x2AC))[1] = 195.0f;
    func_0033ABD8(child, value, &D_L00_001AE1C8);
    first = func_00288FB8(0x2D61);
    second = func_00288FB8(0x2D4E);
    func_0033B050(child, first, second, func_00288FB8(0x2D4F));
    func_0033B030(child, 0.0f, -143.0f, 0.0f, 114.0f, 0.0f, 140.0f);
    func_0033B0A8(child, 0.4f);
    *(int *)(object + 0x2A8) = 0;
    func_L00_00433198(object, 0);
}
