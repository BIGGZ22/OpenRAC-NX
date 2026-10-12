/* 4 of 11 instructions differ (36 bytes). Retail passes the 64-bit return of func_3246F8 straight into func_3246A0 (daddu, no conversion). Any C prototype narrower than unsigned long makes GCC sign-extend it (dsll32/dsra32); with unsigned long, func_3246A0 must then take unsigned long, which breaks its own 32-bit body (sll, not dsll). Missing: a way to declare func_3246A0 so the caller passes the full register while its body keeps 32-bit index math. */
extern unsigned long func_L01_003246F8(void);

extern void func_L01_003246A0(unsigned long);

void func_L01_003246D0(void) {
    func_L01_003246A0(func_L01_003246F8());
}
