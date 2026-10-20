/* Near miss: the level's copy of the boot's func_0031EB00 (the render setup over the
 * level's own float table).
 *
 * Size is exact (360 bytes) and every place fails on one instruction: the compiler
 * materialises the 1.0f constant after the call to the level's func_L00_003CBE28, while
 * retail materialises it before the three position stores and leaves the store in the
 * call's delay slot. The register allocation follows: retail holds position[8] in $f3
 * and the constant in $f2, this candidate holds position[8] in $f2 and reuses $f0 for
 * the constant. Retail also hoists the call's first argument (`daddu $a0,$sp,$zero`)
 * above the two remaining position loads.
 *
 * What was measured:
 * - the level's float table is at 0x001B8C80 in every level (the boot's is 0x001B5200);
 *   `lui $s2,0x1C` + `addiu $s2,$s2,-0x7380` in retail, and the linker fills the %hi/%lo
 *   from the name, so D_L00_001B8C80 reproduces it.
 * - the level's copies of the two helpers are func_L00_003CBC80 and func_L00_003CBE28;
 *   the boot's names (func_0031E750, func_0031E8F8) are the wrong ones here.
 * - the call to the level's func_L00_003CBE28 sits between the third and the fourth
 *   store in retail, so the source order is C0, C4, C8, call, CC.
 * - with the helper's prototype missing, cc1 calls fptodp (the implicit int return
 *   promotes the float argument); the prototype must come before the body.
 *
 * tools/check.py func_L00_003CC030: 0 of 27 places match ("instruction at +0x100
 * differs" in every level).
 */
extern int func_0027C878(int);

extern float func_00283248(float, float);

extern float func_00282E80(void *, void *);

extern void func_00283410(void *, void *);

extern void func_00280B90(void *, void *, int);

extern float D_L00_001B8C80[];

extern void func_L00_003CBC80(char *);

extern void func_L00_003CBE28(char *, float);

void func_L00_003CC030(char *position, int color, float scale, char *resource, float alpha, int mode, int flag)
{
    unsigned char work[0xE0];
    unsigned int rgba;
    unsigned long long mode_bits;
    float first_angle;
    float second_angle;
    float distance;

    func_L00_003CBC80((char *)work);
    *(long long *)(work + 0x78) = func_0027C878(mode);

    mode_bits = ((unsigned long long)flag << 2) | 0x0000008000000040ULL;
    *(long long *)(work + 0x88) = mode_bits;
    *(long long *)(work + 0x80) = 0x0000FF9000000260ULL;

    rgba = (unsigned int)color + ((unsigned int)resource << 24);
    *(int *)(work + 0x4C) = rgba;
    *(int *)(work + 0x48) = rgba;
    *(int *)(work + 0x44) = rgba;
    *(int *)(work + 0x40) = rgba;
    *(long long *)(work + 0x70) = 0;

    *(int *)(work + 0xDC) = 0;
    first_angle = func_00283248(D_L00_001B8C80[0x140 / 4] - *(float *)(position + 0),
                                D_L00_001B8C80[0x144 / 4] - *(float *)(position + 4));
    *(float *)(work + 0xD8) = first_angle;

    distance = func_00282E80(position, &D_L00_001B8C80[0x140 / 4]);
    second_angle = func_00283248(distance, D_L00_001B8C80[0x148 / 4] - *(float *)(position + 8));
    *(float *)(work + 0xD0) = alpha;
    *(float *)(work + 0xD4) = -second_angle;

    func_00283410(work + 0x90, work + 0xD0);
    *(float *)(work + 0xC0) = *(float *)(position + 0);
    *(float *)(work + 0xC4) = *(float *)(position + 4);
    *(float *)(work + 0xC8) = *(float *)(position + 8);
    func_L00_003CBE28((char *)work, scale);
    *(float *)(work + 0xCC) = 1.0f;
    func_00280B90(work, work + 0x90, 0);
}
