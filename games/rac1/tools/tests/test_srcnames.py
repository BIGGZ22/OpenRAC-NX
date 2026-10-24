"""Name normalization must keep the legacy token and line semantics."""
from pathlib import Path
import random
import re
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import srcnames


class CanonTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        header = Path(self.temp.name) / "names.h"
        header.write_text("#define Draw func_00123456\n"
                          "#define DrawLong func_L00_00123456\n"
                          "#define _Draw func_00123458\n"
                          "#define Other unrelated_symbol\n", encoding="utf-8")
        self.patch = patch.object(srcnames, "HEADER", header)
        self.patch.start()
        self.addCleanup(self.patch.stop)
        srcnames._table.cache_clear()
        self.addCleanup(srcnames._table.cache_clear)

    def test_only_complete_known_words_change(self):
        source = "Draw DrawLong _Draw Draw1 1Draw _Draw_ DrawLonger Other"
        self.assertEqual(srcnames.canon(source),
                         "func_00123456 func_L00_00123456 func_00123458 "
                         "Draw1 1Draw _Draw_ DrawLonger Other")

    def test_punctuation_comments_and_lines_keep_legacy_behavior(self):
        source = 'Draw();\n/* DrawLong */\n"_Draw";\r\n'
        result = srcnames.canon(source)
        self.assertEqual(result, 'func_00123456();\n/* func_L00_00123456 */\n'
                                 '"func_00123458";\r\n')
        self.assertEqual(result.count("\n"), source.count("\n"))

    def test_matches_legacy_regex_for_mixed_word_boundaries(self):
        _, names = srcnames._table()
        legacy = re.compile(r"\b(" + "|".join(re.escape(n) for n in
                                              sorted(names, key=len, reverse=True)) + r")\b")
        rng = random.Random(20261023)
        words = ["Draw", "DrawLong", "_Draw", "Draw1", "1Draw", "_Draw_",
                 "DrawLonger", "Other", "Drawé", "éDraw", "123", "0x10"]
        separators = [" ", ";", ".", "\n", "\t", "(", ")", "/", "\\", '"']
        for _ in range(100):
            source = "".join(rng.choice(words) + rng.choice(separators) for _ in range(40))
            expected = legacy.sub(lambda m: names[m.group(1)], source)
            self.assertEqual(srcnames.canon(source), expected)

    def test_missing_header_is_identity(self):
        srcnames.HEADER.unlink()
        srcnames._table.cache_clear()
        self.assertEqual(srcnames.canon("Draw();\n"), "Draw();\n")


if __name__ == "__main__":
    unittest.main()
