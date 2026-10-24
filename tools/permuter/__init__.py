# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""decomp-permuter for every game in OpenRAC: one engine, a profile per game, a queue for the boxes.

  upstream  the pinned decomp-permuter checkout, fetched into a cache and patched
  profile   a game's profile: where a function's C and retail assembly are, how to compile and check
  score     aligned instruction diffs against the retail assembly
  engine    the native random search, with live progress
  accept    the permuter artefacts a result must not have
  runner    the queue runner for the build machines (python3 tools/permuter queue ...)
  cli       the command line (python3 tools/permuter --help)
"""
