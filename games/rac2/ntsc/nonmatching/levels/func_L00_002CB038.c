/* Not attempted - body read in full, no C written (walled behind the same $gp block as
   func_L00_002CB2D8): the front-end screen loader at symbol_addrs's `MenuScreenLoad`
   (0x002CB038, 476 bytes, 27 places).

   Shape: s0 = 0x00189E20. `if (*(int *)(s0+0x1248) < 0) { *(int *)(s0+0x1450) = 0; return; }`
   then, when the flag byte at 0x001A7A80 + *(int *)(s0+0x1248) has bit 0x20 AND
   *(int *)(s0+0x2294) != 0x31, it clears 0x1450 and returns. With a live object at 0x1450 it
   tests a distance (func_2EE2B0(s0+0x80, obj+0x10) > 30.0f), a 3-valued field at
   0x00138320, and `*(u16 *)(obj+0x34) & 0x1000` to decide whether to drop the object.
   The drop path raises the counter block exactly as func_L00_002CB2D8 does
   (u16 at 0x0018C750+0x1B0 clamped to 0xFFFE against the $gp value at 0x001A7390, an
   unsigned max on +0x1B4 from the plain absolute 0x001A7390, and
   `*(int *)(0x0018C750+0x1B8) |= (1 << *(int *)0x001A79F0) | 0x80000000`).
   The spawn path needs two calls with four float arguments each:
   func_2AE888(0x001B8FF0, 5, 1, 25.0f, *(float *)(0x001B8C80+0x158), 0x3F1C61AB, 0x3F9C61AB)
   and, if that returns 0, the same call with (s0, *(float *)(s0+0x98)); the result is
   stored at 0x1450. Blocking requirements: the $gp counter block (see the 002CB2D8
   near-miss) and the 0x001B8C80/0x001B8FF0 level data. */
