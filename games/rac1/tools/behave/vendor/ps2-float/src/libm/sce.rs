//! The same fdlibm as [`super`], as one Sony SDK `libm.a` builds it: an older fdlibm vintage, compiled by a
//! toolchain that converted decimal literals **toward zero**. Verified bit for bit against the copy that Hot
//! Shots Tennis (SCUS-97610) links, each function over 20k inputs with `ps2-oracle`.
//!
//! What differs from [`super`]:
//!
//! - `acosf` and `powf`: every polynomial constant one ulp below the correctly rounded decimal, the term order
//!   of that build, and `powf`'s older `t_h` seed (`+ 0x40000`, unmasked).
//! - `sinf`, `cosf`, `tanf` above 201 radians: `__kernel_rem_pio2f`'s `PIo2` pieces, six of them one ulp below
//!   the exact value (the same literal conversion). Below that the reduction never reads them.
//! - `atan2f`: the older edge cases (a denormal `y` or `x` counts as zero, no `tiny` nudges, `|y/x| > 2^60`
//!   gives `π/2` without the `pi_lo` half) and π written one ulp low.
//!
//! `atanf sqrtf` are the same code in both builds and are re-exported from [`super`]; `sinf cosf tanf` share its
//! code and only pass their own table.
//!
//! Try this flavour when the oracle says [`super`]'s `acosf`/`powf`/`atan2f` are one ulp off your game's.
//!
//! `powf` covers what such games call it with: `x > 0` and `|y| < 2^27` (plus fdlibm's answer for a negative
//! `x` to a fractional power, `+max` on the EE); other arguments panic rather than guess.
#![deny(clippy::float_arithmetic)]

use super::b;
use crate::fpu::F;

pub use super::{atanf, sqrtf};

/// `__kernel_rem_pio2f`'s `PIo2` as this build has it: `π/2` in 8-bit pieces, written in decimal and converted
/// toward zero, so the pieces whose decimal wasn't exact land one ulp low.
const PIO2: [u32; 11] = [
    0x3fc90000, 0x39f00000, 0x37d9ffff, 0x33a20000, 0x2e83ffff, 0x2b4fffff, 0x27c1ffff, 0x22cfffff, 0x1fc40000,
    0x1bc5ffff, 0x17440000,
];

/// `sinf`, this build's: [`super::sinf`] with its own `PIo2`.
pub fn sinf(x: f32) -> f32 {
    super::sinf_with(x, &PIO2)
}

/// `cosf`, this build's: [`super::cosf`] with its own `PIo2`.
pub fn cosf(x: f32) -> f32 {
    super::cosf_with(x, &PIO2)
}

/// `tanf`, this build's: [`super::tanf`] with its own `PIo2`.
pub fn tanf(x: f32) -> f32 {
    super::tanf_with(x, &PIO2)
}

/// `atan2f(y, x)`, this build's: denormal arguments count as zero, as everywhere on the EE.
pub fn atan2f(y: f32, x: f32) -> f32 {
    const PI: u32 = 0x4049_0fda;
    const HALF_PI: u32 = 0x3fc9_0fda;
    const PI_LO: u32 = 0x3422_2168;
    let (hy, hx) = (y.to_bits() as i32, x.to_bits() as i32);
    if hx == 0x3f80_0000 {
        return atanf(y);
    }
    let (iy, ix) = (hy & 0x7fff_ffff, hx & 0x7fff_ffff);
    let m = ((hy as u32) >> 31) | ((hx >> 30) & 2) as u32;
    if iy <= 0x007f_ffff {
        match m {
            2 => return b(PI),
            3 => return (-F(b(PI))).0,
            _ => return y,
        }
    }
    if ix <= 0x007f_ffff {
        return if hy < 0 { (-F(b(HALF_PI))).0 } else { b(HALF_PI) };
    }
    let k = (iy - ix) >> 23;
    let z = if k >= 61 {
        F(b(HALF_PI))
    } else if hx < 0 && k < -60 {
        F::ZERO
    } else {
        F(atanf((F(y) / x).0.abs()))
    };
    let r = match m {
        0 => z,
        1 => -z,
        2 => b(PI) - (z - b(PI_LO)),
        _ => z - b(PI_LO) - b(PI),
    };
    r.0
}

