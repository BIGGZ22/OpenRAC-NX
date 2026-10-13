/* functional: high. Same loads, stores, values and return as retail, in the same order; the only thing left out is the final `sync` (a memory barrier, which does not change any stored value) and the register save of $16 that the candidate's compile does not reproduce. */
typedef int u128 __attribute__((mode(TI)));

// sceGsSetDefStoreImage: fills the GS store-image packet at p and returns 7.
int func_00122818(void *p, int a1, int a2, int a3, int a4, int a5, int a6, int a7) {
    char *b = (char *)p;
    s64 v10, v18, v20, v30, v40;

    *(u128 *)(b + 0x10) = 0;
    v10 = *(s64 *)(b + 0x10);
    v18 = *(s64 *)(b + 0x18);
    v10 = (((v10 & ~0x7FFFLL) | 5 | 0x8000) & (~0ULL >> 4)) | (0x8000ULL << 45);
    v18 = (v18 & ~0xFLL) | 0xE;
    v20 = (s64)(s32)(a2 << 16) | (s64)(s16)a1 | ((s64)(s16)a3 << 24);
    v30 = (s64)(s32)(a5 << 16) | (s64)(s16)a4;
    v40 = (s64)(s16)a6 | ((s64)(s16)a7 << 32);

    *(int *)(b + 0x4) = 0x06008000;
    *(s64 *)(b + 0x68) = 0x53;
    *(int *)(b + 0x8) = 0x13000000;
    *(int *)(b + 0xC) = 0x50000006;
    *(s64 *)(b + 0x10) = v10;
    *(s64 *)(b + 0x18) = v18;
    *(s64 *)(b + 0x20) = v20;
    *(s64 *)(b + 0x28) = 0x50;
    *(s64 *)(b + 0x30) = v30;
    *(s64 *)(b + 0x38) = 0x51;
    *(s64 *)(b + 0x40) = v40;
    *(s64 *)(b + 0x48) = 0x52;
    *(s64 *)(b + 0x58) = 0x61;
    *(s64 *)(b + 0x60) = 1;
    *(int *)(b + 0x0) = 0;
    *(s64 *)(b + 0x50) = 0;
    return 7;
}
