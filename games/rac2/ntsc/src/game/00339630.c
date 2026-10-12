

#ifndef RAC2_T_S32_FEE27D7BC
#define RAC2_T_S32_FEE27D7BC
typedef int s32_Fee27d7bc;
#endif

#ifndef RAC2_T_F32_FEE27D7BC
#define RAC2_T_F32_FEE27D7BC
typedef float f32_Fee27d7bc;
#endif

extern s32_Fee27d7bc func_00336E40_2 (s32_Fee27d7bc, s32_Fee27d7bc);

extern s32_Fee27d7bc func_00336DA8_2 (s32_Fee27d7bc, s32_Fee27d7bc);

extern void func_0033ABD8_2 (s32_Fee27d7bc, s32_Fee27d7bc, const void *);

extern void func_0033B030_2 (s32_Fee27d7bc, f32_Fee27d7bc, f32_Fee27d7bc, f32_Fee27d7bc, f32_Fee27d7bc, f32_Fee27d7bc, f32_Fee27d7bc);

extern void func_003361E0 (s32_Fee27d7bc, const void *, s32_Fee27d7bc);

extern s32_Fee27d7bc func_00335E20_2 (s32_Fee27d7bc);

extern void func_00336390 (s32_Fee27d7bc, const void *, s32_Fee27d7bc);

extern void func_00339798 (s32_Fee27d7bc);

extern char D_L04_001ADF08[];

extern s32_Fee27d7bc D_L04_001A904C;

extern s32_Fee27d7bc D_L04_001ADEF8 __attribute__ ((sda));

void func_00339630 (s32_Fee27d7bc self, s32_Fee27d7bc arg)
{
  s32_Fee27d7bc p;
  *(s32_Fee27d7bc *) self = arg;
  if (arg != 0)
    {
      p = func_00336DA8_2 (16, func_00336E40_2 (arg, arg));
      *(s32_Fee27d7bc *) (self + 756) = p;
      ((s32_Fee27d7bc *) p)[1] = 0;
      ((s32_Fee27d7bc *) p)[2] = 0;
      ((s32_Fee27d7bc *) p)[3] = 0;
      ((s32_Fee27d7bc *) p)[0] = 0;
    }
  ((f32_Fee27d7bc *) *(s32_Fee27d7bc *) (self + 756))[0] = 256.0f;
  ((f32_Fee27d7bc *) *(s32_Fee27d7bc *) (self + 756))[1] = 212.0f;
  *(s32_Fee27d7bc *) (self + 760) = 0;
  func_0033ABD8_2 (self + 8, arg, &D_L04_001ADEF8);
  func_0033B030_2 (self + 8, 64.0f, -164.0f, 10.0f, 136.0f, 10.0f, 99.0f);
  func_003361E0 (self + 680, D_L04_001ADF08, arg);
  p = func_00335E20_2 (self + 680);
  *(s32_Fee27d7bc *) p = 0x70FFFEED;
  func_00336390 (self + 680, D_L04_001A904C + 0x8710, 214);
  func_00339798 (self);
  *(s32_Fee27d7bc *) (self + 768) = 0;
}
