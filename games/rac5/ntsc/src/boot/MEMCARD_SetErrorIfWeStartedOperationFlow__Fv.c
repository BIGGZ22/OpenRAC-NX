extern unsigned char memcard_opFlowStarted;

int MEMCARD_SetErrorIfWeStartedOperationFlow__Fv(void) {
    if (memcard_opFlowStarted != 0)
        memcard_opFlowStarted = 0;
    return 0;
}
