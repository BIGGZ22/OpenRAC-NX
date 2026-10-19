typedef struct {
    char pad00[0x20D0];
    unsigned int functionMask;
} tGuState;

extern tGuState gu_state; /* 0x01F75380 */

void Disable__Q34scee3Psp2GuQ44scee3Psp2Gu13EFunctionName(unsigned int mask) {
    gu_state.functionMask &= ~(1u << mask);
}
