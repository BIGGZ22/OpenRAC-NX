/* SPDX-License-Identifier: GPL-3.0-or-later
 * Deadlocked's libpad API, backed by the host's controller ports.
 * The retail reader at 0x0013BA20 copies the padman's packet; the game's
 * reader at 0x00490400 consumes the status/id prefix, active-low buttons,
 * sticks and pressures. libpad2 uses the same payload without that prefix.
 * Configuration completes synchronously. Multitaps and motors are absent.
 */
#include <stdint.h>
#include <string.h>

#include "openrac/game_host.h"
#include "openrac/game_lib.h"
#include "openrac/guest.h"

static struct {
    unsigned char open, analog, pressure;
} pads[2];

static int opened(int port, int slot) {
    return port >= 0 && port < 2 && slot == 0 && pads[port].open;
}

static int connected(int port, int slot) {
    uint16_t buttons;
    uint8_t analog[4];
    return opened(port, slot) && openrac_game_pad(port, &buttons, analog);
}

int func_0013B1D0(int mode) { /* scePadInit */
    (void)mode;
    memset(pads, 0, sizeof pads);
    return 1;
}

int func_0013B460(void) { /* scePadEnd */
    memset(pads, 0, sizeof pads);
    return 1;
}

int func_0013B628(int port, int slot, int work) { /* scePadPortOpen */
    (void)work; /* DMA work storage belongs to the console's driver. */
    if (port < 0 || port >= 2 || slot != 0) {
        return 0;
    }
    pads[port].open = 1;
    pads[port].analog = 0;
    pads[port].pressure = 0;
    return 1;
}

int func_0013B810(int port, int slot) { /* scePadPortClose */
    if (!opened(port, slot)) {
        return 0;
    }
    memset(&pads[port], 0, sizeof pads[port]);
    return 1;
}

int func_0013BA20(int port, int slot, int data) { /* scePadRead */
    if (data == 0 || !connected(port, slot)) {
        return 0;
    }
    unsigned char *out = G((gaddr)data);
    memset(out, 0, 32);
    out[1] = pads[port].analog ? (pads[port].pressure ? 0x79 : 0x73) : 0x41;
    openrac_lib_scePad2Read(port, (gaddr)data + 2u);
    if (!pads[port].analog) {
        memset(out + 4, 0, 16);
    } else if (!pads[port].pressure) {
        memset(out + 8, 0, 12);
    }
    return 32;
}

int func_0013BA98(int port, int slot) { /* scePadGetState */
    if (!opened(port, slot)) {
        return 99; /* same unopened-port result as the retail library */
    }
    return connected(port, slot) ? 6 : 0; /* stable / disconnected */
}

int func_0013BBD0(int port, int slot) { /* scePadGetReqState */
    return connected(port, slot) ? 0 : 1; /* completed / failed */
}

int func_0013BC30(int port, int slot, int actuator, int term) { /* scePadInfoAct */
    (void)port;
    (void)slot;
    (void)actuator;
    (void)term;
    return 0; /* no motors advertised */
}

int func_0013BD50(int port, int slot, int info, int index) { /* scePadInfoMode */
    if (!connected(port, slot)) {
        return 0;
    }
    switch (info) {
    case 1: /* current id */
    case 2: /* current extended id */
        return pads[port].analog ? 7 : 4;
    case 3: /* current mode-table index */
        return pads[port].analog ? 1 : 0;
    case 4: /* mode count, or entry */
        return index == -1 ? 2 : index == 0 ? 4 : index == 1 ? 7 : 0;
    default:
        return 0;
    }
}

int func_0013BE88(int port, int slot, int mode, int lock) { /* scePadSetMainMode */
    (void)lock;
    if (!connected(port, slot) || (mode != 0 && mode != 1)) {
        return 0;
    }
    pads[port].analog = (unsigned char)mode;
    if (!mode) {
        pads[port].pressure = 0;
    }
    return 1;
}

int func_0013BF40(int port, int slot, int values) { /* scePadSetActDirect */
    (void)port;
    (void)slot;
    (void)values;
    return 0;
}

int func_0013C008(int port, int slot, int alignment) { /* scePadSetActAlign */
    (void)port;
    (void)slot;
    (void)alignment;
    return 0;
}

int func_0013C0E0(int port, int slot, int mask) { /* scePadSetButtonInfo */
    if (!connected(port, slot) || !pads[port].analog) {
        return 0;
    }
    pads[port].pressure = mask != 0;
    return 1;
}

int func_0013C190(int port, int slot) { /* scePadInfoPressMode */
    return connected(port, slot) && pads[port].analog;
}

int func_0013C210(int port, int slot) { /* scePadEnterPressMode */
    return func_0013C0E0(port, slot, 0xFFF);
}

int func_0013C280(void) { /* scePadGetModVersion */
    return 0x0400;
}
