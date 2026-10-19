extern int MC_STATUS;
extern int MC_SEMA;
extern int MC_SEMA2;
extern int MC_OUTBUF;
extern int PollSema(int);
extern int WaitSema(int);
extern int SignalSema(int);

int sceMcSync(int mode, int *code, int *result) {
    int r;
    unsigned int wait;
    if (MC_STATUS == 0)
        return -1;
    r = PollSema(MC_SEMA2);
    if (r < 0) wait = 1; else wait = 0;
    if (mode == 0 && wait) {
        WaitSema(MC_SEMA2);
        wait = 0;
    }
    wait = (wait < 1);
    if (code != 0)
        *code = MC_STATUS;
    if (wait) {
        MC_STATUS = 0;
        if (result != 0)
            *result = MC_OUTBUF;
        SignalSema(MC_SEMA);
    }
    return wait;
}
