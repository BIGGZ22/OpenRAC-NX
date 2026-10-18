typedef struct {
    char pad[0x20D0];
    unsigned int flags;
    unsigned short depth_min;
    unsigned short depth_max;
} tGU_STATE;

extern tGU_STATE gu_state;

void DepthRange__Q34scee3Psp2GuUsUs(unsigned short lo, unsigned short hi) {
    gu_state.depth_min = lo;
    gu_state.depth_max = hi;
}
