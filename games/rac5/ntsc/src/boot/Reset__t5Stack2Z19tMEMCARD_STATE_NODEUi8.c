typedef struct {
    unsigned char base[0x80];
    int field80;
} tStack2Z19tMEMCARD_STATE_NODEUi8;

extern void *memset(void *dst, int value, unsigned int size);

void Reset__t5Stack2Z19tMEMCARD_STATE_NODEUi8(tStack2Z19tMEMCARD_STATE_NODEUi8 *self) {
    self->field80 = 0;
    memset(self, 0, 0x80);
}
