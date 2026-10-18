typedef struct {
    unsigned char raw[0xC0];
} WAD_FILE_HEADER;

typedef struct {
    unsigned char header[0x10C0];
    WAD_FILE_HEADER files[1];
} WAD_ROOT;

extern WAD_ROOT *global_wad_root;

WAD_FILE_HEADER *GLOBALWAD_GetTransitionBank__FUi(unsigned int index) {
    WAD_ROOT *root = global_wad_root;
    WAD_FILE_HEADER *base;
    if (root) {
        base = root->files;
        return base + index;
    }
    return 0;
}
