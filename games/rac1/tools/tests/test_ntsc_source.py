"""tools/ntsc_source.py: the pinned NTSC decomp is fetched once into the cache, shallow, at the pin."""
from pathlib import Path
import os
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import ntsc_source


def git(*args, cwd):
    return subprocess.run(["git", "-c", "user.name=t", "-c", "user.email=t@t", *args], cwd=cwd, check=True,
                          capture_output=True, text=True).stdout.strip()


class FetchTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        t = Path(self.temp.name)
        # an upstream with two commits; the pin is the first, so the fetch must not take HEAD
        self.up = t / "upstream"
        self.up.mkdir()
        git("init", "-q", "-b", "main", cwd=self.up)
        git("config", "uploadpack.allowAnySHA1InWant", "true", cwd=self.up)
        (self.up / "a.txt").write_text("one\n")
        git("add", "a.txt", cwd=self.up)
        git("commit", "-q", "-m", "one", cwd=self.up)
        self.first = git("rev-parse", "HEAD", cwd=self.up)
        (self.up / "a.txt").write_text("two\n")
        git("commit", "-q", "-am", "two", cwd=self.up)
        self.second = git("rev-parse", "HEAD", cwd=self.up)
        self.pin = t / "ntsc_decomp.txt"
        self.pin.write_text(f"# a comment\nfile://{self.up} {self.first}\n")
        self.cache = t / "cache"
        for p in (patch.object(ntsc_source, "PIN", self.pin),
                  patch.dict(os.environ, {"RAC1_NTSC_DECOMP": str(self.cache)})):
            p.start()
            self.addCleanup(p.stop)
        os.environ.pop("RAC1_NTSC_DECOMP_DIR", None)

    def test_fetches_the_pin_once(self):
        d = ntsc_source.path()
        self.assertEqual(d, self.cache / self.first)
        self.assertEqual((d / "a.txt").read_text(), "one\n")
        self.assertEqual(git("rev-list", "--count", "HEAD", cwd=d), "1")   # shallow
        self.assertEqual(git("status", "--porcelain", cwd=d), "")
        (d / "a.txt").write_text("kept\n")
        self.assertEqual(ntsc_source.path(), d)            # no refetch
        self.assertEqual((d / "a.txt").read_text(), "kept\n")
        self.assertEqual(ntsc_source.revision(), self.first)

    def test_resolve_and_write_pin(self):
        url = f"file://{self.up}"
        self.assertEqual(ntsc_source.resolve(url, None), self.second)
        self.assertEqual(ntsc_source.resolve(url, "main"), self.second)
        ntsc_source.write_pin(url, self.second)
        self.assertEqual(ntsc_source.pin(), (url, self.second))
        self.assertTrue(self.pin.read_text().startswith("# a comment\n"))
        self.assertEqual((ntsc_source.path() / "a.txt").read_text(), "two\n")

    def test_override_dir(self):
        with patch.dict(os.environ, {"RAC1_NTSC_DECOMP_DIR": str(self.up)}):
            self.assertEqual(ntsc_source.path(), self.up.resolve())
            self.assertEqual(ntsc_source.revision(), self.second)
        self.assertFalse(self.cache.exists())


if __name__ == "__main__":
    unittest.main()
