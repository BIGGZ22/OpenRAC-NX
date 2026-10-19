extern void static_init_target_01E932D8(int initialize, int priority);

void global_static_init(void) __asm__("_GLOBAL_$I$ENGINE_SndScream.cppbycaaa");

void global_static_init(void) {
    static_init_target_01E932D8(1, 65535);
}
