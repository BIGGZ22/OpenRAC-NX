# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""python3 tools/permuter COMMAND ...: see tools/permuter/README.md."""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from permuter import cli  # noqa: E402

if __name__ == "__main__":
    sys.exit(cli.main())
