/* Near miss from src/levels/shared/L00_0037E898.c: 252 bytes; 240-byte candidate; 0/27. Candidate models state 0x37 gating and state transitions 0x40/0x41. */
extern char D_00193E20[];
extern void func_0029F390(void *object);
extern void func_002A7470(void *object, int mode, int flags, int event);

void func_L00_0038B6F8(char *target)
{
    char *globals = D_00193E20;
    char *source;
    unsigned char state;
    unsigned char oldState;

    if (*(int *)(globals + 0x2294) != 0x37)
        return;
    source = *(char **)(globals + 0x2290);
    oldState = *(unsigned char *)(source + 0x43);
    state = 0x0B;
    if (oldState == 0x40)
        state = 0x13;
    if (oldState == 0x41)
        state = 0x12;
    if (state == 0x12 || state == 0x13) {
        *(unsigned char *)(target + 0x43) = state;
        *(unsigned char *)(target + 0x42) = state;
        *(float *)(target + 0x44) = *(float *)(source + 0x44);
        *(unsigned char *)(target + 0x40) = *(unsigned char *)(source + 0x40);
        *(unsigned char *)(target + 0x41) = *(unsigned char *)(source + 0x41);
        if (*(unsigned char *)(source + 0x42) != *(unsigned char *)(source + 0x43))
            *(unsigned char *)(target + 0x40) = *(unsigned char *)(source + 0x41);
        func_0029F390(target);
        return;
    }
    state = *(unsigned char *)(target + 0x43);
    if (state == 0x0B)
        func_002A7470(target, 0, 0, 7);
    else if (state == 0x13 || state == 0x12)
        func_002A7470(target, 1, 0, 0x11);
}