// --- acos ---------------------------------------------------------------------------------------------------------


/// `acosf`. Its `sqrtf` is fdlibm's bit-by-bit one, correctly rounded.
pub fn acosf(x: f32) -> f32 {
    const PI: f32 = b(0x4049_0fda);
    const PIO2_HI: f32 = b(0x3fc9_0fda);
    const PIO2_LO: f32 = b(0x33a2_2168);
    // p(z)/q(z) in this build's term order: p = z·(pS0 + z·(pS1 + … z·pS5)), q = 1 + z·(qS1 + … z·qS4)
    let pq = |z: F| {
        let mut p = z * b(0x3811_ef08) + b(0x3a4f_7f04);
        for c in [0xbd24_1146, 0x3e4e_0aa8, 0xbea6_b08f, 0x3e2a_aaaa] {
            p = z * p + b(c);
        }
        let mut q = z * b(0x3d9d_c62d) + b(0xbf30_3360);
        for c in [0x4001_572c, 0xc019_d138] {
            q = z * q + b(c);
        }
        z * p / (z * q + 1.0)
    };
    let hx = x.to_bits() as i32;
    let ix = hx & 0x7fff_ffff;
    if ix == 0x3f80_0000 {
        return if hx > 0 { 0.0 } else { PI };
    }
    if ix > 0x3f80_0000 {
        return f32::NAN;
    }
    let x = F(x);
    if ix < 0x3f00_0000 {
        if ix <= 0x2300_0000 {
            return PIO2_HI;
        }
        let r = pq(x * x);
        return (PIO2_HI - (x - (PIO2_LO - x * r))).0;
    }
    if hx < 0 {
        let z = (x + 1.0) * 0.5;
        let r = pq(z);
        let s = F(sqrtf(z.0));
        let w = r * s - PIO2_LO;
        let t = s + w;
        return (PI - (t + t)).0;
    }
    let z = (1.0 - x) * 0.5;
    let s = F(sqrtf(z.0));
    let df = F(b(s.0.to_bits() & 0xffff_f000));
    let c = (z - df * df) / (s + df);
    let r = pq(z);
    let t = df + (r * s + c);
    (t + t).0
}

// --- pow ----------------------------------------------------------------------------------------------------------

/// Whether `y` is a whole number (`y.fract() == 0`, which `core` does not have).
fn integral(y: f32) -> bool {
    let e = ((y.to_bits() >> 23) & 0xff) as i32 - 127;
    match e {
        23.. => true,
        0..23 => y.to_bits() & (0x7f_ffff >> e) == 0,
        _ => y.to_bits() & 0x7fff_ffff == 0,
    }
}

/// The 12-bit-mantissa head of a float: fdlibm's way of splitting one value into two exactly
/// multipliable halves.
fn hi12(x: F) -> F {
    F(b(x.0.to_bits() & 0xffff_f000))
}

