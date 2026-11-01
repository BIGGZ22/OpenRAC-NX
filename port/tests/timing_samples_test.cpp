// SPDX-License-Identifier: GPL-3.0-or-later
#include "common/timing_samples.h"
#include <cassert>
#include <limits>
#include <cstdio>
int main() {
    openrac::TimingSamples s;assert(s.mean()==0 && s.p95()==0);
    for(int i=1;i<=100;++i)s.add(i);
    assert(s.mean()==50.5 && s.p95()==95 && s.maximum()==100);
    s.add(-1);s.add(std::numeric_limits<double>::quiet_NaN());assert(s.values.size()==100);
    s.clear();s.add(7);assert(s.p95()==7);s.clear();assert(s.maximum()==0);
    std::puts("PASS: timing averages, nearest-rank p95, empty windows and invalid samples");
}
