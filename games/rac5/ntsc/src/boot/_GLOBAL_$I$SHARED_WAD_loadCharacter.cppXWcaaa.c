extern void static_init_target(int initialize, int priority);

void global_static_init(void) __asm__("_GLOBAL_$I$SHARED_WAD_loadCharacter.cppXWcaaa");

void global_static_init(void) {
    static_init_target(1, 65535);
}
