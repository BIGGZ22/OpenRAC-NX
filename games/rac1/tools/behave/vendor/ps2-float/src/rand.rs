//! The random generators PS2 games link, with nothing game-specific: newlib's `rand()` (the SDK's C library)
//! and MT19937 (Matsumoto and Nishimura's reference `init_genrand`/`genrand_int32`, which many games compile in
//! next to it). Integer-only, so the same on every host.
//!
//! Both are checked against a shipped game's own code over ≥10k inputs (see the README); a game's *use* of
//! them (how many draws per frame, how a draw becomes a float or a range) is game code and belongs in the port.
#![deny(clippy::float_arithmetic)]

/// newlib's `rand()`: a 64-bit LCG over the reentrancy struct's `_rand_next` (1 until `srand`); each call steps it
/// and returns bits 32..62.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[cfg_attr(feature = "serde", derive(serde::Serialize, serde::Deserialize))]
pub struct Rand(pub u64);

impl Default for Rand {
    fn default() -> Self {
        Rand(1)
    }
}

impl Rand {
    /// `srand(seed)`.
    pub fn new(seed: u32) -> Rand {
        Rand(seed as u64)
    }

    #[allow(clippy::should_implement_trait)] // named after the C function's job; an endless Iterator adds nothing
    pub fn next(&mut self) -> u32 {
        self.0 = self.0.wrapping_mul(0x5851_f42d_4c95_7f2d).wrapping_add(1);
        (self.0 >> 32) as u32 & 0x7fff_ffff
    }
}

/// An MT19937: 624 words and the next word's index (624: regenerate first).
#[derive(Clone, Debug, PartialEq, Eq)]
#[cfg_attr(feature = "serde", derive(serde::Serialize, serde::Deserialize))]
pub struct Mt(#[cfg_attr(feature = "serde", serde(with = "words"))] pub [u32; 624], pub usize);

impl Mt {
    /// `init_genrand(seed)`.
    pub fn new(seed: u32) -> Mt {
        let mut m = [0u32; 624];
        m[0] = seed;
        for i in 1..624 {
            m[i] = (i as u32).wrapping_add((m[i - 1] ^ (m[i - 1] >> 30)).wrapping_mul(0x6c07_8965));
        }
        Mt(m, 624)
    }

    /// `genrand_int32()`.
    #[allow(clippy::should_implement_trait)] // named after the C function's job; an endless Iterator adds nothing
    pub fn next(&mut self) -> u32 {
        if self.1 >= 624 {
            for k in 0..624 {
                let y = (self.0[k] & 0x8000_0000) | (self.0[(k + 1) % 624] & 0x7fff_ffff);
                self.0[k] = self.0[(k + 397) % 624] ^ (y >> 1) ^ if y & 1 != 0 { 0x9908_b0df } else { 0 };
            }
            self.1 = 0;
        }
        let mut y = self.0[self.1];
        self.1 += 1;
        y ^= y >> 11;
        y ^= (y << 7) & 0x9d2c_5680;
        y ^= (y << 15) & 0xefc6_0000;
        y ^ (y >> 18)
    }
}

/// serde has no arrays past 32: the state words as a sequence (filled in place: no `alloc` needed).
#[cfg(feature = "serde")]
mod words {
    use core::fmt;
    use serde::de::{Error, SeqAccess, Visitor};
    use serde::{Deserializer, Serializer};

    pub fn serialize<S: Serializer>(a: &[u32; 624], s: S) -> Result<S::Ok, S::Error> {
        s.collect_seq(a)
    }

    pub fn deserialize<'de, D: Deserializer<'de>>(d: D) -> Result<[u32; 624], D::Error> {
        struct Words;
        impl<'de> Visitor<'de> for Words {
            type Value = [u32; 624];
            fn expecting(&self, f: &mut fmt::Formatter) -> fmt::Result {
                f.write_str("624 words")
            }
            fn visit_seq<A: SeqAccess<'de>>(self, mut seq: A) -> Result<[u32; 624], A::Error> {
                let mut a = [0u32; 624];
                for (i, w) in a.iter_mut().enumerate() {
                    *w = seq.next_element()?.ok_or_else(|| A::Error::invalid_length(i, &self))?;
                }
                if seq.next_element::<u32>()?.is_some() {
                    return Err(A::Error::custom("not 624 words"));
                }
                Ok(a)
            }
        }
        d.deserialize_seq(Words)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn reference_outputs() {
        // MT19937's published outputs for init_genrand(5489)
        let mut m = Mt::new(5489);
        assert_eq!([m.next(), m.next(), m.next()], [3_499_211_612, 581_869_302, 3_890_346_734]);
        let mut m = Mt::new(5489);
        assert_eq!((0..10000).map(|_| m.next()).last(), Some(4_123_659_995));
    }

    #[test]
    fn newlib_rand_steps() {
        // newlib: next = next · 6364136223846793005 + 1, starting from 1; rand() = (next >> 32) & 0x7fffffff
        let mut r = Rand::default();
        assert_eq!(r.next(), 0x5851_f42d);
        assert_eq!(r, Rand(0x5851_f42d_4c95_7f2e));
        let mut r = Rand::new(1);
        assert_eq!(r.next(), 0x5851_f42d, "srand(1) is the default state");
    }
}
