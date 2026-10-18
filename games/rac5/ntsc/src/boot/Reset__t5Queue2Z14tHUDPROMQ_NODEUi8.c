typedef struct {
    unsigned char base[0x60];
    int field60;
    int field64;
    int field68;
} tQueue2Z14tHUDPROMQ_NODEUi8;

extern void *memset(void *dst, int value, unsigned int size);

void Reset__t5Queue2Z14tHUDPROMQ_NODEUi8(tQueue2Z14tHUDPROMQ_NODEUi8 *self) {
    self->field60 = 0;
    self->field64 = 0;
    self->field68 = 0;
    memset(self, 0, 0x60);
}
