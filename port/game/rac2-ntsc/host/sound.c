/* SPDX-License-Identifier: GPL-3.0-or-later
 * Going Commando's 989snd command boundary, without the IOP.
 *
 * The game's sound client sends each request to the IOP's sound driver by
 * RPC: snd_SendCommandSync (0x00132C48: command, size, data) copies the
 * data to its send buffer and waits for the reply, and snd_QueueCommandToRing
 * (0x00132DF0: command, size, data and an optional callback) queues it for
 * the next flush. There is no IOP in the port: commands the native sound
 * engine knows run at once, as Deadlocked's do (rac4-ntsc/host/sound.c), and
 * the others are answered 0 at once instead of waiting for a reply that never
 * comes.
 */
#include <stdint.h>
#include <stdio.h>

#include "openrac/guest.h"
#include "openrac/game_lib.h"
#include "openrac/sound_engine.h"

int func_00132C48(int command_arg, int bytes, int data_arg) {
    const unsigned int command = (unsigned int)command_arg;
    const gaddr data = (gaddr)data_arg;
    if (bytes < 0 || bytes > 0xFFC || (bytes != 0 && data == 0)) {
        return 0;
    }
    if (command == 0) { /* the sound system's start; the host's audio owns the device */
        return openrac_lib_snd_StartSoundSystem();
    }
    if (command == OPENRAC_SND_PLAY_VOL_PAN_PMPB || command == OPENRAC_SND_SET_SOUND_PARAMS) {
        return (int)openrac_sound_command(command, G(data), (uint32_t)bytes);
    }
    static unsigned int last_reported = 0xFFFFFFFFu;
    if (last_reported != command) {
        fprintf(stderr, "[warning] Going Commando synchronous sound command %#x is not supported; answered 0\n",
                command);
        last_reported = command;
    }
    return 0;
}

int func_00132DF0(int command_arg, int bytes, int data_arg, long long callback, int callback_data) {
    const unsigned int command = (unsigned int)command_arg;
    const gaddr data = (gaddr)data_arg;
    (void)callback;
    (void)callback_data;
    if (bytes < 0 || bytes > 0xFFC || (bytes != 0 && data == 0)) {
        return 0;
    }
    if (command == OPENRAC_SND_PLAY_VOL_PAN_PMPB || command == OPENRAC_SND_SET_SOUND_PARAMS) {
        (void)openrac_sound_command(command, G(data), (uint32_t)bytes);
        return 1;
    }
    static unsigned int last_reported = 0xFFFFFFFFu;
    if (last_reported != command) {
        fprintf(stderr, "[warning] Going Commando sound command %#x is not supported; dropped\n", command);
        last_reported = command;
    }
    return 0;
}
