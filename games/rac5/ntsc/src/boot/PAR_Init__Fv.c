typedef struct {
    char pad_00[0x08];
    int  field_08;
    char pad_0C[0x38 - 0x0C];
    int  field_38;
} tPAR_STATE;

extern tPAR_STATE par_state;

void PAR_Init__Fv(void) {
    par_state.field_08 = 0;
    par_state.field_38 = 0;
}
