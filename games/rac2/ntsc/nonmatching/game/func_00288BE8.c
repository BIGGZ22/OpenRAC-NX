/* from src/game/00288BB8.c: C that compiles but does not link at its places yet */
/* launches one queued request using mode-dependent word arguments and resets the queue after a negative submission status */
s32 func_00288BE8(BarlowUtilityQueueView *queue) {
    if (queue->field38 != 0) {
        s32 request[2];
        s32 status;
        s32 mode;
        queue->field3c = 0;
        queue->field44 = 1;
        request[0] = 0;
        request[1] = 0;
        func_00288A00(queue, &request[0], &request[1]);
        status = 0;
        mode = 1;
        if ((u32)(D_L04_001A8F00 - 1) < 2) mode = 2;
        queue->field40 = mode;
        if (request[1] == 0)
            status = func_002B49E0(1, mode, 0, request[0], 0);
        else if (request[1] == 1)
            status = func_002B49E0(2, mode, request[0], 0, 0);
        if (status < 0) func_L00_002F4180(queue);
        return status == 0;
    }
    return 0;
}
