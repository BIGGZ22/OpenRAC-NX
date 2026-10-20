/* SPDX-License-Identifier: GPL-3.0-or-later
 * Native boundary for Deadlocked's 989snd command submission.
 * The IOP is not present in the native port, so commands supported by the
 * native sound engine run synchronously and unsupported ones fail promptly.
 */
#include <stdint.h>
#include <stdio.h>

#include "openrac/guest.h"
#include "openrac/game_lib.h"
#include "openrac/sound_engine.h"

/* The retail snd_BankLoadFromEE copies the bank through an IOP RPC (function
 * 0x57) and waits for the IOP to return a handle. There is no IOP in the native
 * port, so load the same EE bank through OpenRAC's sound engine. The bank's
 * own header determines its size, as in the shared BankLoadFromEE_CB path. */
int func_00158400(int bank_arg) {
    const gaddr bank = (gaddr)bank_arg;
    if (bank == 0) {
        return 0;
    }
    return (int)openrac_sound_load_bank(
        G(bank), 0x02000000u - (bank & 0x01FFFFFFu)
    );
}

/* The synchronous command returns the reply's value (receive buffer +4).
 * Its retail implementation waits for the IOP even after a failed request.
 * Keep the caller's EE-side initialization and answer at the native API. */
int func_00158A30(int command_arg, int bytes, int data_arg) {
    const unsigned int command = (unsigned int)command_arg;
    const gaddr data = (gaddr)data_arg;
    if (bytes < 0 || bytes > 0xFFC || (bytes != 0 && data == 0)) {
        return 0;
    }
    if (command == 0) { /* sound-system startup; host audio owns the device */
        return openrac_lib_snd_StartSoundSystem();
    }
    if (command == OPENRAC_SND_PLAY_VOL_PAN_PMPB || command == OPENRAC_SND_SET_SOUND_PARAMS) {
        return (int)openrac_sound_command(command, G(data), (uint32_t)bytes);
    }
    fprintf(stderr, "[warning] RAC4 synchronous sound command %#x is unsupported; rejected without IOP\n", command);
    return 0;
}

int func_00158C60(int command_arg, int bytes, int data_arg, int callback_arg) {
    const unsigned int command = (unsigned int)command_arg;
    const gaddr data = (gaddr)data_arg;
    const gaddr callback = (gaddr)callback_arg;
    (void)callback;
    if (bytes < 0 || bytes > 0xFFC || (bytes != 0 && data == 0)) {
        return 0;
    }
    if (command == OPENRAC_SND_PLAY_VOL_PAN_PMPB || command == OPENRAC_SND_SET_SOUND_PARAMS) {
        (void)openrac_sound_command(command, G(data), (uint32_t)bytes);
        return 1;
    }
    static unsigned int last_reported = 0xFFFFFFFFu;
    if (last_reported != command) {
        fprintf(stderr, "[warning] RAC4 sound command %#x is unsupported; rejected without IOP\n", command);
        last_reported = command;
    }
    return 0;
}
