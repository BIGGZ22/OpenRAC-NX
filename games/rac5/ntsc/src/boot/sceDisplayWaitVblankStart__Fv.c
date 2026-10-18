extern int sceDisplayGetVcount__Fv(void);
extern void DelayThread(int usec);

void sceDisplayWaitVblankStart__Fv(void) {
    int count = sceDisplayGetVcount__Fv();

    while (sceDisplayGetVcount__Fv() == count) {
        DelayThread(1000);
    }
}
