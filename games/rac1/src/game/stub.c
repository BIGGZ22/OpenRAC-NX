#include "common.h"
#include "structs.h"

/*
 * stub.cpp in the original source; text 0x1E9730-0x1E9808.
 * Name and boundary from the NTSC split of this game, mapped to PAL by matching function
 * sizes -- see docs/DECOMP_PROGRESS.md. Compiled as C for now.
 */

void STUB_printf(int arg0, ...) {
}

void Profile_Mark(void) {
}

void noop_callback_a(void) {
}

void noop_callback_b(void) {
}

void noop_callback_c(void) {
}

void noop_callback_d(void) {
}

void noop_callback_e(void) {
}

void noop_callback_f(void) {
}

void noop_callback_g(void) {
}

void noop_callback_h(void) {
}

void noop_callback_i(void) {
}

void noop_callback_j(void) {
}

void noop_callback_k(void) {
}

/* Takes an argument (callers pass one, e.g. func_001EC210) and ignores
   it -- the unused parameter costs no codegen, still 0/8. */
int noop_callback_l(void *arg0) {
    return 0;
}

void noop_callback_m(void) {
}

void noop_callback_n(void) {
}

void noop_callback_o(void) {
}

int noop_callback_p(void) {
    return 0;
}

void noop_callback_q(void) {
}

void noop_callback_r(void) {
}

void noop_callback_s(void) {
}
