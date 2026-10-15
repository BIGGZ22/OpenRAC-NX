typedef unsigned char u8;
extern int func_0027F178(u8 *, int);
extern int func_0027F198(u8 *, int);
extern int func_0027F1B8(u8 *, int);
extern int func_0027C878(int);
extern void func_0027F2A8(int, int, int, u8 *, int, int, u8 *);
extern u8 D_00263B50[];
extern u8 D_00263EF0[];
extern u8 D_00264290[];

void func_0027F980(int x, int y, int style, u8 *text, int length)
{
    int font;
    x -= func_0027F178(text, length);
    font = func_0027C878(1);
    func_0027F2A8(x, y, style, text, length, font, D_00263B50);
}

void func_0027FA10(int x, int y, int style, u8 *text, int length)
{
    int font;
    x -= func_0027F198(text, length);
    font = func_0027C878(2);
    func_0027F2A8(x, y, style, text, length, font, D_00263EF0);
}

int func_0027FAA8(int x, int y, int style, u8 *text, int length)
{
    int font;
    x -= func_0027F178(text, length) >> 1;
    font = func_0027C878(1);
    func_0027F2A8(x, y, style, text, length, font, D_00263B50);
    return x;
}

int func_0027FB40(int x, int y, int style, u8 *text, int length)
{
    int font;
    x -= func_0027F198(text, length) >> 1;
    font = func_0027C878(2);
    func_0027F2A8(x, y, style, text, length, font, D_00263EF0);
    return x;
}

int func_0027FBD8(int x, int y, int style, u8 *text, int length)
{
    int font;
    x -= func_0027F1B8(text, length) >> 1;
    font = func_0027C878(3);
    func_0027F2A8(x, y, style, text, length, font, D_00264290);
    return x;
}
