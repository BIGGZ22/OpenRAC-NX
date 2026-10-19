/* sceCdNcmdDiskReady — RAC2 src/core/00124AF0.c func_00124AF0, adapted to the
   RAC5 retail shape: the guard is _sceCd_ncmd_prechk (0x01EC2888), the RPC is
   sceSifCallRpc (0x01ED45B0) and the semaphore release is SignalSema.
   RAC5 data (lui 0x1F0 + signed offsets, base 0x01F00000):
     -0x2730 -> 0x01EFD8D0 (client), -0x3880 -> 0x01EFC780 (reply),
     -0x38D8 -> 0x01EFC728 (semaphore). */

extern int D_01EFD8D0[];
extern int D_01EFC780[];
extern int D_01EFC728;

extern int _sceCd_ncmd_prechk(int);
extern int sceSifCallRpc(void *, unsigned int, int, void *, void *, void *, int, void *, void *);
extern int SignalSema(int);

int sceCdNcmdDiskReady(void) {
    int r;

    if (_sceCd_ncmd_prechk(2) == 0) {
        return 0;
    }
    if (sceSifCallRpc(D_01EFD8D0, 0xE, 0, 0, 0, D_01EFC780, 4, 0, 0) < 0) {
        SignalSema(D_01EFC728);
        return 0;
    }
    r = *(int *)((unsigned int)D_01EFC780 | 0x20000000);
    SignalSema(D_01EFC728);
    return r;
}
