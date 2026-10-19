extern void static_init_target_01E8F918(int initialize, int priority);

void global_static_init(void) __asm__("_GLOBAL_$I$LIBHIG_modulemgr.cppkdcaaa");

void global_static_init(void) {
    static_init_target_01E8F918(1, 65535);
}
