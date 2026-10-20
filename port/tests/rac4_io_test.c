/* SPDX-License-Identifier: GPL-3.0-or-later
 * Native API contracts only; no disc, SDK files, window or IOP. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "openrac/game_host.h"
#include "openrac/game_lib.h"
#include "openrac/guest.h"
#include "openrac/sound_engine.h"

int func_0013B1D0(int);
int func_0013B460(void);
int func_0013B628(int, int, int);
int func_0013B810(int, int);
int func_0013BA20(int, int, int);
int func_0013BA98(int, int);
int func_0013BBD0(int, int);
int func_0013BD50(int, int, int, int);
int func_0013BE88(int, int, int, int);
int func_0013C210(int, int);
int func_00158A30(int, int, int);
int func_00158C60(int, int, int, int);

static uint8_t memory[128];
uint8_t *openrac_guest_base = memory;
static uint16_t sample_buttons = 0xFFFF;
static const uint8_t sample_sticks[4] = {17, 34, 51, 68};
static int sample_connected = 1;
static int sound_starts, sound_commands;

static void check(int ok, int line) {
    if (!ok) {
        fprintf(stderr, "RAC4 native I/O contract failed at line %d\n", line);
        exit(1);
    }
}
#define CHECK(value) check((value), __LINE__)

int openrac_game_pad(int port, uint16_t *buttons, uint8_t analog[4]) {
    *buttons = sample_buttons;
    memcpy(analog, sample_sticks, sizeof sample_sticks);
    return port == 0 && sample_connected;
}

int openrac_lib_snd_StartSoundSystem(void) {
    sound_starts++;
    return 0;
}

uint32_t openrac_sound_command(uint32_t command, const void *data, uint32_t size) {
    CHECK(command == OPENRAC_SND_PLAY_VOL_PAN_PMPB || command == OPENRAC_SND_SET_SOUND_PARAMS);
    CHECK(data == memory + 16 && size == 8);
    sound_commands++;
    return 0x12345678;
}

uint32_t openrac_sound_load_bank(const void *data, size_t size) {
    (void)data;
    (void)size;
    return 0;
}

int main(void) {
    uint8_t *packet = memory + 16;
    CHECK(func_0013B1D0(0) == 1);
    CHECK(func_0013BA98(0, 0) == 99);
    CHECK(func_0013B628(-1, 0, 0) == 0);
    CHECK(func_0013B628(0, 1, 0) == 0);
    CHECK(func_0013B628(0, 0, 0) == 1);
    CHECK(func_0013BA98(0, 0) == 6);
    CHECK(func_0013BA98(0, 1) == 99);
    CHECK(func_0013BD50(0, 0, 4, -1) == 2);
    CHECK(func_0013BD50(0, 0, 4, 0) == 4);
    CHECK(func_0013BD50(0, 0, 4, 1) == 7);
    memset(memory, 0xA5, sizeof memory);
    CHECK(func_0013BA20(0, 0, 16) == 32);
    CHECK(packet[0] == 0 && packet[1] == 0x41);
    CHECK(packet[2] == 0xFF && packet[3] == 0xFF);
    CHECK(memory[15] == 0xA5 && memory[48] == 0xA5);
    CHECK(func_0013BE88(0, 0, 1, 3) == 1);
    CHECK(func_0013BBD0(0, 0) == 0);
    CHECK(func_0013BA20(0, 0, 16) == 32 && packet[1] == 0x73);
    CHECK(memcmp(packet + 4, sample_sticks, 4) == 0);
    for (int i = 8; i < 32; i++) {
        CHECK(packet[i] == 0);
    }
    CHECK(func_0013C210(0, 0) == 1);
    /* Protocol pressure order: right, left, up, down, triangle, circle,
     * cross, square, L1, R1, L2, R2. Each pad bit is active low. */
    static const int pressure_bits[12] = {5, 7, 4, 6, 12, 13, 14, 15, 10, 11, 8, 9};
    for (int bit = 0; bit < 16; bit++) {
        sample_buttons = (uint16_t)(0xFFFFu ^ (1u << bit));
        CHECK(func_0013BA20(0, 0, 16) == 32 && packet[1] == 0x79);
        CHECK(packet[2] == (uint8_t)sample_buttons);
        CHECK(packet[3] == (uint8_t)(sample_buttons >> 8));
        for (int i = 0; i < 12; i++) {
            CHECK(packet[8 + i] == (pressure_bits[i] == bit ? 0xFF : 0));
        }
        CHECK(memory[15] == 0xA5 && memory[48] == 0xA5);
    }
    sample_connected = 0;
    memset(packet, 0xA5, 32);
    CHECK(func_0013BA98(0, 0) == 0 && func_0013BBD0(0, 0) == 1);
    CHECK(func_0013BA20(0, 0, 16) == 0 && packet[0] == 0xA5);
    CHECK(func_0013B810(0, 0) == 1 && func_0013BA98(0, 0) == 99);
    CHECK(func_0013B460() == 1);

    CHECK(func_00158A30(0, 8, 16) == 0 && sound_starts == 1);
    CHECK(func_00158A30(OPENRAC_SND_PLAY_VOL_PAN_PMPB, 8, 16) == 0x12345678);
    CHECK(func_00158C60(OPENRAC_SND_SET_SOUND_PARAMS, 8, 16, 0) == 1);
    CHECK(sound_commands == 2);
    CHECK(func_00158A30(0x7F, 0, 0) == 0);
    CHECK(func_00158A30(0, -1, 16) == 0 && sound_starts == 1);
    CHECK(func_00158A30(OPENRAC_SND_PLAY_VOL_PAN_PMPB, 8, 0) == 0 && sound_commands == 2);
    puts("RAC4 native input packets and synchronous sound contracts passed");
    return 0;
}
