extern int asyncCmdSync(int command, unsigned int *argument, unsigned int *result);

void higIoWaitAsync__FiPUi(int handle, unsigned int *result) {
    unsigned int status;
    asyncCmdSync(0, result, &status);
    *result = status;
}
