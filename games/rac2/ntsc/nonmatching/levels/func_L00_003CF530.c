/* Best small-tier attempt: 216 bytes versus 220. The remaining difference is a retained
 * dead addiu in the beqz delay slot; plain C removes it and shifts the body by one word. */
extern char D_00189EA0[];
extern int func_002ACBC8(char *, unsigned int);
extern void func_002E5CF8(int, int, char *);
extern float func_00283180(float);
extern float func_00283198(float);
extern void func_00282CC0(char *, char *, char *);
extern void func_0029F338(char *);

void func_L00_003CF530(char *obj) {
    char *ctx;
    int result;
    float angle;
    struct { float x; float y; int z; } point;
    ctx = *(char **)(obj + 0x68);
    if (*(unsigned char *)(obj + 0x20) == 0) {
        result = func_002ACBC8(D_00189EA0, *(unsigned int *)ctx);
        if (result != 0) {
            *(unsigned char *)(obj + 0x20) = result;
            *(int *)(ctx + 4) = 0;
            func_002E5CF8(0, 0, obj);
        }
    }
    if (*(unsigned char *)(obj + 0x20) != 1) return;
    angle = *(float *)(obj + 0xF8);
    point.x = func_00283180(angle) * 0.4166667f;
    angle = *(float *)(obj + 0xF8);
    point.y = func_00283198(angle) * 0.4166667f;
    point.z = 0;
    func_00282CC0(obj + 0x10, obj + 0x10, (char *)&point);
    *(int *)(ctx + 4) += 1;
    if (*(int *)(ctx + 4) < 0x65) return;
    func_0029F338(obj);
}
