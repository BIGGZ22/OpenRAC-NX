/* snd_PrepareReturnBuffer 0x01E94A68..0x01E94ACC (100 bytes) */
extern int gAwaitingInts;     /* 0x01EF6A7C */
extern unsigned int gCommBusy; /* 0x01EF6A78 */
extern void SyncDCache(void *start, void *end);

void snd_PrepareReturnBuffer(int *buffer, int count) {
    int size;

    gAwaitingInts = count;
    size = count * 4;
    gCommBusy = (unsigned int)buffer;
    *(int *)(size + (int)buffer + 4) = 0;
    buffer[0] = 0;
    SyncDCache(buffer, (char *)buffer + 3);
    size += 4;
    buffer = (int *)((char *)buffer + size);
    SyncDCache(buffer, (char *)buffer + 3);
}
