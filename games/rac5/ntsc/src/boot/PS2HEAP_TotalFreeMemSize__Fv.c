typedef struct {
    char pad[0x14];
    int totalFreeSize;
} tPs2Heap;

extern tPs2Heap gPs2Heap; /* 0x01FD0818 */

int PS2HEAP_TotalFreeMemSize__Fv(void) {
    return gPs2Heap.totalFreeSize;
}
