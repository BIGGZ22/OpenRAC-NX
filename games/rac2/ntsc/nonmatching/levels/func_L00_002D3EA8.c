/* 0 of 27 places match - last line: 00_aranos_tutorial: 132 bytes, expected 136
   The sibling of the matched func_L00_002D3B00: mode 0 blends the object's field 8 with a
   callee float, modes outside 0..2 return 0.0f, else the object is copied into a 16-byte
   local, filled through the per-level dispatcher, and the same two floats (field 8 read
   after the call, the callee float) are combined.  Every instruction matches but one: this
   cc1 fills the `beqz` delay slot with `daddu $a0,$sp,$zero` and sinks the `sq` into the
   jal's slot, retail leaves a `nop` and keeps `lq`/`sq` adjacent.  Tried and identical:
   a V4 local, `float local[4]`, `local.f2` through a float struct, and the copy through a
   separate pointer. */

#include "include_asm.h"

typedef int V4_F6a70b950;

extern float func_2EE268(char *);
extern float func_2EE7A8(float, float);
extern void func_2EE5D8(void *, void *, char *);

float func_L00_002D3EA8(char *in)
{
    char *p = (char *)&D_00189E20_11;
    int mode = *(unsigned char *)(p + 8899);
    V4_F6a70b950 local;

    if (mode == 0)
        return func_2EE7A8(*(float *)(in + 8), func_2EE268(in));
    if (mode < 0)
        return 0.0f;
    if (mode >= 3)
        return 0.0f;
    local = *(V4_F6a70b950 *)in;
    func_2EE5D8(&local, &local, p + 64);
    return func_2EE7A8(*(float *)((char *)&local + 8), func_2EE268((char *)&local));
}
