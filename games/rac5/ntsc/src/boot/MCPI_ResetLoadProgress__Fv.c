extern int mcpi_load_progress;

void MCPI_ResetLoadProgress__Fv(void) {
    mcpi_load_progress = 0x7FFFFFFF;
}
