// SPDX-License-Identifier: GPL-3.0-or-later
// scene/terrain_batch.h: the batch draws exactly its members' triangles, grouped by material,
// each material's pieces in the order given, and refuses (empty) rather than drop a piece.
#include "scene/terrain_batch.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <map>
#include <vector>
using namespace openrac::scene::terrain_batch;
int main() {
    // A shared index list: three "groups" whose primitives are ranges in it.
    std::vector<std::uint32_t> source;
    for (std::uint32_t i = 0; i < 60; ++i) source.push_back(1000 + i);
    // group A: material 2 [0,6), material 1 [6,12); group B: material 1 [12,18); group C: material 2 [30,39), material 3 [39,42)
    const std::vector<Piece> pieces{{2, 0, 6}, {1, 6, 6}, {1, 12, 6}, {2, 30, 9}, {3, 39, 3}};
    std::vector<std::uint32_t> indices;
    std::vector<Run> runs;
    assert(build(pieces, source, indices, runs));
    assert(runs.size() == 3);
    assert(runs[0].material == 1 && runs[0].first == 0 && runs[0].count == 12);
    assert(runs[1].material == 2 && runs[1].first == 12 && runs[1].count == 15);
    assert(runs[2].material == 3 && runs[2].first == 27 && runs[2].count == 3);
    // Material 1: A's piece then B's (order given kept); material 2: A's then C's.
    const std::vector<std::uint32_t> want{1006, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017,
                                          1000, 1001, 1002, 1003, 1004, 1005, 1030, 1031, 1032, 1033, 1034, 1035, 1036, 1037, 1038,
                                          1039, 1040, 1041};
    assert(indices == want);
    // Same triangles per material as the members drew separately (multiset of index triples).
    std::map<int, std::vector<std::vector<std::uint32_t>>> separate, batched;
    for (const Piece& p : pieces)
        for (std::uint32_t t = p.first; t < p.first + p.count; t += 3)
            separate[p.material].push_back({source[t], source[t + 1], source[t + 2]});
    for (const Run& r : runs)
        for (std::uint32_t t = r.first; t < r.first + r.count; t += 3)
            batched[r.material].push_back({indices[t], indices[t + 1], indices[t + 2]});
    for (auto* m : {&separate, &batched})
        for (auto& [k, v] : *m) std::sort(v.begin(), v.end());
    assert(separate == batched);
    // Empty pieces are skipped; an out-of-range piece refuses the whole batch.
    assert(build({{4, 3, 0}, {4, 0, 3}}, source, indices, runs) && runs.size() == 1 && runs[0].count == 3);
    assert(!build({{1, 0, 3}, {1, 58, 3}}, source, indices, runs) && indices.empty() && runs.empty());
    assert(build({}, source, indices, runs) && indices.empty() && runs.empty());
    std::puts("PASS: terrain batch keeps every member triangle, one run a material, order kept, refuses out-of-range pieces");
}
