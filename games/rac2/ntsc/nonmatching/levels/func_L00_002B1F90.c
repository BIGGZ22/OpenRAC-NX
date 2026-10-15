/* First plain-C attempt from worker w08. Last check: 360 bytes vs 400; 0/27. */
extern unsigned char D_00189E20_1[];
extern void func_L00_002B1350(int);
extern void func_002A7470(char *, int, int, int);
extern int D_L00_001B18D8 __attribute__((sda));
extern void func_L00_002B1230(int, int);

void func_L00_002B1F90(int offset)
{
    char *base = (char *)D_00189E20_1 + offset;
    int i;

    for (i = 0; i < 7; i++) {
        char *slot = base + i * 0x50;
        char *entry = *(char **)(slot + 0x1220);
        int state = *(int *)(slot + 0x1244);
        int value;
        void (*callback)(char *);

        if (entry == 0) {
            func_L00_002B1350(i);
            continue;
        }
        if (state == 2) {
            *(int *)(slot + 0x1240) += 1;
            func_L00_002B1350(i);
        }
        if ((*(unsigned char *)(entry + 0x60) & 2) != 0) {
            if (*(unsigned char *)(entry + 0x43) == 0) {
                if (*(unsigned char *)(entry + 0x24) >= 2)
                    func_002A7470(entry, 1, 0, 2);
            }
        } else if (*(int *)(slot + 0x1500) == 0) {
            func_L00_002B1350(i);
        }
        if (D_L00_001B18D8 != 0 && (i == 0 || i == 1)) {
            value = *(int *)((char *)D_00189E20_1 + 0x18D8) + 2;
            func_L00_002B1230(i, value);
        }
        value = *(int *)(slot + 0x1248);
        if (value == 10 && entry[0x20] != (char)0xFE && entry[0x20] != (char)0xFD) {
            callback = *(void (**)(char *))(entry + 0x64);
            if (callback != 0)
                callback(entry);
        } else {
            func_L00_002B1350(i);
        }
    }
}
