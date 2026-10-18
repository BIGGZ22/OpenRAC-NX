typedef struct {
    int pad0;
    int field04;
    int field08;
    int field0C;
    int pad10;
    int pad14;
    unsigned char field18;
    unsigned char field19;
} tMemcardState;

extern tMemcardState memcard_region;
extern int *memcard_unk1EDC7B8;
extern int memcard_unk1E62F28(int);

int MC_SetError__Fi(int error) {
    int r;

    memcard_region.field0C = memcard_region.field19;
    r = memcard_unk1E62F28(0x16);
    if (r != 0) {
        memcard_region.field08 = error;
        memcard_unk1EDC7B8[1] = 0;
    }
    return r;
}
