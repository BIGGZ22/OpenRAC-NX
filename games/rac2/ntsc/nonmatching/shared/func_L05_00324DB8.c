extern int D_L05_001A9D48[];
extern int func_L05_003251A0(void);

int func_L05_00324DB8(void) {
    int index = func_L05_003251A0();
    return index < 0 ? index : D_L05_001A9D48[index];
}
