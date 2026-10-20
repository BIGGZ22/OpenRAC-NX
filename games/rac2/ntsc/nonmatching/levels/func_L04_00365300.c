/* func_L04_00365300, 184 bytes, 2 places, src/levels/shared/L04_002CC748.c (-O2 -G0).
   ABANDON - the two `nop` retail keeps before `div.s` (the known division-floor class).
   Read in full: `span = func_00282E80(object, a1)`; when it is zero the second span
   `func_00282E80(object, a2)` is returned; otherwise the signed area of the object/a1/a2
   triangle is squared and normalised by `span * span` and handed to `func_00282C10(object, r)`.
   The candidate is 176 bytes with every instruction exact but two words: retail runs
   `mul.s $f12,$f12,$f12 ; nop ; nop ; div.s $f12,$f12,$f5`, this cc1 emits the division
   directly after the multiply. The assembler half is measured and present (inserting a code
   label before the division in the generated `.s` reproduces the 184 bytes exactly), but no
   source shape puts a label there: the division sits 38 slots after the last label in all four
   shapes tried (inline expression, squared-area statement, both statements with a local
   quotient, explicit else arm), so the floor never triggers from C. The prologue also rotates
   $s1/$s2 (retail keeps a1 in $s1 and a2 in $s2, this cc1 the other way). Same class as
   func_L00_003EEE20, func_L00_003E4B80, func_L00_00329FF0, func_L02_002DCEA0 and
   func_L04_0039FEA8. Below: the best candidate (176 bytes). */

typedef float f32;

extern float func_00282E80(char *, char *);
extern float func_00282C10(char *, float);

float func_L04_00365300(char *object, char *a1, char *a2) {
    float span = func_00282E80(object, a1);
    float area;

    if (span == 0.0f)
        return func_00282E80(object, a2);
    area = (*(float *)(a2 + 4) - *(float *)(object + 4)) * (*(float *)(a1 + 0) - *(float *)(object + 0)) -
           (*(float *)(a2 + 0) - *(float *)(object + 0)) * (*(float *)(a1 + 4) - *(float *)(object + 4));
    return func_00282C10(object, area * area / (span * span));
}
