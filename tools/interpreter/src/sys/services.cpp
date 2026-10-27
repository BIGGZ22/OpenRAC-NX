// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * What the runtime does in place of library functions that, on the console,
 * ask the second processor for something: the disc, the clock, the console's
 * settings. Each is the documented behaviour of a public function as a
 * program sees it, written from that behaviour, not from any SDK source.
 *
 * Each service is a lambda that `Machine::syscall` calls when the program enters the replaced
 * function, on the EE's thread. Its arguments are the EE's registers, read with `Machine::arg`,
 * and its result is set with `Machine::result`. A service is run by name: the hooks table of a
 * game says at which address each one goes. It leaves out the effects of a request on the
 * second processor, which is not modelled.
 *
 * Sources: the public behaviour of the disc, remote call, pad, sound and clock library functions,
 * and what the games' own code passes to them and checks in their results.
 */

#include <ctime>
#include <vector>

#include "machine.h"
#include "snd/bank.h"
#include "snd/player.h"

namespace sys {

using namespace ps2;

namespace {

/**
 * Converts a number below 100 to binary coded decimal, one digit to a nibble.
 *
 * @param v The number, 0-99.
 * @return The two digits, tens in the high nibble.
 */
u8 bcd(int v) {
    return static_cast<u8>(((v / 10) << 4) | (v % 10));
}

/**
 * Loads a sound effect bank from the disc, where a program names it by its first sector.
 *
 * @param m The machine.
 * @param handle The number the program will name the bank by.
 * @param sector The bank file's first sector.
 * @return True if a bank was there and is loaded.
 */
bool load_bank_from_disc(Machine& m, u32 handle, u32 sector) {
    std::vector<u8> file(Disc::kSector);

    // The first sector holds the table that says how long the file is.
    if (!m.disc.read(sector, 1, file.data())) {
        return false;
    }

    std::size_t bytes = snd::Bank::file_bytes(file.data());
    u32 sectors = static_cast<u32>((bytes + Disc::kSector - 1) / Disc::kSector);

    // Not a bank, or the disc ends before it does.
    file.resize(std::size_t{sectors} * Disc::kSector);

    if (bytes == 0 || !m.disc.read(sector, sectors, file.data())) {
        return false;
    }

    return m.effects.load_bank(handle, file.data(), bytes);
}

/**
 * Loads a sound effect bank that the program has read into its own memory.
 *
 * @param m The machine.
 * @param handle The number the program will name the bank by.
 * @param address Where the bank file starts in the EE's memory.
 * @return True if a bank was there and is loaded.
 */
bool load_bank_from_memory(Machine& m, u32 handle, u32 address) {
    u8 table[snd::Bank::kTableBytes];

    for (u32 n = 0; n < sizeof(table); n++) {
        table[n] = m.ee.read8(address + n);
    }

    std::size_t bytes = snd::Bank::file_bytes(table);
    const u8* first = m.ee.pointer(address);

    // Not a bank, or one that does not lie whole in main memory.
    if (bytes == 0 || !first
        || m.ee.pointer(address + static_cast<u32>(bytes) - 1) != first + bytes - 1) {
        return false;
    }

    return m.effects.load_bank(handle, first, bytes);
}

/**
 * Starts a sound effect: the library's play call with volume, pan, pitch modifier and pitch bend.
 *
 * @param m The machine.
 * @param data Address of the command's six words: bank, sound, volume, pan, modifier, bend.
 * @return The new sound's handle, or 0 when it did not start.
 */
u32 play_effect(Machine& m, u32 data) {
    snd::SoundStart how;
    u32 bank = m.ee.read32(data);

    // The words after the bank, 4 bytes each, in the order of the call's arguments.
    how.index = m.ee.read32(data + 4);
    how.volume = static_cast<s32>(m.ee.read32(data + 8));
    how.pan = static_cast<s32>(m.ee.read32(data + 12));
    how.pitch_modifier = static_cast<s32>(m.ee.read32(data + 16));
    how.pitch_bend = static_cast<s32>(m.ee.read32(data + 20));

    u32 handle = m.sound_next_handle++;

    return m.effects.play(handle, bank, how) ? handle : 0;
}

/**
 * Changes a playing sound effect: the library's call that sets several of its parameters at once.
 *
 * @param m The machine.
 * @param data Address of the command's six words: handle, which to set, volume, pan, modifier,
 *     bend.
 * @return The sound's handle, or 0 when it is no longer playing.
 */
u32 change_effect(Machine& m, u32 data) {
    u32 handle = m.ee.read32(data);
    u32 which = m.ee.read32(data + 4);
    snd::PlayingSound* playing = m.effects.sound(handle);

    // Over already.
    if (!playing) {
        return 0;
    }

    // Bit 0 of `which` is the volume, bit 1 the pan, bit 2 the modifier, bit 3 the bend (assumed).
    s32 volume = which & 1 ? static_cast<s32>(m.ee.read32(data + 8)) : snd::SoundStart::kKeepVolume;
    s32 pan = which & 2 ? static_cast<s32>(m.ee.read32(data + 12)) : snd::SoundStart::kKeepPan;

    playing->set_volume_pan(volume, pan);

    // The pitch modifier was named.
    if (which & 4) {
        playing->set_pitch_modifier(static_cast<s32>(m.ee.read32(data + 16)));
    }

    // The pitch bend was named.
    if (which & 8) {
        playing->set_pitch_bend(static_cast<s32>(m.ee.read32(data + 20)));
    }

    return handle;
}

}  // namespace

void add_library_services(Machine& machine) {
    // --- the disc drive ---

    // Every request completes at once.
    machine.add_service("sceCdInit", [](Machine& m) { m.result(1); });
    machine.add_service("sceCdDiskReady", [](Machine& m) { m.result(2); });  // "complete"
    machine.add_service("sceCdMmode", [](Machine& m) { m.result(1); });
    machine.add_service("sceCdSync", [](Machine& m) { m.result(0); });  // nothing in progress
    machine.add_service("sceCdGetError", [](Machine& m) { m.result(0); });
    machine.add_service("sceCdBreak", [](Machine& m) { m.result(1); });
    // sceCdRead(first sector, count, buffer, mode)
    machine.add_service("sceCdRead", [](Machine& m) {
        u32 sector = m.arg(0), count = m.arg(1), buffer = m.arg(2);
        u8* to = m.ee.pointer(buffer);

        // The buffer must lie whole in main memory; a sector is 2048 bytes (documented).
        bool fits = to && m.ee.pointer(buffer + count * 2048 - 1) == to + count * 2048 - 1;

        // The buffer does not fit, or the disc cannot give the sectors: the read fails.
        if (!fits || !m.disc.read(sector, count, to)) {
            m.log(0, "sceCdRead: cannot read %u sectors at %u to %08x", count, sector, buffer);
            m.result(0);
            return;
        }

        m.log(2, "sceCdRead: %u sectors at %u to %08x", count, sector, buffer);
        m.result(1);
    });

    // sceCdReadClock(clock): eight bytes, binary coded decimal, Japan's time on
    // the console; the host's local time here.
    machine.add_service("sceCdReadClock", [](Machine& m) {
        std::time_t now = std::time(nullptr);
        std::tm t{};
        localtime_r(&now, &t);

        /*
         * Byte 0 is a status, 0 here; then seconds, minutes, hours, a spare byte, day, month, and
         * the year within the century (assumed).
         */
        const u8 clock[8] =
            {0,
             bcd(t.tm_sec),
             bcd(t.tm_min),
             bcd(t.tm_hour),
             0,
             bcd(t.tm_mday),
             bcd(t.tm_mon + 1),
             bcd(t.tm_year % 100)};

        // Write the eight bytes to the buffer the program gave.
        for (u32 n = 0; n < 8; n++) {
            m.ee.write8(m.arg(0) + n, clock[n]);
        }

        m.result(1);
    });

    // sceCdCallback(function): returns the one it replaces.
    machine.add_service("sceCdCallback", [](Machine& m) {
        m.result(m.cd_callback);
        m.cd_callback = m.arg(0);
        m.cd_callback_gp = static_cast<u32>(m.ee.gpr[28].lo);
    });
    /*
     * A read that announces its end through that callback (sceCdRead used
     * this way, and the sound library's own wrapper around it).
     */
    machine.add_service("sceCdRead.callback", [](Machine& m) {
        u32 sector = m.arg(0), count = m.arg(1), buffer = m.arg(2);
        u8* to = m.ee.pointer(buffer);

        // As for sceCdRead: the whole buffer must lie in main memory.
        bool fits = to && m.ee.pointer(buffer + count * 2048 - 1) == to + count * 2048 - 1;

        // The buffer does not fit, or the disc cannot give the sectors: the read fails.
        if (!fits || !m.disc.read(sector, count, to)) {
            m.log(0, "disc read: cannot read %u sectors at %u to %08x", count, sector, buffer);
            m.result(0);
            return;
        }

        m.log(2, "disc read: %u sectors at %u to %08x", count, sector, buffer);

        // The callback fires later, after the caller has returned.
        m.cd_read_finished();
        m.result(1);
    });

    // --- remote calls ---

    /*
     * sceSifBindRpc(client, server, mode): the server is found at once. A
     * program waits for the client record's server pointer (its tenth word).
     */
    machine.add_service("sceSifBindRpc", [](Machine& m) {
        m.rpc_servers[m.arg(0)] = m.arg(1);

        // The tenth word, at byte 0x24, is the server pointer; non-zero means bound.
        m.ee.write32(m.arg(0) + 0x24, 1);
        m.result(0);
    });
    machine.add_service("sceSifCheckStatRpc", [](Machine& m) { m.result(0); });  // never busy
    /*
     * sceSifCallRpc as the 989snd sound library uses it, answered by a sound server of the
     * runtime's own (`Machine::sound` and `Machine::effects`). The library has two clients. The
     * loader's calls return one word, the handle of what was loaded. The command client's calls
     * return a word of all ones, one result word a command, and all ones again; function 0x4D
     * carries a batch (a count, then for each command its number and size as half-words and its
     * data, padded to a word), any other function is one command with its data.
     */
    machine.add_service("sceSifCallRpc.989snd", [](Machine& m) {
        /*
         * The arguments are client, function, mode, send, send size, receive, receive size, end
         * function and end argument. From the receive buffer on they are in registers 9-11 and
         * the stack (the ninth).
         */
        u32 function = m.arg(1), send = m.arg(3);
        u32 receive = static_cast<u32>(m.ee.gpr[9].lo), size = static_cast<u32>(m.ee.gpr[10].lo);
        u32 end_function = static_cast<u32>(m.ee.gpr[11].lo);
        u32 end_argument = m.ee.read32(static_cast<u32>(m.ee.gpr[29].lo));

        /*
         * Server 0x11 is the games' own "stash": a stretch of the second
         * processor's memory where a program parks data (sent there with
         * SifSetDma) to fetch it back later. Function 2 tells where the stretch
         * is and how long; function 1 copies from an address in it (the word
         * sent) into the receive buffer.
         */
        auto server = m.rpc_servers.find(m.arg(0));
        if (server != m.rpc_servers.end() && server->second == 0x11) {
            /** Where the stash starts in the second processor's memory, and how long it is. */
            constexpr u32 kStashBase = 0x00080000, kStashBytes = 0x00170000;

            // Function 2: tell the program where the stash is.
            if (function == 2 && size >= 8) {
                m.ee.write32(receive, kStashBase);
                m.ee.write32(receive + 4, kStashBytes);
            } else if (function == 1) {
                // Function 1: copy `size` bytes from the address sent into the receive buffer.
                u32 from = m.ee.read32(send);
                for (u32 n = 0; n < size; n++) {
                    m.ee.write8(receive + n, m.iop_memory[(from + n) & (Machine::kIopBytes - 1)]);
                }
            }

            // Tell the program the call is over, if it asked to be told.
            if (end_function) {
                m.ee.call(end_function, end_argument);
            }

            m.result(0);
            return;
        }

        /*
         * What the sound server answers: streams (music, speech) and the sounds of the banks.
         * Called below for each command, on the EE's thread, with its number and data address.
         */
        auto command = [&m](u32 number, u32 data) -> u32 {
            switch (number) {
                case 0x11:  // play a sound from a bank
                    return play_effect(m, data);

                case 0x21:  // change a playing sound
                    return change_effect(m, data);

                case 0x06:  // unload a bank (its handle)
                    m.effects.unload_bank(m.ee.read32(data));
                    return 0;

                case 0x16:  // pause the sounds of some groups (one bit a group)
                    m.effects.set_groups_paused(m.ee.read32(data), true);
                    return 0;

                case 0x17:  // and let them go on
                    m.effects.set_groups_paused(m.ee.read32(data), false);
                    return 0;

                case 0x18:  // stop every sound of the banks
                    m.effects.stop_all();
                    return 0;

                case 0x09: {  // the volume of a group of sounds (group, volume)
                    u32 group = m.ee.read32(data);
                    s32 volume = static_cast<s32>(m.ee.read32(data + 4));

                    // Streams and sound effects share the groups.
                    m.sound.set_group_volume(group, volume);
                    m.effects.set_group_volume(group, volume);
                    return 0;
                }

                case 0x2C: {
                    /*
                     * Play a stream from the disc: (sector, a second sector, volume
                     * above an offset, pan above an offset, group, the stream to
                     * follow, flags). Flag 4: repeat. The answer is the handle, or
                     * that of the stream it follows.
                     */
                    // Command words by byte offset: sector 0, volume 8 (high half), group 16.
                    u32 sector = m.ee.read32(data), after = m.ee.read32(data + 20),
                        flags = m.ee.read32(data + 24);
                    int volume = static_cast<int>(m.ee.read32(data + 8) >> 16);
                    u32 group = m.ee.read32(data + 16);

                    // The stream to follow is still playing: stitch the new file on behind it.
                    if (after && m.sound.playing(after)) {
                        return m.sound.play(after, sector, volume, group, (flags & 4) != 0, after)
                                   ? after
                                   : 0;
                    }

                    u32 handle = m.sound_next_handle++;
                    return m.sound.play(handle, sector, volume, group, (flags & 4) != 0, 0) ? handle
                                                                                            : 0;
                }

                case 0x2D:  // pause a stream
                    m.sound.pause(m.ee.read32(data), true);
                    return 0;

                case 0x2E:  // and go on
                    m.sound.pause(m.ee.read32(data), false);
                    return 0;

                case 0x15:  // stop a sound, or a stream
                    m.sound.stop(m.ee.read32(data));
                    m.effects.stop(m.ee.read32(data));
                    return 0;

                case 0x34:  // stop every stream
                    m.sound.stop_all();
                    return 0;

                case 0x4F:  // is the stream buffered?
                    // Streams are read whole when they start, so they always are.
                    return 1;

                case 0x19: {  // is it still playing? Its handle if so.
                    u32 handle = m.ee.read32(data);
                    bool playing = m.sound.playing(handle) || m.effects.is_playing(handle);

                    return playing ? handle : 0;
                }

                case 0x32:  // time left in the stream
                    return m.sound.remaining(m.ee.read32(data));

                default:
                    // A command this does not know is answered with 0.
                    return 0;
            }
        };

        // A send size of 4 bytes is the loader client: one word of answer, the handle.
        if (size == 4) {
            // Only for the trace: list up to ten words of what the loader was sent.
            if (m.verbose >= 2) {
                std::string words;
                for (u32 w = 0; w < m.arg(4) && w < 40; w += 4) {
                    char text[12];
                    std::snprintf(text, sizeof(text), " %08x", m.ee.read32(send + w));
                    words += text;
                }
                m.log(2, "sound loader call %x (%u bytes):%s", function, m.arg(4), words.c_str());
            }

            // The handle sits in the upper half of the word.
            u32 handle = m.sound_next_handle++ << 16;
            bool loaded = false;

            // Function 3 loads a bank from a sector of the disc (seen in game code).
            if (function == 3) {
                loaded = load_bank_from_disc(m, handle, m.ee.read32(send));
            }

            // Function 0x57 loads a bank the program holds in its memory (seen in game code).
            if (function == 0x57) {
                loaded = load_bank_from_memory(m, handle, m.ee.read32(send));
            }

            m.log(
                2, "sound loader call %x: handle %x%s", function, handle, loaded ? ", a bank" : ""
            );
            m.ee.write32(receive, handle);
        } else if (size >= 8) {
            // The command client: a word of all ones, one result word per command, all ones again.
            m.ee.write32(receive, 0xFFFFFFFFu);
            u32 results = size / 4 - 2;

            if (function == 0x4D) {
                // A batch: the count, then each command as number, size, data padded to a word.
                u32 count = m.ee.read32(send), at = send + 4;
                for (u32 n = 0; n < count && n < results; n++) {
                    u32 number = m.ee.read16(at), bytes = m.ee.read16(at + 2);
                    u32 answer = command(number, at + 4);

                    // Only for the trace: list up to ten words of the command's data.
                    if (m.verbose >= 2) {
                        std::string words;
                        for (u32 w = 0; w < bytes && w < 40; w += 4) {
                            char text[12];
                            std::snprintf(text, sizeof(text), " %08x", m.ee.read32(at + 4 + w));
                            words += text;
                        }
                        m.log(
                            2,
                            "sound command %02x (%u bytes):%s -> %x",
                            number,
                            bytes,
                            words.c_str(),
                            answer
                        );
                    }

                    m.ee.write32(receive + 4 + n * 4, answer);

                    // The next command follows this one's header and data, padded to a word.
                    at += (4 + bytes + 3) & ~3u;
                }
            } else {
                // Any other function is one command, answered in every result word.
                u32 answer = command(function, send);
                m.log(2, "sound command %02x, waited for -> %x", function, answer);
                for (u32 n = 0; n < results; n++) {
                    m.ee.write32(receive + 4 + n * 4, answer);
                }
            }

            m.ee.write32(receive + 4 + results * 4, 0xFFFFFFFFu);
        }

        // Tell the program the call is over, if it asked to be told.
        if (end_function) {
            m.ee.call(end_function, end_argument);
        }

        m.result(0);
    });

    // --- the memory card ---

    add_memory_card_services(machine);

    // --- the pad ---

    machine.add_service("scePad2GetState", [](Machine& m) {
        m.result(1);
    });  // connected and stable
    // scePad2GetButtonProfile(socket, profile): which buttons exist, four bytes
    // of ones for a pad with every button and both sticks.
    machine.add_service("scePad2GetButtonProfile", [](Machine& m) {
        m.ee.write32(m.arg(1), 0xFFFFFFFFu);
        m.result(4);
    });
    // sceVibGetProfile(socket, profile): which motors exist.
    machine.add_service("sceVibGetProfile", [](Machine& m) {
        // Bits 0 and 1 set: both motors exist (assumed).
        m.ee.write32(m.arg(1), 3);
        m.result(1);
    });
    /*
     * scePad2Read(socket, data): two bytes of buttons (a clear bit is a
     * pressed button), the right stick, the left stick, twelve pressures.
     */
    machine.add_service("scePad2Read", [](Machine& m) {
        const Machine::Pad& pad = m.pad;
        u32 to = m.arg(1);

        // The pad word is active low: a clear bit is a pressed button.
        m.ee.write8(to + 0, static_cast<u8>(~pad.buttons));
        m.ee.write8(to + 1, static_cast<u8>(~(pad.buttons >> 8)));
        m.ee.write8(to + 2, pad.right_x);
        m.ee.write8(to + 3, pad.right_y);
        m.ee.write8(to + 4, pad.left_x);
        m.ee.write8(to + 5, pad.left_y);

        /*
         * Pressures in the order right, left, up, down, triangle, circle, cross,
         * square, L1, R1, L2, R2: full when pressed. `bit` gives each one's bit in the button
         * mask (see `Machine::Pad`).
         */
        static constexpr unsigned bit[12] = {5, 7, 4, 6, 12, 13, 14, 15, 10, 11, 8, 9};
        for (u32 n = 0; n < 12; n++) {
            m.ee.write8(to + 6 + n, (pad.buttons >> bit[n]) & 1 ? 0xFF : 0);
        }

        // The result is the number of bytes written: 6 for buttons and sticks, 12 pressures.
        m.result(18);
    });

    // --- the display ---

    // sceGsSyncV(mode): wait for the next vertical blank; which field follows.
    machine.add_service("sceGsSyncV", [](Machine& m) {
        m.skip_to_vblank();

        // The result says which field follows: 0 for the odd one, 1 for the even one (assumed).
        m.result(m.odd_field() ? 0 : 1);
    });

    // --- the console ---

    machine.add_service("sceScfGetLanguage", [](Machine& m) { m.result(1); });  // English
    // A printf-like function of the game: show what it says.
    machine.add_service("printf", [](Machine& m) {
        std::string text = m.format(m.arg(0), 1);

        // The log adds its own newline, so drop the ones the program put at the end.
        while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) {
            text.pop_back();
        }
        m.log(0, "[game] %s", text.c_str());
        m.result(0);
    });
}

}  // namespace sys
