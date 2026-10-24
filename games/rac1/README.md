# Ratchet & Clank (PS2) Decompilation

[![Website](https://img.shields.io/badge/Website-openrac.dev-ff8a00?logo=googlechrome&logoColor=white)](https://openrac.dev)
[![Discord](https://img.shields.io/badge/Discord-Join%20Community-5865F2?logo=discord&logoColor=white)](https://discord.gg/Sfd2B54PDG)
[![Progress report](https://github.com/OpenRAC/rac1-decomp/actions/workflows/progress.yml/badge.svg)](https://github.com/OpenRAC/rac1-decomp/actions/workflows/progress.yml)
[![Code](https://bytematch.dev/OpenRAC/rac1-decomp.svg?mode=shield&label=Code&measure=matched_code_percent)](https://bytematch.dev/OpenRAC/rac1-decomp)
[![Functions](https://bytematch.dev/OpenRAC/rac1-decomp.svg?mode=shield&label=Functions&measure=matched_functions)](https://bytematch.dev/OpenRAC/rac1-decomp)

A work-in-progress **matching decompilation** of *Ratchet & Clank* (Insomniac Games, 2002) for the PlayStation 2 (`SCES_509.16`, PAL v2.00), part of the **[OpenRAC](https://openrac.dev)** initiative.

The objective is to produce C/C++ source code that, when compiled with the original toolchain, generates a byte-identical copy of the retail executable. Matched code is then refactored toward readable, idiomatic C++ with accurate types and naming, using matching builds as continuous regression tests.

> [!NOTE]
> This repository contains **no game assets, retail executables, or disassembly**. To build, you must provide your own legally obtained copy of the game. Please review [`LEGAL.md`](LEGAL.md) before contributing.

---

## Progress

Decompilation progress is tracked live on **[openrac.dev](https://openrac.dev)** and **[bytematch.dev/OpenRAC/rac1-decomp](https://bytematch.dev/OpenRAC/rac1-decomp)**.

| Version | Region | Target ID | Code Matched | Functions Matched |
|---|---|---|---|---|
| v2.00 | PAL (En, Fr, De, Es, It) | `SCES_509.16` | [![](https://bytematch.dev/OpenRAC/rac1-decomp.svg?mode=shield&label=Code&measure=matched_code_percent)](https://bytematch.dev/OpenRAC/rac1-decomp) | [![](https://bytematch.dev/OpenRAC/rac1-decomp.svg?mode=shield&label=Functions&measure=matched_functions)](https://bytematch.dev/OpenRAC/rac1-decomp) |
| v1.00 | PAL (En, Fr, De, Es, It) | `SCES_509.16_v1` | [![](https://bytematch.dev/OpenRAC/rac1-decomp.svg?mode=shield&label=Code&measure=matched_code_percent&version=SCES_509.16_v1)](https://bytematch.dev/OpenRAC/rac1-decomp/SCES_509.16_v1) | [![](https://bytematch.dev/OpenRAC/rac1-decomp.svg?mode=shield&label=Functions&measure=matched_functions&version=SCES_509.16_v1)](https://bytematch.dev/OpenRAC/rac1-decomp/SCES_509.16_v1) |
| v1.02 | NTSC-J | `SCPS_150.37` | [![](https://bytematch.dev/OpenRAC/rac1-decomp.svg?mode=shield&label=Code&measure=matched_code_percent&version=SCPS_150.37)](https://bytematch.dev/OpenRAC/rac1-decomp/SCPS_150.37) | [![](https://bytematch.dev/OpenRAC/rac1-decomp.svg?mode=shield&label=Functions&measure=matched_functions&version=SCPS_150.37)](https://bytematch.dev/OpenRAC/rac1-decomp/SCPS_150.37) |
| v1.00 | NTSC-U (the NTSC decomp's own count, Lombyte's) | `SCUS_971.99` | [![](https://bytematch.dev/OpenRAC/rac1-decomp.svg?mode=shield&label=Code&measure=matched_code_percent&version=SCUS_971.99)](https://bytematch.dev/OpenRAC/rac1-decomp/SCUS_971.99) | [![](https://bytematch.dev/OpenRAC/rac1-decomp.svg?mode=shield&label=Functions&measure=matched_functions&version=SCUS_971.99)](https://bytematch.dev/OpenRAC/rac1-decomp/SCUS_971.99) |

PAL v2.00 is the primary version; the other releases share its C, with their own definitions only where their bytes differ, and each builds byte-identical from assembly and has its own report (see [`docs/VERSIONS.md`](docs/VERSIONS.md)). NTSC-U is built and checked here like the others (`config/versions/us/`, `progress/us/report.json`); the `SCUS_971.99` figure ByteMatch shows is the NTSC decomp's own count, Lombyte's (Mateusz Kłysz's) decompilation of NTSC-U, which this repository reads at a pinned commit, outside the checkout, for its names and its report (`tools/ntsc_source.py`; see [`docs/VERSIONS.md`](docs/VERSIONS.md)).

Every function links at its original retail address. Functions not yet decompiled are built from disassembly, ensuring the full binary always links and matches retail byte-for-byte outside in-progress functions. For level overlays and breakdown details, see [ByteMatch](https://bytematch.dev/OpenRAC/rac1-decomp) and [`docs/OVERLAYS.md`](docs/OVERLAYS.md).

---

## Quick Start

### Prerequisites
- **Linux & macOS**: [Docker](https://www.docker.com/) or [Podman](https://podman.io/) (uses our prebuilt Wine container via GitHub Container Registry).
- **Windows**: Git Bash, Python 3.10+, and community toolchain mirrors.

### Setup & Build

1. **Clone the repository:**
   ```bash
   git clone https://github.com/OpenRAC/rac1-decomp.git
   cd rac1-decomp
   ```
   *(On Windows, keep the directory path short to avoid path length limits in the legacy toolchain's `make`.)*

2. **Provide your original executable:**
   Copy `SCES_509.16` from your disc image into `baserom/`:
   ```bash
   # Expected SHA-1: 79956931bd62fafd8d20fa2eae796dbaf2e15e83
   cp /path/to/SCES_509.16 baserom/SCES_509.16
   ```

3. **Install dependencies and fetch toolchains:**
   ```bash
   # Windows (native):
   pip install -r requirements.txt
   bash tools/setup_asm.sh
   git clone https://github.com/AngheloAlf/SN-Systems-ProDG_for_PS2_3.01 toolchain/sn-prodg-3.01
   git clone https://github.com/AngheloAlf/sce_ps2_sdk_24 toolchain/sn-prodg-24

   # Linux & macOS (via container wrapper):
   bash tools/docker/run.sh bash tools/setup_asm.sh
   git clone https://github.com/AngheloAlf/SN-Systems-ProDG_for_PS2_3.01 toolchain/sn-prodg-3.01
   git clone https://github.com/AngheloAlf/sce_ps2_sdk_24 toolchain/sn-prodg-24
   ```

4. **Build and verify:**
   ```bash
   # Windows (native):
   bash tools/build_sn.sh

   # Linux & macOS (via container):
   bash tools/docker/run.sh bash tools/build_sn.sh
   ```

For detailed documentation on the toolchain and container setup:
- [`docs/TOOLCHAIN.md`](docs/TOOLCHAIN.md) – Compiler and assembler configurations
- [`docs/BUILD_FIDELITY.md`](docs/BUILD_FIDELITY.md) – What the build reproduces of retail's toolchain, what it models, and the rules that keep it honest
- [`docs/CONTAINERS.md`](docs/CONTAINERS.md) – Docker/Podman container workflow

---

## Contributing

Contributions are warmly welcome! Whether you are interested in decompiling functions, researching engine quirks, or improving documentation:

- See [`CONTRIBUTING.md`](CONTRIBUTING.md) for contribution guidelines and rules.
- See [`docs/WORKFLOW.md`](docs/WORKFLOW.md) for the step-by-step function matching guide.
- Join our **[Discord Community](https://discord.gg/Sfd2B54PDG)** to discuss progress, ask questions, and collaborate!

---

## Credits

This project builds upon years of dedicated reverse-engineering research and tooling by the community:

- **GFI (Game Fuckery Inc.)** – Special thanks to the GFI Discord community for years of reverse engineering, game research, and technical insights that made this decompilation possible.
- **[Lombyte](https://github.com/mateuszklysz/Lombyte)** by mateuszklysz – Matching decompilation of the NTSC build of *Ratchet & Clank*. Invaluable reference for function pairing, struct definitions, and symbol names ([`docs/SIBLING_DECOMPS.md`](docs/SIBLING_DECOMPS.md)).
- **[ReRAC](https://github.com/re-rac/rerac)** by the ReRAC team – Native PC port of the US release. Essential documentation of formats (mobys, models, collision), symbol names ([`config/names.tsv`](config/names.tsv)), and overlay mechanics ([`docs/OVERLAYS.md`](docs/OVERLAYS.md)).
- **[rac3-uya-decomp](https://github.com/OpenRAC/rac3-uya-decomp)** by vetusmagnus – Matching decompilation of *Ratchet & Clank: Up Your Arsenal*. Foundation for SN Systems compiler flag discoveries and build setup.
- **[Wrench](https://github.com/chaoticgd/wrench)** by chaoticgd – Ratchet & Clank PS2 modding tools and asset format specifications ([OpenRAC's `editor/README.md`](../../../editor/README.md)).
- **Decompilation Tooling & Ecosystem**:
  - [splat](https://github.com/ethteck/splat) & [spimdisasm](https://github.com/Decompollaborate/spimdisasm) – Binary splitting and MIPS disassembly.
  - [m2c](https://github.com/matt-kempster/m2c) & [asm-differ](https://github.com/simonlindholm/asm-differ) – Assembly-to-C translation and diffing.
  - [objdiff](https://github.com/encounter/objdiff) – Object diffing tool.
  - [ByteMatch](https://bytematch.dev) – Progress tracking (formerly on decomp.dev).
  - [decomp.wiki](https://decomp.wiki) – Decompilation knowledge base.
  - [AngheloAlf](https://github.com/AngheloAlf) – PS2 toolchain mirrors.

---

## License

- Code written for this project is licensed under the [GNU General Public License v3.0](LICENSE).
- Reconstructed libraries and third-party components retain their original licenses (see [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md) and [`LEGAL.md`](LEGAL.md)).
- *Ratchet & Clank* is a registered trademark of Sony Interactive Entertainment. This project is not affiliated with or endorsed by Sony or Insomniac Games.
