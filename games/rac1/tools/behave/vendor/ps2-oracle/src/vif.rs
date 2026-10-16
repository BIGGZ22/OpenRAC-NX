//! VIF0 fed by a DMA source chain, enough for what a game does once at boot: upload VU0's micro programs.
//!
//! There is no DMA controller and no clock here: [`Ee::vif0_dma`] walks a chain of DMA tags in memory (as
//! the controller would after the game writes `D0_TADR` and starts channel 0 in chain mode), gathers the
//! quadwords it would send, and runs the VIF codes in them at once. `MPG` writes VU0's micro memory and
//! `MSCAL` runs a program ([`Ee::vu0_run`]); the codes that only set up unpacking or synchronise are accepted
//! and do nothing. `UNPACK` (VU0 data memory through the VIF's decompressor) is not here and panics.

use crate::Ee;

impl Ee {
    /// Run the DMA source chain whose first tag is at `tadr` into VIF0. `tte` is the channel's "transfer
    /// tag" bit: set, each tag's upper 64 bits go to the VIF too (games put VIF codes there).
    pub fn vif0_dma(&mut self, tadr: u32, tte: bool) {
        let mut stream: Vec<u32> = Vec::new();
        let (mut tag_at, mut stack) = (tadr, Vec::new());
        for _ in 0..0x10000 {
            let tag = self.read(tag_at, 16);
            let (qwc, id, addr) = (tag as u32 & 0xffff, (tag >> 28 & 7) as u32, (tag >> 32) as u32 & 0x7fff_fff0);
            if tte {
                stream.extend([(tag >> 64) as u32, (tag >> 96) as u32]);
            }
            let after = tag_at + 16;
            // (where the data is, where the next tag is; None = the chain ends with this one)
            let (data, next) = match id {
                0 => (addr, None),                        // refe
                1 => (after, Some(after + 16 * qwc)),     // cnt
                2 => (after, Some(addr)),                 // next
                3 | 4 => (addr, Some(after)),             // ref, refs
                5 => {
                    stack.push(after + 16 * qwc); // call
                    (after, Some(addr))
                }
                6 => (after, stack.pop()),                // ret (an empty stack ends the chain)
                _ => (after, None),                       // end
            };
            for k in 0..4 * qwc {
                stream.push(self.u32_at(data + 4 * k));
            }
            match next {
                Some(n) => tag_at = n,
                None => return self.vif0(&stream),
            }
        }
        panic!("VIF0 DMA chain from {tadr:#x}: no end tag after 65536 tags");
    }

    /// Run a stream of VIF0 codes (and their data words).
    pub fn vif0(&mut self, words: &[u32]) {
        let mut k = 0;
        while k < words.len() {
            let w = words[k];
            k += 1;
            let (imm, num, cmd) = (w & 0xffff, (w >> 16 & 0xff) as usize, w >> 24 & 0x7f);
            match cmd {
                // nop, stcycl, itop, stmod, mark, flushe, flush, flusha: nothing to set up or wait for
                0x00 | 0x01 | 0x04 | 0x05 | 0x07 | 0x10 | 0x11 | 0x13 => {}
                0x20 => k += 1, // stmask (only unpack reads it)
                0x30 | 0x31 => k += 4, // strow, stcol
                0x14 | 0x15 => self.vu0_run(imm, 1 << 20), // mscal, mscalf
                0x4a => {
                    // mpg: `num` 64-bit instructions (0 = 256) to micro memory at `imm` × 8
                    let n = if num == 0 { 256 } else { num };
                    assert!(k + 2 * n <= words.len(), "VIF0 MPG of {n} instructions runs past the end of the data");
                    for j in 0..2 * n {
                        let at = (8 * imm as usize + 4 * j) % crate::vu0::MEM;
                        self.vu0_micro[at..at + 4].copy_from_slice(&words[k + j].to_le_bytes());
                    }
                    k += 2 * n;
                }
                _ => panic!("unimplemented VIF0 code {w:#010x} (command {cmd:#x}). Teach it to ps2-oracle's \
                             src/vif.rs (FIZL0/ps2-oracle)."),
            }
        }
    }
}
