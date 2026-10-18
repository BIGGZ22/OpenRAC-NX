typedef struct {
    unsigned char bytes[20];
} tMEMCARD_SLOT_HEADER;

extern tMEMCARD_SLOT_HEADER memcard_slot_headers[];

tMEMCARD_SLOT_HEADER *MEMCARD_GetSlotHeaderDetail__FUi(unsigned int slot) {
    return &memcard_slot_headers[slot];
}
