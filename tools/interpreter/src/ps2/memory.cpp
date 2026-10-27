// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * Mapping the guest's memories into one reservation of address space (see memory.h).
 */

#include "memory.h"

#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>

#include <sys/mman.h>

namespace ps2 {
namespace {

/** The whole of a 32-bit address space. */
constexpr std::size_t kSpaceBytes = std::size_t{1} << 32;

/** Where main memory answers besides address 0: the uncached and kernel segments (documented). */
constexpr u32 kMirrors[] = {0x20000000, 0x80000000, 0xA0000000};

/**
 * Maps readable and writable memory at a fixed place inside the reservation.
 *
 * @param at Where; a multiple of the host's page size.
 * @param bytes How much.
 * @param file A memory object to map, or -1 for fresh zero-filled memory.
 * @return True if the mapping is there.
 */
bool map_at(u8* at, std::size_t bytes, int file) {
    // Shared for a memory object, so that every mapping of it is the same memory.
    int flags = MAP_FIXED | (file >= 0 ? MAP_SHARED : MAP_PRIVATE | MAP_ANON);

    return mmap(at, bytes, PROT_READ | PROT_WRITE, flags, file, 0) != MAP_FAILED;
}

/**
 * Makes a memory object of a size, with no name left behind.
 *
 * @param bytes Its size.
 * @return Its descriptor, or -1.
 */
int memory_object(std::size_t bytes) {
    char name[32];

    // The name only has to be free for the moment between creating and unlinking it.
    std::snprintf(name, sizeof(name), "/openrac-%d", static_cast<int>(getpid()));

    int file = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);

    // No such object could be made.
    if (file < 0) {
        return -1;
    }

    shm_unlink(name);

    // It could not be given its size.
    if (ftruncate(file, static_cast<off_t>(bytes)) != 0) {
        close(file);
        return -1;
    }

    return file;
}

}  // namespace

GuestMemory::GuestMemory() {
    void* space = mmap(nullptr, kSpaceBytes, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);

    // Without address space there is no machine.
    if (space == MAP_FAILED) {
        std::fprintf(stderr, "openrac: cannot reserve the guest's address space\n");
        std::abort();
    }

    space_ = static_cast<u8*>(space);
    ram_ = space_;
    scratchpad_ = space_ + kScratchpadBase;

    int file = memory_object(kRamBytes);

    // Main memory as one object mapped at each mirror; failing that, plain memory at 0 only.
    mirrored_ = file >= 0 && map_at(ram_, kRamBytes, file);

    for (u32 mirror : kMirrors) {
        mirrored_ = mirrored_ && map_at(space_ + mirror, kRamBytes, file);
    }

    if (file >= 0) {
        close(file);
    }

    bool mapped = mirrored_ || map_at(ram_, kRamBytes, -1);

    mapped = mapped && map_at(scratchpad_, kScratchpadBytes, -1);

    // The interpreter cannot run without its memories.
    if (!mapped) {
        std::fprintf(stderr, "openrac: cannot map the guest's memory\n");
        std::abort();
    }
}

GuestMemory::~GuestMemory() {
    munmap(space_, kSpaceBytes);
}

}  // namespace ps2
