typedef struct {
    char pad[0x20D0];
    unsigned int flags;
    unsigned short depth_min;
    unsigned short depth_max;
    int depth_offset;
} tGU_STATE;

extern tGU_STATE gu_state;
extern void DepthRange__Q34scee3Psp2GuUsUs(unsigned short lo, unsigned short hi);

void DepthOffset__Q34scee3Psp2Gui(int offset) {
    gu_state.depth_offset = offset;
    DepthRange__Q34scee3Psp2GuUsUs(gu_state.depth_min, gu_state.depth_max);
}
