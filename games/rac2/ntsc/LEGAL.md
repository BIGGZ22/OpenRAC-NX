# Distribution and source scope

This repository holds authored C, tools and function catalogues (names, sizes, fingerprints
and addresses, never bytes) for Ratchet & Clank: Going Commando, USA v1.01.

Disc images, extracted executables and level programs, disassembly, textures, audio and other
game assets are never committed: `baserom/` and `build/` stay local, and every contributor
supplies their own disc. Proprietary tools (SN ProDG's assembler and linker) stay in
`toolchain/`, which is never committed either; the compilers are rebuilt from public source
(`tools/compiler/`).

The [GPL-3.0 licence](LICENSE) covers the project's own work; code released earlier under MIT
keeps its notice in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). `tools/compiler/` carries
patches to GCC under GCC's licence. [CREDITS.md](CREDITS.md)
records what was taken from elsewhere.
