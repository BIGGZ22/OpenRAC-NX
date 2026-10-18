typedef struct {
    unsigned char pad[0x14];
} tMemcardSlot;

extern tMemcardSlot memcard_region[];

extern void sceCdReadClock(void *rtc);
extern void sceScfGetLocalTimefromRTC(void *rtc);

void MEMCARD_SetHeaderCreationDateDetail__FUi(unsigned int slot) {
    unsigned char t[8];
    unsigned char *d;

    sceCdReadClock(t);
    sceScfGetLocalTimefromRTC(t);
    if (t[0] != 0) {
        t[7] = 0;
        t[5] = 1;
        t[6] = 1;
    }
    d = (unsigned char *)&memcard_region[slot];
    d[0x78] = t[5];
    d[0x79] = t[6];
    d[0x7A] = t[7];
}
