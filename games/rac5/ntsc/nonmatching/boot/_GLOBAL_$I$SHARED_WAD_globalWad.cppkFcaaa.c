extern void static_init_target_01E90A68(int initialize, int priority);

void global_static_init(void) __asm__("_GLOBAL_$I$SHARED_WAD_globalWad.cppkFcaaa");

void global_static_init(void) {
    static_init_target_01E90A68(1, 65535);
}
