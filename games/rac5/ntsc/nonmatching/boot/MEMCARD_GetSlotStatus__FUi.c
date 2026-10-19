typedef struct {
    unsigned char pad[0x50];
    unsigned char slot_status[1];
} tMEMCARD_SLOT_STATUS;

extern tMEMCARD_SLOT_STATUS memcard_region;

unsigned char MEMCARD_GetSlotStatus__FUi(unsigned int slot) {
    return memcard_region.slot_status[slot];
}
