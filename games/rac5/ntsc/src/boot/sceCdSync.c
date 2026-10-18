/* sceCdSync — RAC2 src/core/00124AF0.c func_00124C28, adapted to the RAC5
   retail shape (RAC5 branch 0x01EC2A78): the wait calls DelayThread(0xFA0)
   and sceSifCheckStatRpc. RAC5 data: 0x01EFC710 (cb_thid), 0x01EFC734
   (pending flag), 0x01EFD8D0 (client). Format string at 0x01F26458. */

extern int D_01EFC710;
extern int D_01EFC734;
extern int D_01EFD8D0[];
extern char S_01F26458[];

extern int scePrintf(char *, ...);
extern int DelayThread(int);
extern int sceSifCheckStatRpc(void *);

int sceCdSync(int arg0) {
    if (arg0 == 0) {
        if (D_01EFC710 > 0) {
            scePrintf(S_01F26458);
        }
        while (D_01EFC734 != 0 || sceSifCheckStatRpc(D_01EFD8D0) != 0) {
            DelayThread(0xFA0);
        }
        return 0;
    }
    if (D_01EFC734 != 0 || sceSifCheckStatRpc(D_01EFD8D0) != 0) {
        return 1;
    }
    return 0;
}
