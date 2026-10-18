/* Candidate only: the full-program build and compiler profile are not qualified. */
extern unsigned char fade_flag;

int MCPTRANS_ShouldFade__Fv(void) { return fade_flag; }
