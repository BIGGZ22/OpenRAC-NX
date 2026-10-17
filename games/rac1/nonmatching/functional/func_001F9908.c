/* functional: high. Written from the assembly (no C existed). FastDecTimer(int *timer): returns 1 if the timer was already 0 (idle, nothing written); otherwise stores max(t, 1) - 1 and returns 0 while it is still above 0, 2 when it has just run out. The pmaxw is a signed 32-bit max with 1. */
int func_001F9908(int *timer)
{
    int t = *timer;

    if (t == 0) return 1;
    if (t < 1) t = 1;
    t = t - 1;
    *timer = t;
    if (t > 0) return 0;
    return 2;
}
