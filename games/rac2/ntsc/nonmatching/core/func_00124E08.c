/* from src/core/00124C98.c: 736 bytes, 736 compiled; the callee-saved
 * allocation is a rotation of retail's over the same set of values
 * (client, buffer, init_mode, two 0x13 bases, the format and debug bases and
 * the reply buffer land in $s3/$s4/$s2/$s5/$s1/$s6/$s7/$fp here against
 * retail's $s1/$s2/$s3/$s4/$s5/$s6/$s7/$fp, and the counter base is copied
 * from a temporary into a callee-saved register instead of being materialised
 * there).
 *
 * sceCdInit (libcdvd.a cdvd000.o): take the S-command semaphore through
 * sceCdSyncS(1), publish the thread id in the owner slot and the client table,
 * bind the 0x80000592 server, send init_mode through the uncached buffer and
 * read the three reply words back through the EE uncached alias (| 0x20000000).
 * The control flow, the spin loops, the two scePrintf strings
 * ("Libcdvd bind err %d CD_Init %d", "Libcdvd Exit"), the switch on init_mode
 * and every relocation agree; the volatile aliases below reproduce retail's
 * store order inside the initialisation block.
 *
 * To unblock: the allocation is decided by the reference order of the values
 * live across the two calls and the loop; a form of the initialisation block
 * that makes the counter base (D_001363D0) and the reply buffer (D_00137580)
 * the low-priority values would settle it. rac1-decomp left the same function
 * (its func_001211B0) unmatched, so no C for it exists upstream. */

extern int func_00124C28(int);

extern void func_0011CF78(int);

extern int func_0011AB10(void);

extern int func_0011D450(void *, unsigned int, int);

extern void func_0011C820();

extern void func_0011CEC8(void *, int);

extern int func_0011D620(void *, unsigned int, int, void *, int, void *, int, void *, int);

extern void func_00124780(void);

extern void func_00124818(void);

extern void func_001248F8(void);

extern int r1_D_00136390;

extern int r1_D_001363C8;

extern int r1_D_00141850;

extern int r1_D_001363AC;

extern int r1_D_001363D0;

/* The printf re-reads the counter; a local copy would not be read again. */
extern volatile int r1_D_001363D0_read __asm__("r1_D_001363D0");

extern volatile int r1_D_001363A4;

/* The "= 0" store below is a different access in retail: the address is
   materialised again instead of being kept live. */
extern volatile int r1_D_001363A4_cleared __asm__("r1_D_001363A4");

extern volatile int r1_D_00141850_v __asm__("r1_D_00141850");

extern int r1_D_001363B4;

extern int r1_D_001363B8;

extern int r1_D_001363BC;

extern int r1_D_001363C0;

extern int r1_D_001363C4;

extern int r1_D_001363CC;

extern volatile int r1_D_001363A0;

extern volatile int r1_D_001363A8;

extern volatile int r1_D_001363AC_v __asm__("r1_D_001363AC");

extern int r1_D_00141A68;

extern int r1_D_00141AC0;

extern char r1_D_00137580[];

extern char r1_D_0013B268[];

extern char r1_D_0013B288[];

typedef struct {
    char pad[0x24];
    void *serve;
} SifRpcClientData;

int func_00124E08(int init_mode) {
    int ret;
    int r;
    int i;
    int stat;
    int a;
    int b;
    int owner;
    int count;
    char *rbuf;

    if (func_00124C28(1) != 0) {
        return 0;
    }
    func_0011CF78(0);
    owner = func_0011AB10();
    count = r1_D_001363D0 + 1;
    r1_D_001363A4 = 1;
    r1_D_00141850_v = owner;
    r1_D_001363BC = -1;
    r1_D_001363C0 = -1;
    r1_D_001363B8 = -1;
    r1_D_001363C8 = -1;
    r1_D_001363C4 = -1;
    rbuf = r1_D_00137580;
    r1_D_001363B4 = 0;
    r1_D_001363D0 = count;
    r1_D_001363CC = -1;
    while (1) {
        r = func_0011D450(&r1_D_00141A68, 0x80000592, 0);
        if (r < 0) {
            if (r1_D_00136390 > 0) {
                func_0011C820(r1_D_0013B268, r, r1_D_001363D0_read);
            }
            for (i = 0x100000; i != -1; i--) {
            }
            continue;
        }
        if (((SifRpcClientData *)&r1_D_00141A68)->serve != 0) {
            r1_D_00141AC0 = init_mode;
            r1_D_001363CC = 0;
            func_0011CEC8(&r1_D_00141AC0, 4);
            if (func_0011D620(&r1_D_00141A68, 0, 0, &r1_D_00141AC0, 4, rbuf, 0x10, 0, 0) < 0) {
                r1_D_001363A4 = 0;
                return 0;
            }
            break;
        }
        for (i = 0x100000; i != -1; i--) {
        }
    }
    stat = *(int *)((unsigned int)(rbuf + 0xC) | 0x20000000);
    b = *(int *)((unsigned int)(rbuf + 8) | 0x20000000);
    a = *(int *)((unsigned int)(rbuf + 4) | 0x20000000);
    ret = 1;
    if (stat == 0xFF) {
    } else if (stat == 0xFE) {
        r1_D_00136390 = ret;
    } else {
        if (a / 256 < 2 || b / 256 < 2) {
            ret = 2;
        }
    }
    r1_D_001363A4_cleared = 0;
    switch (init_mode) {
    case 0:
    case 1:
        break;
    case 5:
        if (r1_D_00136390 > 0) {
            func_0011C820(r1_D_0013B288);
        }
        func_00124818();
        r1_D_001363A8 = -1;
        r1_D_001363AC_v = -1;
        r1_D_001363A0 = -1;
        return ret;
    }
    func_00124780();
    func_001248F8();
    return ret;
}
