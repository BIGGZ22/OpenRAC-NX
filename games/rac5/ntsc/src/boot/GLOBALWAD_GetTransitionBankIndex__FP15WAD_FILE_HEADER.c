typedef struct {
    unsigned char raw[0xC0];
} WAD_FILE_HEADER;

typedef struct {
    unsigned char header[0x10C0];
    WAD_FILE_HEADER files[1];
} WAD_ROOT;

extern WAD_ROOT *global_wad_root;

int GLOBALWAD_GetTransitionBankIndex__FP15WAD_FILE_HEADER(WAD_FILE_HEADER *file) {
    WAD_ROOT *root = global_wad_root;
    WAD_FILE_HEADER *base;
    if (root) {
        base = (WAD_FILE_HEADER *)((unsigned char *)root + 0x10C0);
        return (int)(file - base);
    }
    return 0;
}
