// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/pss.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The PSS demuxer and its audio (pss.h).

#include "assets/sound/pss.h"

#include <algorithm>
#include <array>
#include <cstring>

#include "assets/sound/adpcm.h"

namespace openrac::assets {

namespace {

constexpr u8 kAudioSubstream[3] = {0xff, 0xa1, 0x00};

bool is_pes_stream(u8 id) {
    return id == kPssPrivate1 || (id >= 0xc0 && id <= 0xef);
}

// A 33-bit PES time stamp at `p` (5 bytes, marker bits between the parts).
u64 time_stamp(const u8* p) {
    return (static_cast<u64>(p[0] >> 1) & 7) << 30 | static_cast<u64>(p[1]) << 22
           | static_cast<u64>(p[2] >> 1) << 15 | static_cast<u64>(p[3]) << 7
           | static_cast<u64>(p[4] >> 1);
}

}  // namespace

std::optional<PssPacket> PssDemuxer::next() {
    const u8* d = m_data.data();
    const std::size_t size = m_data.size();

    for (;;) {
        const std::size_t i = m_pos;

        if (m_ended || i + 4 > size) {
            return std::nullopt;
        }

        if (d[i] != 0 || d[i + 1] != 0 || d[i + 2] != 1) {
            fail("pss: no start code at {:#x}", i);
        }

        const u8 id = d[i + 3];

        if (id == kPssPack) {
            if (i + 14 > size) {
                fail("pss: truncated pack header at {:#x}", i);
            }

            if (d[i + 4] >> 6 != 1) {
                fail("pss: the pack at {:#x} is not MPEG-2", i);
            }

            m_pos = i + 14 + (d[i + 13] & 7);
            ++m_packs;
            continue;
        }

        if (id == kPssEnd) {
            m_pos = i + 4;
            m_ended = true;
            return std::nullopt;
        }

        if (id < kPssSystemHeader) {
            fail("pss: unexpected start code {:#x} at {:#x}", id, i);
        }

        if (i + 6 > size) {
            fail("pss: truncated packet header at {:#x}", i);
        }

        const std::size_t length = static_cast<std::size_t>(d[i + 4]) << 8 | d[i + 5];
        const std::size_t end = i + 6 + length;

        if (end > size) {
            fail("pss: packet {:#x} at {:#x} runs past the end ({:#x} > {:#x})", id, i, end, size);
        }

        m_pos = end;
        PssPacket packet;
        packet.offset = i;
        packet.id = id;

        if (id == kPssSystemHeader || id == kPssPadding || !is_pes_stream(id)) {
            packet.payload = m_data.sub(i + 6, length);
            return packet;
        }

        // The MPEG-2 PES header: '10' marker, flags, header data length.
        if (length < 3 || d[i + 6] >> 6 != 2) {
            fail("pss: packet {:#x} at {:#x} has no MPEG-2 PES header", id, i);
        }

        const u8 flags = d[i + 7];
        const std::size_t header_length = d[i + 8];
        const std::size_t body = i + 9 + header_length;

        if (body > end) {
            fail("pss: the PES header of {:#x} at {:#x} is longer than the packet", id, i);
        }

        if ((flags & 0x80) && header_length >= 5) {
            packet.pts = time_stamp(d + i + 9);
        }

        if ((flags & 0xc0) == 0xc0 && header_length >= 10) {
            packet.dts = time_stamp(d + i + 14);
        }

        ByteView payload = m_data.sub(body, end - body);

        if (id == kPssPrivate1) {
            if (payload.size() < 4 || std::memcmp(payload.data(), kAudioSubstream, 3) != 0) {
                fail("pss: private stream 1 at {:#x} has no audio sub-stream header", i);
            }

            packet.kind = PssStreamKind::Audio;
            packet.id = payload.data()[3];
            packet.payload = payload.tail(4);
        } else if (id >= kPssVideo) {
            packet.kind = PssStreamKind::Video;
            packet.payload = payload;
        } else {
            packet.payload = payload;
        }

        return packet;
    }
}

PssStreams demux_pss(ByteView data) {
    PssStreams out;
    PssDemuxer demuxer(data);

    while (const std::optional<PssPacket> p = demuxer.next()) {
        switch (p->kind) {
            case PssStreamKind::Video:
                if (!out.first_video_pts) {
                    out.first_video_pts = p->pts;
                }

                out.video.insert(
                    out.video.end(), p->payload.data(), p->payload.data() + p->payload.size()
                );
                ++out.video_packets;
                break;
            case PssStreamKind::Audio: {
                std::vector<u8>& channel = out.audio[p->id];
                channel.insert(
                    channel.end(), p->payload.data(), p->payload.data() + p->payload.size()
                );
                ++out.audio_packets;
                break;
            }
            case PssStreamKind::Other:
                ++out.other_packets;
                break;
        }
    }

    out.packs = demuxer.packs();
    out.ended = demuxer.ended();
    out.trailing = data.size() - demuxer.position();
    return out;
}

PssAudioHeader parse_pss_audio_header(ByteView bytes) {
    if (bytes.size() < kPssAudioHeaderSize) {
        fail("pss audio: {} bytes, no SShd header", bytes.size());
    }

    if (std::memcmp(bytes.data(), "SShd", 4) != 0 || bytes.u32_at(4) != 0x18) {
        fail("pss audio: bad SShd header");
    }

    if (std::memcmp(bytes.data() + 0x20, "SSbd", 4) != 0) {
        fail("pss audio: no SSbd at 0x20");
    }

    PssAudioHeader h;
    h.type = bytes.u32_at(0x08);
    h.rate = bytes.u32_at(0x0c);
    h.channels = bytes.u32_at(0x10);
    h.interleave = bytes.u32_at(0x14);
    h.loop_start = bytes.s32_at(0x18);
    h.loop_end = bytes.s32_at(0x1c);
    h.body_size = bytes.u32_at(0x24);

    if (h.type != kPssAudioAdpcm) {
        fail("pss audio: type {:#x} is not SPU ADPCM", h.type);
    }

    if (h.channels < 1 || h.channels > 2 || h.interleave == 0
        || h.interleave % kAdpcmFrameBytes != 0 || h.rate == 0) {
        fail(
            "pss audio: unsupported layout ({} Hz, {} channels, interleave {:#x})",
            h.rate,
            h.channels,
            h.interleave
        );
    }

    return h;
}

PssAudio PssAudio::parse(ByteView bytes) {
    PssAudio audio;
    audio.header = parse_pss_audio_header(bytes);
    const ByteView body = bytes.tail(kPssAudioHeaderSize);
    audio.body = body.sub(0, std::min<std::size_t>(body.size(), audio.header.body_size));
    return audio;
}

std::size_t PssAudio::blocks() const {
    return body.size() / (static_cast<std::size_t>(header.interleave) * header.channels);
}

std::size_t PssAudio::samples() const {
    return blocks() * header.interleave / kAdpcmFrameBytes * kAdpcmFrameSamples;
}

double PssAudio::seconds() const {
    return static_cast<double>(samples()) / header.rate;
}

std::vector<s16> PssAudio::decode() const {
    const std::size_t interleave = header.interleave;
    const std::size_t channels = header.channels;
    const std::size_t per_block = interleave / kAdpcmFrameBytes * kAdpcmFrameSamples;
    std::vector<s16> out(samples() * 2);
    std::array<AdpcmHistory, 2> history{};

    for (std::size_t b = 0; b < blocks(); ++b) {
        for (std::size_t c = 0; c < channels; ++c) {
            const u8* block = body.data() + (b * channels + c) * interleave;

            for (std::size_t f = 0; f < interleave / kAdpcmFrameBytes; ++f) {
                const auto pcm = decode_adpcm_frame(block + f * kAdpcmFrameBytes, history[c]);
                const std::size_t at = b * per_block + f * kAdpcmFrameSamples;

                for (std::size_t k = 0; k < kAdpcmFrameSamples; ++k) {
                    out[2 * (at + k) + c] = pcm[k];

                    if (channels == 1) {
                        out[2 * (at + k) + 1] = pcm[k];
                    }
                }
            }
        }
    }

    return out;
}

}  // namespace openrac::assets
