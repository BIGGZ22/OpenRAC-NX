// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "sys/snapshot.h"

#include <zlib.h>

#include <cstdio>
#include <cstring>

namespace sys {

namespace {

/** The file starts with this: "ORSNAP" and the format's number. */
constexpr char kMagic[8] = {'O', 'R', 'S', 'N', 'A', 'P', '1', 0};

/** Writes a block as its size before and after compression, then the compressed bytes. */
bool put_block(std::FILE* f, const std::vector<u8>& data) {
    uLongf packed = compressBound(static_cast<uLong>(data.size()));
    std::vector<u8> out(packed);

    // Level 1: the memory is mostly zeroes and repeats, speed matters more.
    if (compress2(out.data(), &packed, data.data(), static_cast<uLong>(data.size()), 1) != Z_OK) {
        return false;
    }

    u64 sizes[2] = {data.size(), packed};
    return std::fwrite(sizes, sizeof(sizes), 1, f) == 1 && std::fwrite(out.data(), 1, packed, f) == packed;
}

/** Reads what `put_block` wrote. */
bool get_block(std::FILE* f, std::vector<u8>& data) {
    u64 sizes[2];

    if (std::fread(sizes, sizeof(sizes), 1, f) != 1 || sizes[0] > (u64{1} << 26) || sizes[1] > (u64{1} << 27)) {
        return false;
    }

    std::vector<u8> in(sizes[1]);
    data.resize(sizes[0]);
    uLongf size = static_cast<uLongf>(sizes[0]);

    return std::fread(in.data(), 1, in.size(), f) == in.size()
        && uncompress(data.data(), &size, in.data(), static_cast<uLong>(in.size())) == Z_OK
        && size == sizes[0];
}

/** The registers, in the file's order, as one run of bytes each way. */
template <typename State, typename Fn>
void registers(State& s, Fn&& fn) {
    fn(&s.address, sizeof(s.address));
    fn(s.gpr.data(), sizeof(s.gpr));
    fn(s.fpr.data(), sizeof(s.fpr));
    fn(&s.hi, sizeof(u64) * 4);
    fn(&s.sa, sizeof(u32) * 3);
    fn(s.vf.data(), sizeof(s.vf));
    fn(s.vi.data(), sizeof(s.vi));
    fn(s.acc.data(), sizeof(s.acc));
    fn(&s.q, sizeof(u32) * 7);
}

}  // namespace

void CallState::take(const ps2::Ee& ee, const ps2::Vu& vu0, ps2::GuestMemory& memory,
                     const u8* micro, const u8* data) {
    gpr = ee.gpr;
    fpr = ee.fpr;
    hi = ee.hi;
    lo = ee.lo;
    hi1 = ee.hi1;
    lo1 = ee.lo1;
    sa = ee.sa;
    facc = ee.facc;
    fcr31 = ee.fcr31;
    vf = vu0.vf;
    vi = vu0.vi;
    acc = vu0.acc;
    q = vu0.q;
    p = vu0.p;
    i = vu0.i;
    r = vu0.r;
    mac = vu0.mac;
    status = vu0.status;
    clip = vu0.clip;
    ram.assign(memory.ram(0), memory.ram(0) + ps2::GuestMemory::kRamBytes);
    scratchpad.assign(memory.scratchpad(0), memory.scratchpad(0) + ps2::GuestMemory::kScratchpadBytes);
    vu0_micro.assign(micro, micro + 4096);
    vu0_data.assign(data, data + 4096);
}

void CallState::put(ps2::Ee& ee, ps2::Vu& vu0, ps2::GuestMemory& memory, u8* micro, u8* data) const {
    ee.gpr = gpr;
    ee.fpr = fpr;
    ee.hi = hi;
    ee.lo = lo;
    ee.hi1 = hi1;
    ee.lo1 = lo1;
    ee.sa = sa;
    ee.facc = facc;
    ee.fcr31 = fcr31;
    vu0.vf = vf;
    vu0.vi = vi;
    vu0.acc = acc;
    vu0.q = q;
    vu0.p = p;
    vu0.i = i;
    vu0.r = r;
    vu0.mac = mac;
    vu0.status = status;
    vu0.clip = clip;
    std::memcpy(memory.ram(0), ram.data(), ram.size());
    std::memcpy(memory.scratchpad(0), scratchpad.data(), scratchpad.size());
    std::memcpy(micro, vu0_micro.data(), vu0_micro.size());
    std::memcpy(data, vu0_data.data(), vu0_data.size());
}

bool CallState::save(const std::string& path) const {
    std::FILE* f = std::fopen(path.c_str(), "wb");

    if (!f) {
        return false;
    }

    bool ok = std::fwrite(kMagic, sizeof(kMagic), 1, f) == 1;
    registers(*this, [&](const void* p, std::size_t n) { ok = ok && std::fwrite(p, n, 1, f) == 1; });
    ok = ok && put_block(f, ram) && put_block(f, scratchpad) && put_block(f, vu0_micro)
        && put_block(f, vu0_data);
    ok = std::fclose(f) == 0 && ok;

    return ok;
}

bool CallState::load(const std::string& path, std::string* error) {
    std::FILE* f = std::fopen(path.c_str(), "rb");

    if (!f) {
        *error = "cannot open " + path;
        return false;
    }

    char magic[8];
    bool ok = std::fread(magic, sizeof(magic), 1, f) == 1 && std::memcmp(magic, kMagic, 8) == 0;
    registers(*this, [&](void* p, std::size_t n) { ok = ok && std::fread(p, n, 1, f) == 1; });
    ok = ok && get_block(f, ram) && ram.size() == ps2::GuestMemory::kRamBytes
        && get_block(f, scratchpad) && scratchpad.size() == ps2::GuestMemory::kScratchpadBytes
        && get_block(f, vu0_micro) && vu0_micro.size() == 4096 && get_block(f, vu0_data)
        && vu0_data.size() == 4096;
    std::fclose(f);

    if (!ok) {
        *error = path + " is not a call state written by openrac-boot --capture";
    }

    return ok;
}

}  // namespace sys
