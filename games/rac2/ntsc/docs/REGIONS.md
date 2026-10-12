# Regions and versions

The project targets the USA v1.01 release. The other two releases are identified here so
their functions can later be mapped onto this one (as rac1-decomp maps its US build onto PAL).

| Release | Serial (boot) | Disc image SHA-256 | Size |
| --- | --- | --- | ---: |
| USA v1.01 (target) | `SCUS_972.68` | `9db2e33e276133cc283647fa3279b37911955e123d6199d10065547eaa9b1ce5` | 3,828,350,976 |
| USA v2.00 (Greatest Hits) | `SCUS_972.68` | `eda31b4519d16014164090e8a36733af63ef8e458a96d37d6054b65c00ec6e72` | 3,824,615,424 |
| Europe/Australia v1.00 | `SCES_516.07` | `519f9fa78b8e821b6bb7fa501fc10e8b10f378e27a6410f68d0dd8952860b41a` | 3,548,545,024 |

Put a disc in `baserom/` as `SCUS_972.68.iso`, `SCUS_972.68_v2.00.iso` or `SCES_516.07.iso`.
The v2.00 boot has the same name as v1.01's but other bytes; nothing is built from v2.00 or the
PAL release yet.
