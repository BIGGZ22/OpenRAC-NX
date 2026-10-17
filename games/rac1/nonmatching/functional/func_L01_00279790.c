/* functional: high. Written from the assembly (no C existed). Distance test: f = func_001F9D48(moby + 0x10, D_0013E633 + 0xE9D); flag = (f < 7.0f) ? 2 : 0; then func_L00_00261B00(moby + 0x10, 4, 7, flag, -1). Returns void: the retail return is whatever $2 the void callee leaves, and no caller reads it. */
extern float func_001F9D48(void *, void *);
extern void func_L00_00261B00(void *, int, int, int, int);
extern char D_0013E633[];

void func_L01_00279790(void *moby) {
    char *m = moby;
    int flag = 0;
    float f = func_001F9D48(m + 0x10, D_0013E633 + 0xE9D);
    if (f < 7.0f) {
        flag = 2;
    }
    func_L00_00261B00(m + 0x10, 4, 7, flag, -1);
}
