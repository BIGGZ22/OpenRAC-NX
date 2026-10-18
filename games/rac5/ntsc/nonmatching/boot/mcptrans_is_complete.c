/* Candidate only: the full-program build and compiler profile are not qualified. */
extern unsigned char transition_active;
extern unsigned char completion_flag;
extern int transition_state;

int MCPTRANS_IsComplete__Fv(void) {
    if (transition_active == 0) return 1;
    if (transition_state == 3) return 1;
    return completion_flag;
}
