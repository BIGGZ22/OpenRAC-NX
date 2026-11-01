// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>
namespace openrac {
struct TimingSamples {
    std::vector<double> values;
    void add(double ms) { if(std::isfinite(ms) && ms>=0 && values.size()<3600) values.push_back(ms); }
    double mean() const { return values.empty()?0:std::accumulate(values.begin(),values.end(),0.0)/values.size(); }
    double maximum() const { return values.empty()?0:*std::max_element(values.begin(),values.end()); }
    double p95() const {
        if(values.empty()) return 0;
        auto sorted=values;
        const auto index=(sorted.size()*95+99)/100-1;
        std::nth_element(sorted.begin(),sorted.begin()+index,sorted.end());
        return sorted[index];
    }
    void clear() { values.clear(); }
};
}
