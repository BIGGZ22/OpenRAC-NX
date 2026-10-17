/* functional: high. Nothing differs: the int absolute value, as the assembly's branch over the negate. Retail's negate is a trapping subtract, so INT_MIN traps there and wraps here; no caller reaches that input (they pass differences of small counters). */
int func_001F9B70(int x) {
    if (x < 0) {
        x = (int)(0u - (unsigned int)x);
    }
    return x;
}
