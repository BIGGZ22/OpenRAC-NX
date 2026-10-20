/* 0 of 27 places match - last line: 00_aranos_tutorial: 192 bytes, expected 196
   One instruction short and the loop shape is close: retail walks a pointer array with
   a counter decremented *inside* the branch paths (`bnel`/`blezl` delay slots), which
   is what a `for (; a2 > 0; a2--)` with `continue`s produces - that form gives 48 of
   retail's 49 words here. The remaining difference is in the two `likely` branches
   (`blezl $s2` then `lw $a1,0x0($s0)` in its delay slot) and the counter direction:
   an index form with `a1[i]` gets closer than the pointer form but still 30 words off.
   What would unblock it: the exact spell-out of the branch-likely pair - the delay
   slot of `blezl` runs only when the branch is taken, so the source's null check must
   be ordered to match. */

typedef unsigned char u8;
typedef short s16;
typedef int s32;
typedef unsigned int u32;

extern s32 func_L00_0037DDE8(u32 key, u32 owner);

u8 *func_L00_0037DE30(int a0, u8 **a1, int a2, int a3)
{
    u8 *found = 0;
    int best = 0x270F;

    for (; a2 > 0; a2--) {
        s32 v;

        if (a3 <= 0)
            continue;
        if (*(s16 *)(*(u8 **)(*a1 + 0x24) + 0x46) != a3)
            continue;
        v = func_L00_0037DDE8((u32)a0, (u32)*a1);
        if (v >= best)
            continue;
        if (v == -1)
            continue;
        found = *a1;
        best = v;
    }
    return found;
}
