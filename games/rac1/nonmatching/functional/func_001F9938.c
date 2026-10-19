/* functional: high. fcheck: unchecked. Countdown timer tick on a 16-bit counter: a counter already at 0 returns 1 and is left alone; otherwise the counter (clamped up to 1 first, so negatives count as 1) is decremented and stored, returning 0 while it is still above 0 and 2 on the tick that reaches 0. Differential test: 9 counter values (0, 1, 2, 3, 100, -1, -100, 32767, -32768) agree with retail in an EE emulator. */
int func_001F9938(short *timer) {
    int t = *timer;
    if (t == 0) {
        return 1;
    }
    if (t < 1) {
        t = 1;
    }
    t--;
    *timer = (short)t;
    return t > 0 ? 0 : 2;
}