/// `__ieee754_powf(x, y)` for `x > 0` and `|y| < 2^27`, plus fdlibm's answer for a negative `x` to a
/// fractional power. This build's constant roundings and the older `t_h` seed (`+ 0x40000`, unmasked).
pub fn powf(x: f32, y: f32) -> f32 {
    if y == 0.5 && x.is_sign_positive() && x.is_finite() {
        return sqrtf(x); // its sqrtf: the exact bit-by-bit one
    }
    // a negative x to a fractional power is fdlibm's (x − x) / (x − x): 0/0, +max on the EE
    if x < 0.0 && x.to_bits() & 0x7fff_ffff >= 0x80_0000 && x != -1.0 && !integral(y) && y.abs() < 1.0e8 {
        return ((F(x) - x) / (F(x) - x)).0;
    }
    // not ported: other x ≤ 0, |y| ≥ 2^27, |x| near 1 with huge y, over/underflow
    assert!(x > 0.0 && x.is_finite() && y.abs() < 1.0e8, "powf({x}, {y})");
    if y.to_bits() & 0x7fff_ffff < 0x80_0000 {
        return 1.0;
    }
    if y == 1.0 {
        return x;
    }
    if y == 2.0 {
        return (F(x) * x).0;
    }
    let (bp, dp_h, dp_l) = ([1.0, 1.5], [0.0, b(0x3f15_c000)], [0.0, b(0x35d1_cfdc)]);
    let ix = x.to_bits();
    let mut n = (ix >> 23) as i32 - 0x7f;
    let j = ix & 0x7f_ffff;
    let mut ix = j | 0x3f80_0000;
    let k = if j <= 0x1c_c471 {
        0
    } else if j < 0x5d_b3d7 {
        1
    } else {
        n += 1;
        ix -= 0x80_0000;
        0
    };
    let ax = F(b(ix));
    // log2(ax) as t1 + t2
    let u = ax - bp[k];
    let v = 1.0 / (ax + bp[k]);
    let ss = u * v;
    let s_h = hi12(ss);
    let t_h = F(b(((ix >> 1) | 0x2000_0000) + ((k as u32) << 21) + 0x4_0000));
    let t_l = ax - (t_h - bp[k]);
    let s_l = v * (u - s_h * t_h - s_h * t_l);
    let s2 = ss * ss;
    let mut r = F(b(0x3e53_f142));
    for c in [0x3e6c_3254, 0x3e8b_a304, 0x3eaa_aaab, 0x3edb_6db7, 0x3f19_9999] {
        r = s2 * r + b(c);
    }
    let r = s2 * s2 * r + s_l * (s_h + ss);
    let s2 = s_h * s_h;
    let t_h = hi12(s2 + 3.0 + r);
    let t_l = r - (t_h - 3.0 - s2);
    let u = s_h * t_h;
    let v = s_l * t_h + t_l * ss;
    let p_h = hi12(u + v);
    let p_l = v - (p_h - u);
    let z_h = p_h * b(0x3f76_3800);
    let z_l = p_h * b(0x369d_c39f) + p_l * b(0x3f76_384e) + dp_l[k];
    let t = n as f32;
    let t1 = hi12(z_h + z_l + dp_h[k] + t);
    let t2 = z_l - (t1 - t - dp_h[k] - z_h);
    // y·log2(x) as p_h + p_l, then 2^that
    let y1 = hi12(F(y));
    let p_l = (y - y1) * t1 + y * t2;
    let mut p_h = y1 * t1;
    let z = p_l + p_h;
    let jz = z.0.to_bits() as i32;
    let i = jz & 0x7fff_ffff;
    assert!(i < 0x42fc_0000, "powf({x}, {y}) out of range");
    let mut n = 0;
    if i > 0x3f00_0000 {
        let m = jz + (0x80_0000 >> ((i >> 23) - 0x7e));
        let e = ((m >> 23) & 0xff) - 0x7f;
        p_h -= b((m & !(0x7f_ffff >> e)) as u32);
        n = ((m & 0x7f_ffff) | 0x80_0000) >> (23 - e);
        if jz < 0 {
            n = -n;
        }
    }
    let t = hi12(p_l + p_h);
    let v = (p_l - (t - p_h)) * b(0x3f31_7217) + t * b(0x35bf_be8c);
    let z = t * b(0x3f31_7200) + v;
    let w = v - (z - t * b(0x3f31_7200));
    let t = z * z;
    let mut r = F(b(0x3331_bb4b));
    for c in [0xb5dd_ea0e, 0x388a_b354, 0xbb36_0b60, 0x3e2a_aaaa] {
        r = t * r + b(c);
    }
    let t1 = z - t * r;
    let r = z * t1 / (t1 - 2.0) - (w + z * w);
    let z = 1.0 - (r - z);
    let j = z.0.to_bits() as i32 + (n << 23);
    assert!(j >> 23 > 0, "powf({x}, {y}) underflows");
    b(j as u32)
}

