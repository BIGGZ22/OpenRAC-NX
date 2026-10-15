/* First plain-C attempt from worker w08. Last check: 364 bytes vs 400; 0/16. */
extern unsigned char D_00189E20_1[];
extern void func_L00_002AF7D0(void);
extern void func_L00_00322EC8(char *);
extern void func_L00_002D2E68(void);
extern void func_L00_002A6708(void);
extern void func_L00_002A53B8(void);
extern void func_L00_002B32F8(void);
extern void func_L00_002D24B8(void);
extern void func_002A677C(char *);
extern void func_L00_002C42E0(int, int);
extern void func_0028B3D0(int, int, char *, char *, char *, int, int);
extern void func_L00_002F6B90(int);
extern int D_00139320;
extern char D_L00_002F8588[];
extern char D_L00_002F85D0[];
extern char D_L00_002F8AD8[];

void func_L00_002B2A40(int mode, int value, char *object)
{
    char *p = (char *)D_00189E20_1;
    char *current;
    int count;

    func_L00_002AF7D0();
    *(int *)(p + 0x2290) = *(int *)(p + 0xC18);
    func_L00_00322EC8(p + 0x18E0);
    func_L00_00322EC8(p + 0x1A30);
    current = *(char **)(p + 0x2290);
    if (mode == 4)
        *(int *)(current + 0x9C) = -1;
    if ((unsigned char)p[0x22B4] != mode) {
        func_L00_002D2E68();
        func_L00_002A6708();
        func_L00_002A53B8();
    }
    *(int *)(p + 0x2290) = (int)object;
    p[0x22B4] = (char)mode;
    *(int *)(p + 0xC14) = (int)object;
    if (mode == 1 && (D_00139320 & 0x10) != 0) {
        func_0028B3D0(0x23, 0, D_L00_002F8588, D_L00_002F85D0, D_L00_002F8AD8, 0, 0);
        func_L00_002F6B90(3);
        func_L00_002B32F8();
    }
    *(float *)(p + 0x250) = *(float *)(p + 0x258);
    *(float *)(p + 0x254) = *(float *)(p + 0x25C);
    *(unsigned short *)(object + 0x34) |= 6;
    *(float *)(p + 0xC24) = 1.0f;
    *(float *)(p + 0xC20) = 1.0f;
    func_L00_002D24B8();
    func_002A677C(object);
    if (mode == 1) {
        *(short *)(p + 0x24F2) = (short)value;
        count = *(int *)(p + 0x24D0);
        *(short *)(p + 0x24D4) = *(short *)(p + 0x24CC);
        if (count >= 5)
            count = 4;
        *(int *)(p + 0x24CC) = count;
        func_L00_002C42E0(value, 2);
    }
}
