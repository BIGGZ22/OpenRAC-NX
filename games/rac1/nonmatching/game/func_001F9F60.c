/* FastCountChars: sums a byte buffer two bytes per pass. The loop always runs
   at least once and keeps going while the count stays positive after each
   step of two. */
int func_001F9F60(unsigned char *p, int n) {
    int sum = 0;
    do {
        int a = p[0];
        int b = p[1];
        p += 2;
        sum += a;
        n -= 2;
        sum += b;
    } while (n > 0);
    return sum;
}
