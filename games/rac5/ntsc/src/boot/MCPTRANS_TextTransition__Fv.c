extern unsigned int mcptrans_textState;

int MCPTRANS_TextTransition__Fv(void) {
    return (mcptrans_textState ^ 3u) < 1u;
}
