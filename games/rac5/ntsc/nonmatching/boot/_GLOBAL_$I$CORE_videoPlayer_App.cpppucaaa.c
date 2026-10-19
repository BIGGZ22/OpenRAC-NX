extern void static_init_target_01E94448(int initialize, int priority);

void global_static_init(void) __asm__("_GLOBAL_$I$CORE_videoPlayer_App.cpppucaaa");

void global_static_init(void) {
    static_init_target_01E94448(1, 65535);
}
