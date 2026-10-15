/* Best small-tier attempt. Six checks stopped at 296 bytes against 292; the quadword
 * copies and constructor body are present, but the entry scheduling and saved-register
 * assignment remain different. */
extern void *func_002B9030(int);
extern int func_00283CF0(float);
extern int func_002A77E0(int);
extern float func_00283CE0(int);
extern unsigned char *D_L00_001FEA00[];
typedef int TI128 __attribute__((mode(TI)));

void *func_L00_00338E90(TI128 *arg0, TI128 *arg1, int arg2, int arg3, int arg4, float arg5) {
    unsigned char *obj;
    unsigned char *cursor;
    obj = func_002B9030(0x30);
    if (obj == 0) return 0;
    cursor = (unsigned char *)arg0;
    *(TI128 *)(obj + 0x10) = *(TI128 *)cursor;
    cursor = obj + 0x20;
    *(int *)(obj + 4) = arg4 & 0x00FFFFFF;
    *(unsigned char *)(obj + 9) = func_00283CF0(2.0f) + 0x10;
    obj[3] = 0x44;
    obj[1] = 0;
    *(float *)(obj + 0x0C) = arg5 * 210000.0f;
    obj[2] = D_L00_001FEA00[0][0];
    obj[8] = func_002A77E0(0x100);
    *(TI128 *)cursor = *arg1;
    *(float *)(cursor + 0x10) = 2.0f / func_00283CE0(arg2);
    *(unsigned short *)(obj + 0x0A) = arg2;
    *(int *)(cursor + 0x18) = arg4;
    *(unsigned short *)(cursor + 0x1C) = arg3;
    *(int *)(cursor + 0x14) = arg4 & 0x00FFFFFF;
    return (void *)obj;
}
