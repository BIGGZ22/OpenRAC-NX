// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Independent pieces of one job run on a few threads kept for it: the mobys' poses of a tick, each
// a pure function of the game's memory, which on a phone's slow cores cost milliseconds one after
// another. The calling thread works too and waits for the rest; the results are the same as one
// thread's, only sooner. A machine with two threads or fewer runs everything on the caller.

#pragma once

#include <cstddef>
#include <functional>

namespace openrac::parallel {

// Runs body(i) for each i in [0, count), on the calling thread and the pool's threads, in no set
// order, and returns when every one has returned. The first exception one threw is thrown here
// once all have finished. Any thread may call it: while one call has the pool, another (from a
// second thread, or from inside a body) runs its pieces on its own caller.
void for_each(std::size_t count, const std::function<void(std::size_t)>& body);

// The threads beside the caller: up to three, two fewer than the machine has (the game's and the
// render thread's own), 0 on two threads or fewer.
std::size_t workers();

}  // namespace openrac::parallel
