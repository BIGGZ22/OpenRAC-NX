/* functional: high. __umoddi3: unsigned 64-bit remainder (retail is the libgcc shift-and-subtract routine on 16-bit halves that stores the remainder through a stack slot and returns it); the C % operator gives the same remainder for every input; divisor 0 traps in retail (break 7), here the host division faults the same way. */
typedef unsigned long u64_;
u64_ func_0011F4F8(u64_ n, u64_ d) {
    return n % d;
}
