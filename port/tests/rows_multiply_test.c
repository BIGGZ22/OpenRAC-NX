/* SPDX-License-Identifier: GPL-3.0-or-later */
/* func_001FA4F0 (rac1 PAL's 3x3 rows multiply) with its output aliasing either input, as the
 * Pyrocitor's pilot flame calls it (out == b): the same result as into a separate buffer. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../game/rac1-pal/hand/game_rows_multiply.c"
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "%d: %s\n", __LINE__, #c); exit(1); } } while (0)
int main(void) {
    float a[12], b[12], expected[12], alias[12];
    for (int i = 0; i < 12; ++i) {
        a[i] = 0.5f + (float)i * 0.25f - (float)(i % 3);
        b[i] = (float)((i * 7) % 5) - 1.5f;
    }
    func_001FA4F0(expected, a, b);
    for (int j = 0; j < 3; ++j) {
        for (int i = 0; i < 4; ++i) {
            float t = a[i] * b[4 * j];
            t = t + a[4 + i] * b[4 * j + 1];
            t = t + a[8 + i] * b[4 * j + 2];
            CHECK(!memcmp(&t, &expected[4 * j + i], sizeof t));
        }
    }
    memcpy(alias, b, sizeof alias);
    func_001FA4F0(alias, a, alias);
    CHECK(!memcmp(alias, expected, sizeof alias));
    memcpy(alias, a, sizeof alias);
    func_001FA4F0(alias, alias, b);
    CHECK(!memcmp(alias, expected, sizeof alias));
    puts("rows multiply: OK");
    return 0;
}
