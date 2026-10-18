extern int mcptrans_level;
extern int mcptrans_level_index;

void MCPTRANS_SetLevel__F16LEVEL_IDENTIFIER(int level) {
    mcptrans_level = level;
    mcptrans_level_index = 0;
    if (level != 0) {
        unsigned int idx = level - 1;
        mcptrans_level_index = idx;
        if (idx >= 11) {
            mcptrans_level_index = 0;
        }
    }
}
