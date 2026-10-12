/* 96 bytes: under a -G8 probe the compile is 80 bytes; the prefix through the jal and the sb agree, the tail differs (retail stores -1 through a beq and keeps a dead li 10 before a b in the else path); also needs config/cflags.txt -O2 -G8 */
extern void func_L00_00402DB8(char *arg);
extern int *D_L00_001B0CA0 __attribute__((sda));

void func_L00_004028C0(char *arg, int value)
{
    int *obj = D_L00_001B0CA0;
    if (obj) {
        int *p = (int *)obj[26];
        ((unsigned char *)obj)[32] = arg ? 5 : 7;
        func_L00_00402DB8(arg);
        if (value == -1) {
            p[24] = -1;
        } else {
            p[24] = value;
        }
    }
}
