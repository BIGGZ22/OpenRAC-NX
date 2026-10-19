#include "types.h"
extern s32 vi_buf_end_put(s32, s32) __asm__("FUN_0023bf18");
void video_dec_end_put(s32 video_dec, s32 bytes) __asm__("FUN_0023cc10");

/* The byte count goes through untouched: retail only adds 0x48 to $a0 in the jal's delay slot. */
void video_dec_end_put(s32 video_dec, s32 bytes) {
    vi_buf_end_put(video_dec + 0x48, bytes);
}

extern void func_0023CC10(s32 video_dec, s32 bytes) __attribute__((alias("FUN_0023cc10")));
