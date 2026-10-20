/* 0 of 4 places match - candidate is 152 bytes against 164.
   The call sequence, the arguments and the clamp are all right; two differences.
   (1) Retail keeps the travel in $f22 across the `func_2F2270` call and saves f20/f21/f22 (frame
   0x50, saves at 0x30/0x38/0x40); this cc1 keeps it in $f21, saves only two floats (frame 0x40) and
   the `sub.s` reads `$f21,$f20` instead of `$f22,$f20`.
   (2) The branch layout is mirrored: retail `c.olt.s $f21,$f20; bc1f` with `mov.s $f22,$f20` and
   `mov.s $f20,$f21` in the two delay slots, this cc1 `c.olt.s $f20,$f21; bc1t`.
   Callees, in retail order: func_L02_002F1FA0(local, a1, object), func_L02_002F20B8(local),
   func_L02_002F2270(local, local, low), func_L02_002F1F88(out, object, local). */

float func_L02_00321C30(char *object, int arg1, float step)
{
    char local[16];
    char out[16];
    float travel;
    float low;

    func_L02_002F1FA0(local, arg1, object);
    travel = func_L02_002F20B8(local);
    if (step < travel)
        low = step;
    else if (travel < -step)
        low = -step;
    else
        low = travel;
    func_L02_002F2270(local, local, low);
    func_L02_002F1F88(out, object, local);
    return travel - low;
}
