/* Not attempted - body read in full, no C written: 396 bytes, 27 places, the float work
   around two 16-byte block copies.

   Shape: s0 = 0x00189E20. Bails out when *(u8 *)(s0+0x22C3) == 2 or when
   *(float *)(s0+0x174) < *(float *)(s0+0x264) * 0.05f (0x3D4CCCCD), then
   f0 = func_2EF1E0(func_2EE7A8(*(float *)(s0+0x110), *(float *)(s0+0x114)),
                    *(float *)(s0+0x98)) and bails out again when 0.52380955f (0x3F060A93)
   is below f0. It then copies 16 bytes from s0+0x80 (the hero position block) onto the
   stack (`lq`/`sq` - only a 16-byte aggregate reaches that pair; no C shape found in the
   time available), adds
   func_2EE6E0(*(float *)(s0+0x98)) * *(float *)(s0+0x264) * 0.75f to the first float and
   func_2EE6F8(*(float *)(s0+0x98)) * *(float *)(s0+0x264) * 0.75f to the second, copies
   the 16 bytes again to a second slot (another lq/sq pair), adds 0.47f (0x3EF0A3D7) to the
   first copy's third float and subtracts 1.0f from the second copy's third float, then
   calls func_2DE4B0(&first, &second, 2, *(int *)(s0+0x2290), 0) and returns 0 when it
   returns 0. Finally t = 0x001BF228 - *(float *)(s0+0x88): returns 0 when t < 0.37f
   (0x3EBD70A4), else returns -1.0f < t. Data: the float at 0x001BF228 is level-local
   (>= 0x001A7C80), everything else is boot-resident. */
