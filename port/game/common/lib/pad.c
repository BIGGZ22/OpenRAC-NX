/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * The pad libraries (libdbc, libpad2, libvib). The pads come from the
 * platform layer (port/platform, through openrac_game_pad).
 *
 * Not done yet: the layout of what scePad2Read fills in. The game's own
 * reader (func_00217F68, matched C) and its post-processing (func_002181F0)
 * show which bytes it takes; the layout below is the one OpenRAC's earlier
 * runtime answered with, which the game played with. */
#include "openrac/game_host.h"
#include "openrac/game_lib.h"
#include "openrac/guest.h"

#include <stdint.h>

void (*openrac_game_pad_report_hook)(int port, uint8_t* report, int size);

int openrac_lib_sceDbcInit(void) {
    return 1;
} /* sceDbcInit */

int openrac_lib_scePad2Init(int mode) {
    (void)mode;
    return 1;
} /* scePad2Init */

/* The socket is the port the parameters name (their second word; rac1 writes {2, port}), so a
 * game that sets its pads up again (rac1 does after Quit Game) reads the same controller. Counting
 * sockets up gave the second setup socket 1, the empty second port, and every button stopped. */
int openrac_lib_scePad2CreateSocket(gaddr params, gaddr work) { /* scePad2CreateSocket */
    (void)work;
    const int port = params != 0 ? GREF(int, params + 4) : 0;
    return port >= 0 && port < 2 ? port : 0;
}

/* scePad2Read(socket, data): two bytes of buttons (a clear bit is a pressed
 * button), the right stick, the left stick, then twelve pressures in the
 * order right, left, up, down, triangle, circle, cross, square, L1, R1, L2,
 * R2 (full when pressed). The result is the number of bytes written. The
 * socket number is the port. */
int openrac_lib_scePad2Read(int socket, gaddr data) { /* scePad2Read */
    static const unsigned bit[12] = {5, 7, 4, 6, 12, 13, 14, 15, 10, 11, 8, 9};
    uint16_t buttons = 0xFFFF;
    uint8_t analog[4] = {0x80, 0x80, 0x80, 0x80};
    uint8_t *out = (uint8_t *)G(data);
    unsigned n;

    openrac_game_pad(socket, &buttons, analog);
    out[0] = (uint8_t)buttons;
    out[1] = (uint8_t)(buttons >> 8);
    out[2] = analog[0];
    out[3] = analog[1];
    out[4] = analog[2];
    out[5] = analog[3];
    for (n = 0; n < 12; n++) {
        out[6 + n] = (buttons >> bit[n]) & 1 ? 0 : 0xFF;
    }
    if (openrac_game_pad_report_hook != 0) {
        openrac_game_pad_report_hook(socket, out, 18);
    }
    return 18;
}

/* Which buttons exist: every button and both sticks. */
int openrac_lib_scePad2GetButtonProfile(int socket, gaddr profile) { /* scePad2GetButtonProfile */
    uint32_t all = 0xFFFFFFFFu;
    (void)socket;
    __builtin_memcpy(G(profile), &all, 4);
    return 4;
}

int openrac_lib_scePad2GetState(int socket) { /* scePad2GetState */
    (void)socket;
    return 1; /* connected and stable */
}

int openrac_lib_sceVibGetProfile(int socket, gaddr profile) { /* sceVibGetProfile */
    (void)socket;
    (void)profile;
    return 0;
}
