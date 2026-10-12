#include "common.h"
#include "structs.h"

extern void func_0012FAE8(int);

extern int r1_func_00128C90(void *);

extern int func_00130020(void *);

extern int r1_func_00129690(void *, int);

extern int r1_func_00129530(void *);

extern void r1_func_00129600(void *, int, int);

extern int func_0012FEC0(void *, int, int);

struct MpegFieldDec
{
  int unk0;
  int unk4;
  int unk8;
  unsigned char pad_C[0xA0];
  int unkAC;
  unsigned char pad_B0[0x24];
  int unkD4;
  unsigned char pad_D8[0x40];
  int unk118;
  int unk11C;
  int unk120;
  unsigned char pad_124[0x50];
  int unk174;
};

struct MpegHandle
{
  unsigned char pad_0[0x8];
  int unk8;
  unsigned char pad_C[0x34];
  struct MpegFieldDec *unk40;
};

extern int r1_func_00129530();

extern int r1_func_00129690();

extern void r1_func_00129600();

/* rac1-decomp src/core/0012AC80.c: func_0012C0A0 */
/* _decodeOrSkipField (libmpeg). Adapted from Lombyte (MIT) for PAL. */
int func_0012FEC0(void *handle, int arg1, int arg2)
{
  struct MpegHandle *arg0 = handle;
  struct MpegFieldDec *p;
  int decode;
  unsigned int ref;
  int want;
  long r;
  int ret;
  int gate;
  decode = 0;
  p = arg0->unk40;
  p->unk120 = 0;
  if ((arg2 == (-1)) || (arg1 < arg2))
  {
    decode = 1;
  }
  if (p->unk8 == 0)
  {
    arg0->unk8 = 0;
    p->unk8 = 1;
  }
  r = r1_func_00129690(p, 0);
  if ((r != 0) && (decode != 0))
  {
    r1_func_00129530(p);
  }
  p->unk120 = 1;
  r = r1_func_00128C90(p);
  if (r == 0)
  {
    func_00130020(arg0);
    p->unk0 = 1;
    return 0;
  }
  want = 2;
  if (p->unkD4 != 1)
  {
    want = 1;
  }
  if (p->unk174 != want)
  {
    return -1;
  }
  ref = r1_func_00129690(p, 1);
  gate = 0;
  if (ref != 0)
  {
    gate = 1;
  }
  ret = 0;
  if (gate != 0)
  {
    if (decode == 0)
    {
      goto out;
    }
    r = r1_func_00129530(p);
    if (r != 0)
    {
      ret = 1;
    }
  }
  out:
  r1_func_00129600(p, p->unk118, p->unk4);

  p->unk120 = 0;
  arg0->unk8 = p->unk118 - p->unkAC;
  p->unk118 = p->unk118 + 1;
  p->unk4 = (unsigned long long) (p->unk4 + 1);
  if (decode == 0)
  {
    func_0012FAE8((int)arg0);
  }
  return ret;
}
