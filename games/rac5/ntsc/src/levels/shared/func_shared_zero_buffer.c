/* Both globals point to level-local records; their addresses vary by module. */
extern void *level_data_a;
extern void *level_data_b;
extern void *memset(void *, int, unsigned int);

void func_shared_zero_buffer(void) {
    void *dst = *(void **)((char *)level_data_a + 12);
    unsigned int count = *(unsigned int *)((char *)level_data_b + 0x48);
    memset(dst, 0, ((count + 31U) & ~31U) >> 3);
}
