/* Near miss: 160 against 176, short by exactly the two address materialisations.
   Retail at 0x003973F8, level 11 (176 bytes, 1 place) spawns the object through the shared
   allocator and fills it from the owner:

     int f(char *a0, char *a1, float f12, float f13)
     {
         char *s1 = a0, *s2 = a1;
         char *s0 = func_3224B0(0x12B2);
         char *p;

         if (s0 == 0) return 0;
         *(int *)(s0 + 0xB8) = (int)s1;
         *(unsigned char *)(s0 + 0x30) = 0xFF;
         *(u128 *)(s0 + 0x10) = *(u128 *)(s1 + 0x10);
         *(u128 *)(s0 + 0xF0) = *(u128 *)(s1 + 0xF0);
         func_300D78(s0 + 0xC0);
         p = *(char **)(s0 + 0x68);
         *(float *)(p + 0x28) = f13;
         *(int *)(p + 0x10) = (int)s1;
         *(int *)(p + 0x24) = (int)s2;
         *(float *)(p + 0x20) = f12;
         return 1;
     }

   probes "as measured": `u128` via `__attribute__((mode(TI)))` does reach `lq`/`sq` here (both
   copies come out as one `lq` + one `sq`, not the four `ldl`/`ldr` + `sdl`/`sdr` pairs a plain
   16-byte struct assignment gives), and the four callee-saved moves (`$s1`=a0, `$s2`=a1, and the
   two floats into `$f21`/`$f20`), the allocator call and the four sub-object stores all agree.

   The 16 bytes are four `addiu`: retail materialises `$a0=s0+0x10`, `$v1=s1+0x10`, `$a1=s0+0xF0`
   and `$v1=s1+0xF0` and then uses offset 0 in the `lq`/`sq`; this cc1 folds the +0x10 and +0xF0
   straight into the `lq`/`sq` offsets (`lq $v0,0x10($s1)` / `sq $v0,0x10($s0)`), which is two
   words shorter per copy. The float registers also come out swapped (`$f21`=f13 / `$f20`=f12
   against retail's `$f21`=f12 / `$f20`=f13) and the scheduler moves the +0x20 store first, so the
   four stores are emitted in a different order. */
