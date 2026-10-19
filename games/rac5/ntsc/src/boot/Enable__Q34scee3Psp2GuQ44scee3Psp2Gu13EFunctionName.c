typedef struct {
    char pad[0x20D0];
    unsigned int flags;
} tGU_STATE;

extern tGU_STATE gu_state;

void Enable__Q34scee3Psp2GuQ44scee3Psp2Gu13EFunctionName(unsigned int f) {
    gu_state.flags |= (1u << f);
}
