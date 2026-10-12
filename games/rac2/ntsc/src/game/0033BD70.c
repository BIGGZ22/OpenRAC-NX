

#ifndef RAC2_T_S32_F09F721E0
#define RAC2_T_S32_F09F721E0
typedef int s32_F09f721e0;
#endif

#ifndef RAC2_T_F32_F09F721E0
#define RAC2_T_F32_F09F721E0
typedef float f32_F09f721e0;
#endif

extern s32_F09f721e0 func_00336E40 (s32_F09f721e0, s32_F09f721e0);

extern s32_F09f721e0 func_00336DA8 (s32_F09f721e0, s32_F09f721e0);

extern void func_0033ABD8 (s32_F09f721e0, s32_F09f721e0, const void *);

extern s32_F09f721e0 func_00288FB8 (s32_F09f721e0);

extern void func_0033B050 (s32_F09f721e0, s32_F09f721e0, s32_F09f721e0, s32_F09f721e0);

extern void func_0033B030 (s32_F09f721e0, f32_F09f721e0, f32_F09f721e0, f32_F09f721e0, f32_F09f721e0, f32_F09f721e0, f32_F09f721e0);

extern void func_0033B0A8 (s32_F09f721e0, f32_F09f721e0);

extern void func_0033BE98 (s32_F09f721e0, s32_F09f721e0);

extern s32_F09f721e0 D_L04_001AE040 __attribute__ ((sda));

void func_0033BD70 (s32_F09f721e0 self, s32_F09f721e0 arg)
{
  s32_F09f721e0 q;
  s32_F09f721e0 p;
  s32_F09f721e0 a, b;
  *(s32_F09f721e0 *) self = arg;
  if (arg != 0)
    {
      p = func_00336DA8 (16, func_00336E40 (arg, arg));
      *(s32_F09f721e0 *) (self + 684) = p;
      ((s32_F09f721e0 *) p)[1] = 0;
      ((s32_F09f721e0 *) p)[2] = 0;
      ((s32_F09f721e0 *) p)[3] = 0;
      ((s32_F09f721e0 *) p)[0] = 0;
    }
  q = self + 8;
  ((f32_F09f721e0 *) *(s32_F09f721e0 *) (self + 684))[0] = 255.0f;
  ((f32_F09f721e0 *) *(s32_F09f721e0 *) (self + 684))[1] = 195.0f;
  func_0033ABD8 (q, arg, &D_L04_001AE040);
  a = func_00288FB8 (12773);
  b = func_00288FB8 (11598);
  func_0033B050 (q, a, b, func_00288FB8 (11599));
  func_0033B030 (q, 0.0f, -143.0f, 0.0f, 114.0f, 0.0f, 140.0f);
  func_0033B0A8 (q, 0.65f);
  *(s32_F09f721e0 *) (self + 680) = 0;
  *(s32_F09f721e0 *) (q + 668) = 0;
  func_0033BE98 (self, 0);
}
