/* SPDX-License-Identifier: GPL-3.0-or-later
 * Native DMA hand-off contract, with synthetic registers and no game data. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "openrac/game_host.h"
#include "openrac/game_lib.h"

#define CHECK(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "DMA contract failed at line %d: %s\n", __LINE__, #condition); \
            exit(1); \
        } \
    } while (0)

#define FIRST_CHANNEL 0x10008000u
static uint8_t registers[0x3000], expected[sizeof registers];
static gaddr expected_channel, expected_tag;
static int expected_channel_id, stage;

/* Keep the production API and implementation, mapping its register addresses
 * to a small fixture instead of reserving the full guest address space. */
static void *test_guest(gaddr address) {
    CHECK(address >= FIRST_CHANNEL && address - FIRST_CHANNEL < sizeof registers);
    return registers + (address - FIRST_CHANNEL);
}
#undef G
#define G(address) test_guest((gaddr)(address))
#include "../game/common/lib/graph.c"

static uint32_t word(const uint8_t *bank, gaddr address) {
    uint32_t value;
    memcpy(&value, bank + address - FIRST_CHANNEL, sizeof value);
    return value;
}

static void set_word(uint8_t *bank, gaddr address, uint32_t value) {
    memcpy(bank + address - FIRST_CHANNEL, &value, sizeof value);
}

void openrac_game_dma_send(gaddr channel, gaddr tag) {
    CHECK(stage == 0);
    CHECK(channel == expected_channel && tag == expected_tag);
    CHECK((word(registers, channel) & 0x100u) != 0);
    CHECK(memcmp(registers, expected, sizeof registers) == 0);
    /* A renderer-side change must survive the completion bookkeeping. */
    set_word(registers, channel, word(registers, channel) ^ 0x40000000u);
    set_word(expected, channel, word(expected, channel) ^ 0x40000000u);
    uint32_t completed = word(expected, channel) & ~0x100u;
    if (channel == 0x10009000u) {
        completed = (completed & ~0x03FF0000u) | 0x02010000u;
    }
    set_word(expected, channel, completed);
    stage = 1;
}

void openrac_game_run_dmac_handlers(int channel) {
    CHECK(stage == 1 && channel == expected_channel_id);
    CHECK((word(registers, expected_channel) & 0x100u) == 0);
    CHECK(memcmp(registers, expected, sizeof registers) == 0);
    stage = 2;
}

/* Other graphics entry points share the source file but are not exercised. */
int openrac_game_vsync(void) { CHECK(0); return 0; }
void openrac_game_run_vsync_handlers(void) { CHECK(0); }
void openrac_game_set_video_mode(int a, int b, int c) {
    (void)a; (void)b; (void)c; CHECK(0);
}
void openrac_game_set_display(const openrac_game_display *display) {
    (void)display; CHECK(0);
}
void openrac_game_load_image(const openrac_game_image *image) {
    (void)image; CHECK(0);
}
void openrac_game_store_image(const openrac_game_image *image) {
    (void)image; CHECK(0);
}

int main(void) {
    static const gaddr channels[] = {0x10008000u, 0x10009000u, 0x1000A000u};
    static const gaddr tags[] = {0x00123450u, 0x70001230u, 0xA0123450u};
    for (unsigned i = 0; i < sizeof channels / sizeof channels[0]; ++i) {
        for (unsigned j = 0; j < sizeof tags / sizeof tags[0]; ++j) {
            for (int sentinel = 0; sentinel < 2; ++sentinel) {
                const gaddr channel = channels[i];
                const uint32_t initial_chcr = 0xA5FE0088u;
                const uint32_t initial_tadr = sentinel ? UINT32_MAX : 0x00555550u;
                memset(registers, 0xA5, sizeof registers);
                set_word(registers, channel, initial_chcr);
                set_word(registers, channel + 0x20u, 73);
                set_word(registers, channel + 0x30u, initial_tadr);
                memcpy(expected, registers, sizeof expected);
                expected_channel = channel;
                expected_channel_id = (int)i;
                expected_tag = j == 1 ? 0x80001230u : tags[j];
                set_word(expected, channel, (initial_chcr & ~0xCu) | 0x105u);
                set_word(expected, channel + 0x20u, 0);
                set_word(expected, channel + 0x30u, sentinel ? UINT32_MAX : expected_tag);
                stage = 0;
                openrac_lib_sceDmaSend(channel, tags[j]);
                CHECK(stage == 2);
                CHECK(memcmp(registers, expected, sizeof registers) == 0);
            }
        }
    }
    puts("Native DMA submission and idle completion contracts passed");
    return 0;
}
