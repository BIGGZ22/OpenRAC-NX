#include "include_asm.h"

void func_001339F0(void *dest, const void *src, int n)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    int i = 0;

    if (n <= 0) {
        return;
    }

    do {
        d[i] = s[i];
        i++;
    } while (i < n);
}

extern void func_001334B8(int, int, void *, unsigned char *);
extern void func_00133230(void);
extern void func_00132028(void);
extern void func_00133580(int);

void func_00133A28(int first, int second, void *third) {
    int enabled = 1;
    unsigned char mode[16];
    mode[0] = 32;
    mode[1] = enabled;
    mode[2] = 0;
    mode[3] = 0;
    func_001334B8(first, second, third, mode);
    func_00133230();
    func_00132028();
    func_00133580(0);
}

extern void func_00133A28(int, int, void *);
typedef struct {
    int first;
    int reserved04;
    int second;
    int reserved0c;
    int third;
    int reserved14;
} CopySetupRow;
typedef struct {
    char reserved00[0x4FF8];
    CopySetupRow rows[28];
    char first[0x60];
    char second[0x1018];
    char third[0x137C];
} CopySetupData;
extern CopySetupData D_0014B540;

void func_00133A78(void) {
    func_00133A28(1001, 11, &D_0014B540);
}

void func_00133AA0(int index, void *buffer) {
    func_00133A28(D_0014B540.rows[index].first, 1, buffer);
    func_001339F0(D_0014B540.first, buffer, 0x60);
    func_00133A28(D_0014B540.rows[index].second, 3, buffer);
    func_001339F0(D_0014B540.second, buffer, 0x1018);
    func_00133A28(D_0014B540.rows[index].third, 3, buffer);
    func_001339F0(D_0014B540.third, buffer, 0x137C);
}
