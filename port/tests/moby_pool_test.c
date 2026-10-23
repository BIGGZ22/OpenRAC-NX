/* SPDX-License-Identifier: GPL-3.0-or-later */
/* The bigger moby pool (rac1/host/moby_pool.c, PAL's table): while the level loader runs, the
 * gameplay header it named gets at least 768 spare slots as its decompression ends (but on the
 * three levels whose heap has no room); nothing else
 * that is decompressed changes, nor the header outside a run, and the loader's argument and result
 * pass through. And the tick's run list keeps its target list to 511 mobys, the rest still run. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "%d: %s\n", __LINE__, #c); exit(1); } } while (0)
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif
#include "../game/rac1/host/moby_pool.c"

/* Guest memory: the first 4 MB and the scratchpad, at their addresses. */
enum { RAM = 0x400000 };
uint8_t* openrac_guest_base;
static void map_guest(void) {
    const size_t span = GUEST_SCRATCHPAD + GUEST_SCRATCHPAD_SIZE;
#ifdef _WIN32
    uint8_t* p = VirtualAlloc(NULL, span, MEM_RESERVE, PAGE_NOACCESS);
    CHECK(p != NULL);
    CHECK(VirtualAlloc(p, RAM, MEM_COMMIT, PAGE_READWRITE) != NULL);
    CHECK(VirtualAlloc(p + GUEST_SCRATCHPAD, GUEST_SCRATCHPAD_SIZE, MEM_COMMIT, PAGE_READWRITE) != NULL);
#else
    uint8_t* p = mmap(NULL, span, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
    CHECK(p != MAP_FAILED);
    CHECK(mprotect(p, RAM, PROT_READ | PROT_WRITE) == 0);
    CHECK(mprotect(p + GUEST_SCRATCHPAD, GUEST_SCRATCHPAD_SIZE, PROT_READ | PROT_WRITE) == 0);
#endif
    openrac_guest_base = p;
}
gaddr openrac_relocate_low = 0, openrac_relocate_high = 0;
gaddr openrac_relocate_data(gaddr a) { return a; }
gaddr openrac_relocate_level_data(int from, gaddr a) { (void)from; return a; }

static int warnings;
void openrac_game_warn(const char* message) {
    (void)message;
    ++warnings;
}

enum { LOAD_STATE = 0x00173F00, HEADER = 0x100000, OTHER = 0x140000, SECTION = 0x2000 };

/* A gameplay header at `at`: the moby section at +SECTION, `placed` mobys and `spare` spare. */
static void header(gaddr at, int placed, int spare) {
    GREF(int, at + 0x44) = SECTION;
    GREF(int, at + SECTION) = placed;
    GREF(int, at + SECTION + 4) = spare;
}

static int spare_of(gaddr at) { return GREF(int, at + SECTION + 4); }

/* The loader as the game runs it: its header decompressed (with another WAD before it), the spare
 * count read as the C reads it. */
static int shipped, seen, other_seen, loads, last_arg;
gaddr func_L00_002422D8__hooked(int a0) {
    ++loads;
    last_arg = a0;
    header(OTHER, 7, 256);
    rac1_wad_decompressed(OTHER);
    other_seen = spare_of(OTHER);
    header(HEADER, 1569, shipped);
    rac1_wad_decompressed(HEADER);
    seen = spare_of(HEADER);
    return 0x01234560u;
}

/* `count` live mobys in an array at ARRAY, each active (drawn last tick), in no group, of class
 * slot i % 3, with mode 0x1000 on all but every tenth; the tick's run list built. */
enum { ARRAY = 0x200000, NEXT_TABLE = 0x001C8480 };
static gaddr run(int count) {
    memset(G(ARRAY), 0, (size_t)(count + 1) * 0x100u);
    for (int i = 0; i < count; ++i) {
        const gaddr m = ARRAY + (gaddr)i * 0x100u;
        GREF(signed char, m + 0x21) = -1;
        GREF(unsigned char, m + 0x22) = (unsigned char)(i % 3);
        GREF(unsigned char, m + 0x31) = 1;
        GREF(unsigned short, m + 0x34) = i % 10 == 9 ? 0 : 0x1000;
    }
    GREF(unsigned char, ARRAY + (gaddr)count * 0x100u + 0x20) = 0xFF;
    GREF(gaddr, 0x00160018u) = ARRAY;
    GREF(gaddr, NEXT_TABLE) = 0xA5A5A5A5u;
    return func_0020E6B8();
}

/* How many mobys the run list starting at `first` links. */
static int run_length(gaddr first) {
    int n = 0;
    for (gaddr m = first; m != 0; m = GREF(gaddr, m + 0x28)) {
        ++n;
    }
    return n;
}

int main(void) {
    map_guest();
    GREF(int, 0x0015EE84u) = 9; /* Gaspar */
    GREF(gaddr, LOAD_STATE + 4) = HEADER;

    /* A level's 256 become 768; a WAD decompressed elsewhere in the run keeps its own. */
    shipped = 256;
    CHECK(func_L00_002422D8(1) == 0x01234560u);
    CHECK(loads == 1 && last_arg == 1);
    CHECK(seen == 768 && other_seen == 256);
    CHECK(GREF(int, HEADER + SECTION) == 1569);

    /* Drek's fleet's 400 become 768 too; a count above 768 stays. */
    shipped = 400;
    func_L00_002422D8(0);
    CHECK(seen == 768 && last_arg == 0);
    shipped = 1000;
    func_L00_002422D8(0);
    CHECK(seen == 1000);

    /* Rilgar, Blarg Station and Quartu keep the disc's count: their heap has no room. */
    for (int level = 0; level < 19; ++level) {
        GREF(int, 0x0015EE84u) = level;
        shipped = 256;
        func_L00_002422D8(0);
        CHECK(seen == (level == 5 || level == 6 || level == 15 ? 256 : 768));
    }
    GREF(int, 0x0015EE84u) = 9;

    /* Outside a loader run, the same header decompressed again stays as it is. */
    header(HEADER, 1569, 256);
    rac1_wad_decompressed(HEADER);
    CHECK(spare_of(HEADER) == 256);

    /* Each run raises the header its own load state names. */
    GREF(gaddr, LOAD_STATE + 4) = OTHER;
    shipped = 256;
    func_L00_002422D8(1);
    CHECK(seen == 256 && other_seen == 768);

    /* A tick with 100 mobys: the 90 with 0x1000 in the target table, in array order, NULL after;
     * the run list links all 100, class slot by class slot. */
    gaddr first = run(100);
    CHECK(run_length(first) == 100 && first == ARRAY);
    CHECK(GREF(gaddr, 0x001C7C80u) == ARRAY && GREF(gaddr, 0x001C7C80u + 4) == ARRAY + 0x100);
    CHECK(GREF(gaddr, 0x001C7C80u + 9 * 4) == ARRAY + 10 * 0x100); /* the tenth has no 0x1000 */
    CHECK(GREF(gaddr, 0x001C7C80u + 90 * 4) == 0 && warnings == 0);

    /* 900 (810 with 0x1000): 511 in the table, its NULL in its last word, the next table and the
     * scratchpad's group flags untouched, all 900 still run; one warning, once. */
    first = run(900);
    CHECK(run_length(first) == 900);
    CHECK(GREF(gaddr, 0x001C7C80u + 510 * 4) != 0 && GREF(gaddr, 0x001C7C80u + 511 * 4) == 0);
    CHECK(GREF(gaddr, NEXT_TABLE) == 0xA5A5A5A5u);
    for (gaddr f = 0x70000C80u; f != 0x70000D00u; ++f) {
        CHECK(GREF(unsigned char, f) == 0);
    }
    CHECK(warnings == 1);
    run(900);
    CHECK(warnings == 1);
    puts("moby_pool: ok");
    return 0;
}
