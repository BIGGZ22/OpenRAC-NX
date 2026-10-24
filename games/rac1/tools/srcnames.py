"""Source text with readable function names turned back into address names.

C may define a function by its readable name (`void FadeToBlack(void)`);
include/names.h makes that a macro for the address name (`func_001F4E08`), so
the object is the same. Tools that find definitions by their address name
read source through read_c() so they see `void func_001F4E08(void)` either way.
Line structure is kept: every replacement stays on its line.
"""
import re
from functools import lru_cache
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
HEADER = ROOT / "include/names.h"


@lru_cache(maxsize=1)
def _table():
    text = HEADER.read_text() if HEADER.exists() else ""
    names = dict(re.findall(r"(?m)^#define\s+(\w+)\s+(func_(?:L\d\d_)?[0-9A-Fa-f]{8})\s*$", text))
    if not names:
        return None, names
    # All keys above are whole \w+ tokens. Scanning tokens once avoids trying
    # thousands of alternatives at every word in every source file.
    token = re.compile(r"\b\w+\b")
    return token, names


def canon(text: str) -> str:
    """TEXT with every readable function name replaced by its address name."""
    token, names = _table()
    if token is None:
        return text
    return token.sub(lambda m: names.get(m.group(0), m.group(0)), text)


def read_c(path, **kw) -> str:
    """PATH's text, as canon() gives it."""
    return canon(Path(path).read_text(**kw))
