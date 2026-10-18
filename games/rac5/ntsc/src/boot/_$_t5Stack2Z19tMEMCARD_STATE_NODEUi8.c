typedef struct {
    unsigned char base[0x80];
    int field80;
} tStack2Z19tMEMCARD_STATE_NODEUi8;

extern void __builtin_delete(void *pointer);

void _$_t5Stack2Z19tMEMCARD_STATE_NODEUi8(tStack2Z19tMEMCARD_STATE_NODEUi8 *self, unsigned int deleting) {
    if (deleting & 1) {
        __builtin_delete(self);
    }
}
