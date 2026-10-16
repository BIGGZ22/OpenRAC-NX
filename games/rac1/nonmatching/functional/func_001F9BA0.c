/* functional: high. The least of three signed ints (retail: two pminw on the low words). */
int func_001F9BA0(int a, int b, int c) {
    int m = a < b ? a : b;
    return m < c ? m : c;
}
