/* Earlier candidate: 180 bytes vs. retail's 176; floating-point register order differed.
 * The symbolic global stores below compile, but need a fresh placement check. */
extern float D_L00_001BD370 __attribute__((sda));
extern float D_L00_001B24DC __attribute__((sda));
extern volatile float D_L00_001B24E4;
extern volatile float D_L00_001B24E8;
extern void func_002EF080(int);
void func_L00_00368F58(int force)
{
    float value = 0.62f / D_L00_001BD370;
    float one = 1.0f;
    float four = 4.0f;
    if (value < one)
        value = one;
    if (value > four)
        value = four;
    if (force)
        value = one;
    {
        float base = D_L00_001B24DC;
        float fourValue = value * four;
        float sixValue = value * 6.0f;
        D_L00_001B24DC = base * sixValue;
        D_L00_001B24E4 = base * fourValue;
        D_L00_001B24E8 = (base + base) * value;
    }
    func_002EF080(force);
}
