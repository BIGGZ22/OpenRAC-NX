/* RADSetMemory 0x01EAB4A8..0x01EAB4C0 */
extern void *rad_memory_base;
extern void *rad_memory_size;
void RADSetMemory(void *base, void *size) {
    rad_memory_base = base;
    rad_memory_size = size;
}
