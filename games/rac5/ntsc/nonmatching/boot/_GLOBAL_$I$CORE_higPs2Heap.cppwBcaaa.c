extern void static_init_target_01E8FDB8(int initialize, int priority);

void global_static_init(void) __asm__("_GLOBAL_$I$CORE_higPs2Heap.cppwBcaaa");

void global_static_init(void) {
    static_init_target_01E8FDB8(1, 65535);
}
