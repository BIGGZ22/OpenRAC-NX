/* RAC5 boot — sceSifAllocIopHeap, retail 0x01ED7E10..0x01ED7E7C (108 bytes).
 * Authored 2026-10-17, wave 11 lot c1 from the lombyte source
 * sdk/rpc/sce_sif_alloc_iop_heap.c re-pointed at RAC5 data: heap base
 * 0x01EFEFF4, client 0x01FE7140, size word 0x01FE71C0, result 0x01FE7180. */
typedef struct {
    unsigned char pad_0[0x24];
    void *server;
} SifClient;

extern int IOP_HEAP_BASE[];
extern SifClient IOP_HEAP_CLIENT;
extern int IOP_HEAP_SIZE[];
extern int IOP_HEAP_RESULT[];
extern int sceSifCallRpc(SifClient *, int, int, void *, int, void *, int, void *, void *);

void *sceSifAllocIopHeap(int size) {
    if (IOP_HEAP_BASE[0] < 0) {
        return 0;
    }
    IOP_HEAP_SIZE[0] = size;
    if (sceSifCallRpc(&IOP_HEAP_CLIENT, 1, 0, IOP_HEAP_SIZE, 4, IOP_HEAP_RESULT, 4,
                      0, 0) < 0) {
        return 0;
    }
    return (void *)IOP_HEAP_RESULT[0];
}
