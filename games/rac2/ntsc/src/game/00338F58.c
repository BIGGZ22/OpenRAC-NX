extern float D_L00_001BD2C0[];
extern void func_0027AB40(void);
extern void func_00338AD8(char *);
extern void func_00338F50(char *);

void func_00338F58(char *object)
{
    float saved;
    if (*(float *)*(int *)(object + 4) == 0.0f)
        return;
    saved = D_L00_001BD2C0[44];
    D_L00_001BD2C0[44] = 0.62f;
    func_0027AB40();
    switch (*(int *)(object + 0x1C8)) {
    case 0: func_00338AD8(object); break;
    case 2: func_00338F50(object); break;
    }
    D_L00_001BD2C0[44] = saved;
    func_0027AB40();
}
