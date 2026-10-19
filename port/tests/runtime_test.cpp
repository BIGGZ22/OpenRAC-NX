// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Game memory, the guest contract and the executable loader.

#include <cstring>
#include <vector>

#include "check.h"
#include "openrac/elf.h"
#include "openrac/guest.h"
#include "openrac/memory.h"

extern "C" {
gaddr test_frame_address(gaddr* sp_inside);
gaddr test_string(void);
void test_register(void);
int test_call(gaddr fn, int a, int b);
}

namespace {

using openrac::runtime::ElfImage;
using openrac::runtime::Memory;

void memory_map() {
    Memory& m = Memory::get();
    CHECK(openrac_guest_base == m.base());
    CHECK(m.mapped(0, GUEST_RAM_SIZE));
    CHECK(!m.mapped(GUEST_RAM_SIZE, 4));
    CHECK(m.mapped(GUEST_SCRATCHPAD, GUEST_SCRATCHPAD_SIZE));
    CHECK(!m.mapped(GUEST_SCRATCHPAD + GUEST_SCRATCHPAD_SIZE, 1));
    CHECK(m.mapped(0x1000A000, 0x100));  // a DMA channel's registers
    CHECK(m.mapped(0x12000000, 0x100));  // GS privileged registers

    GREF(std::uint32_t, 0x00137C80) = 0x12345678u;
    CHECK(*reinterpret_cast<std::uint32_t*>(m.base() + 0x00137C80) == 0x12345678u);
    if (m.has_aliases()) {
        // The same RAM through its uncached addresses.
        CHECK(GREF(std::uint32_t, 0x20137C80) == 0x12345678u);
        GREF(std::uint32_t, 0x30137C80) = 0xCAFEF00Du;
        GBARRIER();
        CHECK(GREF(std::uint32_t, 0x00137C80) == 0xCAFEF00Du);
    }
    gaddr back = 0;
    CHECK(m.contains(G(0x70000010), &back) && back == 0x70000010);
}

void frames_and_strings() {
    const gaddr before = openrac_guest_sp;
    gaddr inside = 0;
    const gaddr frame = test_frame_address(&inside);
    CHECK(frame == inside);
    CHECK(frame % 16 == 0);
    CHECK(frame < before && frame >= GUEST_STACK_BOTTOM);
    CHECK(GREF(int, frame + 4) == 7);
    CHECK(openrac_guest_sp == before);  // given back on return

    const gaddr s = test_string();
    CHECK(s >= GUEST_STRINGS && s < GUEST_STRINGS + GUEST_STRINGS_SIZE);
    CHECK(std::strcmp(static_cast<const char*>(G(s)), "ratchet") == 0);
    CHECK(test_string() == s);
    CHECK(openrac_guest_string("ratchet", 8) == s);  // one copy per text
}

int level_add(int a, int b) {
    return a + b + 100;
}

void functions() {
    test_register();
    CHECK(test_call(0x00100000u, 2, 3) == 5);
    CHECK(std::strcmp(openrac_guest_function_name(0x00100000u), "add") == 0);

    // A level's function at an address the executable also uses wins while
    // that level is loaded.
    static const openrac_fn_entry level[] = {
        {0x00100000u, reinterpret_cast<openrac_host_fn>(level_add), "level_add"}
    };
    openrac_guest_register(5, level, 1);
    openrac_guest_set_overlay(5);
    CHECK(test_call(0x00100000u, 2, 3) == 105);
    openrac_guest_set_overlay(OPENRAC_OVERLAY_EXE);
    CHECK(test_call(0x00100000u, 2, 3) == 5);
    CHECK(openrac_guest_function_name(0x00200000u) == nullptr);
}

// A copy of a function that calls another through a pointer the compiler turned into a jal: the
// address it takes is the one the copy at self calls at the same instruction.
void code_in_copy_jal() {
    const auto jal = [](gaddr target) { return 0x0C000000u | ((target & 0x0FFFFFFFu) >> 2); };
    const std::uint32_t canon[] = {0x27BDFFF0u, jal(0x00310000u), 0, 0x03E00008u};
    const std::uint32_t self[] = {0x27BDFFF0u, jal(0x00330000u), 0, 0x03E00008u};
    std::memcpy(G(0x00300000u), canon, sizeof canon);
    std::memcpy(G(0x00320000u), self, sizeof self);
    openrac_guest_set_overlay(5);
    CHECK(openrac_code_in_copy(5, 0x00300000u, sizeof canon, 0x00320000u, 0x00310000u) == 0x00330000u);
    // Formed with lui and addiu, the address is read from the copy's own pair, as before.
    const std::uint32_t formed[] = {0x3C040031u, 0x24840010u, jal(0x00310010u), 0};
    const std::uint32_t own[] = {0x3C040034u, 0x24840020u, jal(0x00340020u), 0};
    std::memcpy(G(0x00300100u), formed, sizeof formed);
    std::memcpy(G(0x00320100u), own, sizeof own);
    CHECK(openrac_code_in_copy(5, 0x00300100u, sizeof formed, 0x00320100u, 0x00310010u) == 0x00340020u);
    openrac_guest_set_overlay(OPENRAC_OVERLAY_EXE);
}

int folded_measure(int a) {
    return a;
}

int font_printer(int a) {
    return a;
}

// An executable global relocated into a level whose program has three copies of the function that
// names it (rac1: the measurers of the regular, small and large fonts, one name in the catalogue),
// each on a table of its own: the copies disagree on it, so none of them decides where it went;
// the function with a single copy (the printer of the regular font) does, whatever order the
// copies were registered or are kept in.
void relocation_with_folded_copies() {
    const auto words = [](gaddr at, std::initializer_list<std::uint32_t> w) {
        std::memcpy(G(at), w.begin(), w.size() * 4);
    };
    // Executable: the measurer at 0x104000 and the printer at 0x104100, both forming 0x150000.
    std::vector<std::uint8_t> exe(0x200, 0);
    const std::uint32_t measure[] = {0x3C040015u, 0x24840000u, 0x03E00008u, 0};
    const std::uint32_t print[] = {0x3C060015u, 0x24C60000u, 0x03E00008u, 0};
    std::memcpy(exe.data(), measure, sizeof measure);
    std::memcpy(exe.data() + 0x100, print, sizeof print);
    // The level: the three measurer copies on 0x160000, 0x160200 and 0x160100 (the regular one's
    // table at 0x160000, the others' after it), the printer's copy on 0x160000.
    words(0x00180000u, {0x3C040016u, 0x24840000u, 0x03E00008u, 0});
    words(0x00180020u, {0x3C040016u, 0x24840200u, 0x03E00008u, 0});
    words(0x00180040u, {0x3C040016u, 0x24840100u, 0x03E00008u, 0});
    words(0x00180100u, {0x3C060016u, 0x24C60000u, 0x03E00008u, 0});
    const auto fn = [](auto f) { return reinterpret_cast<openrac_host_fn>(f); };
    static const openrac_fn_entry exe_fns[] = {
        {0x00104000u, fn(folded_measure), "measure"},
        {0x00104100u, fn(font_printer), "print"},
    };
    static const openrac_fn_entry level_fns[] = {
        {0x00180040u, fn(folded_measure), "measure"},
        {0x00180000u, fn(folded_measure), "measure"},
        {0x00180020u, fn(folded_measure), "measure"},
        {0x00180100u, fn(font_printer), "print"},
    };
    openrac_guest_register(OPENRAC_OVERLAY_EXE, exe_fns, 2);
    openrac_guest_register(7, level_fns, 4);
    openrac_guest_set_relocation(0x00140000u, 0x00170000u, 0, exe.data(), 0x00104000u,
                                 static_cast<std::uint32_t>(exe.size()));
    openrac_guest_set_overlay(7);
    CHECK(OPENRAC_DATA(0x00150000u) == 0x00160000u);
    CHECK(OPENRAC_DATA(0x00150004u) == 0x00160004u);  // a field of it, from the same pair
    CHECK(OPENRAC_CODE(0x00104100u) == 0x00180100u);
    CHECK(OPENRAC_CODE(0x00104000u) == 0x00180000u);  // a folded one: its lowest copy
    openrac_guest_set_overlay(OPENRAC_OVERLAY_EXE);
    openrac_guest_set_relocation(0, 0, 0, exe.data(), 0, 0);
}

void put32(std::vector<std::uint8_t>& f, std::size_t at, std::uint32_t v) {
    for (int i = 0; i < 4; i++) {
        f[at + i] = static_cast<std::uint8_t>(v >> (8 * i));
    }
}

void put16(std::vector<std::uint8_t>& f, std::size_t at, std::uint16_t v) {
    f[at] = static_cast<std::uint8_t>(v);
    f[at + 1] = static_cast<std::uint8_t>(v >> 8);
}

// A minimal executable: one segment of 8 bytes of data and 8 of .bss.
std::vector<std::uint8_t> tiny_elf() {
    std::vector<std::uint8_t> f(0x80, 0);
    std::memcpy(
        f.data(),
        "\x7f"
        "ELF",
        4
    );
    f[4] = 1;  // 32-bit
    f[5] = 1;  // little-endian
    f[6] = 1;
    put16(f, 16, 2);  // executable
    put16(f, 18, 8);  // MIPS
    put32(f, 24, 0x00100008);
    put32(f, 28, 0x34);  // program headers
    put16(f, 42, 32);
    put16(f, 44, 1);
    const std::size_t ph = 0x34;
    put32(f, ph + 0, 1);         // PT_LOAD
    put32(f, ph + 4, 0x60);      // offset
    put32(f, ph + 8, 0x200000);  // address
    put32(f, ph + 16, 8);        // file size
    put32(f, ph + 20, 16);       // memory size
    put32(f, ph + 24, 6);
    for (int i = 0; i < 8; i++) {
        f[0x60 + i] = static_cast<std::uint8_t>(0xA0 + i);
    }
    return f;
}

void elf() {
    std::vector<std::uint8_t> f = tiny_elf();
    ElfImage image;
    std::string reason;
    CHECK(openrac::runtime::read_elf(f, &image, &reason));
    CHECK(image.entry == 0x00100008);
    CHECK(image.segments.size() == 1);
    std::memset(G(0x200000), 0xFF, 16);
    CHECK(openrac::runtime::load_elf(f, image, &reason));
    CHECK(GREF(std::uint8_t, 0x200000) == 0xA0);
    CHECK(GREF(std::uint8_t, 0x200007) == 0xA7);
    CHECK(GREF(std::uint8_t, 0x200008) == 0);  // .bss zeroed
    CHECK(GREF(std::uint8_t, 0x20000F) == 0);

    // The image as loaded comes back over what a program wrote since, only inside the range asked.
    std::memset(G(0x200000), 0x55, 16);
    CHECK(openrac_guest_restore_executable(0x200004, 0x200100) == 12);
    CHECK(GREF(std::uint8_t, 0x200003) == 0x55);
    CHECK(GREF(std::uint8_t, 0x200004) == 0xA4);
    CHECK(GREF(std::uint8_t, 0x200007) == 0xA7);
    CHECK(GREF(std::uint8_t, 0x20000F) == 0);
    CHECK(openrac_guest_restore_executable(0x300000, 0x300100) == 0);

    std::vector<std::uint8_t> bad = f;
    put16(bad, 18, 3);  // x86
    CHECK(!openrac::runtime::read_elf(bad, &image, &reason));
    CHECK(reason == "not a MIPS program");

    std::vector<std::uint8_t> outside = f;
    put32(outside, 0x34 + 8, 0x05000000);  // past the end of RAM
    CHECK(openrac::runtime::read_elf(outside, &image, &reason));
    CHECK(!openrac::runtime::load_elf(outside, image, &reason));
}

volatile bool g_asked = false;

// A function of its own, so its caller is a frame above it; the store after the call keeps the
// call from being its last act, which would drop this frame (a tail call).
[[gnu::noinline]] void ask_caller(char* text, std::size_t size) {
    openrac_host_caller(0, text, size);
    g_asked = true;
}

void host_stack() {
    char text[160] = "";
    ask_caller(text, sizeof(text));
    CHECK(g_asked);
    CHECK(text[0] != '\0' && std::strcmp(text, "?") != 0);  // the host names who asked
#if defined(_WIN32)
    // On Windows the name is the module and the offset in it.
    CHECK(std::strstr(text, "runtime_test") != nullptr && std::strstr(text, "+0x") != nullptr);
#endif
    char small[4] = "abc";
    openrac_host_caller(64, small, sizeof(small));  // deeper than it looks: no name, nothing overrun
    CHECK(std::strcmp(small, "?") == 0);
    openrac_host_caller(0, small, 0);  // no room: left alone
    CHECK(std::strcmp(small, "?") == 0);
}

}  // namespace

int main() {
    Memory::create();
    memory_map();
    frames_and_strings();
    functions();
    code_in_copy_jal();
    relocation_with_folded_copies();
    elf();
    host_stack();
    return openrac::test::result();
}
