# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Tests for tools/build_android.py's pure steps: where it finds the NDK, and how it downloads and
checks Python for Android. A fake network and no build."""
import contextlib
import hashlib
import io
import os
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import build_android  # noqa: E402

NDK_VARIABLES = ("ANDROID_NDK_HOME", "NDK_HOME", "ANDROID_NDK_ROOT", "ANDROID_NDK_LATEST_HOME",
                 "ANDROID_HOME", "ANDROID_SDK_ROOT")


class Response(io.BytesIO):
    """What urllib.request.urlopen gives, as a context manager."""

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        return False


class BuildAndroidTests(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.dir = Path(tmp.name)

    def make_ndk(self, path: Path) -> Path:
        (path / "build/cmake").mkdir(parents=True)
        (path / "build/cmake/android.toolchain.cmake").write_text("")
        return path

    def test_ndk_home_from_its_variables_in_order(self):
        first, second = self.make_ndk(self.dir / "one"), self.make_ndk(self.dir / "two")
        env = {name: "" for name in NDK_VARIABLES}
        with mock.patch.dict(os.environ, dict(env, NDK_HOME=str(second), ANDROID_NDK_ROOT=str(first))):
            self.assertEqual(build_android.ndk_home(), second)
        # A variable naming no NDK is passed over.
        with mock.patch.dict(os.environ, dict(env, ANDROID_NDK_HOME=str(self.dir), NDK_HOME=str(first))):
            self.assertEqual(build_android.ndk_home(), first)

    def test_ndk_home_from_the_sdk_takes_the_newest(self):
        for version in ("26.1", "27.0"):
            self.make_ndk(self.dir / "sdk/ndk" / version)
        env = dict({name: "" for name in NDK_VARIABLES}, ANDROID_SDK_ROOT=str(self.dir / "sdk"))
        with mock.patch.dict(os.environ, env):
            self.assertEqual(build_android.ndk_home(), self.dir / "sdk/ndk/27.0")

    def test_no_ndk_stops(self):
        with mock.patch.dict(os.environ, {name: "" for name in NDK_VARIABLES}), self.assertRaises(SystemExit) as stop:
            build_android.ndk_home()
        self.assertIn("ANDROID_NDK_HOME", str(stop.exception))

    def download(self, data: bytes, sha256: str) -> tuple[Path, mock.Mock]:
        urlopen = mock.Mock(return_value=Response(data))
        with mock.patch.object(build_android, "BUILD", self.dir), mock.patch("urllib.request.urlopen", urlopen), \
                contextlib.redirect_stdout(io.StringIO()):
            return build_android.download("stdlib", sha256), urlopen

    def test_download_fetches_checks_and_keeps(self):
        data = b"python for android"
        good = hashlib.sha256(data).hexdigest()
        dest, urlopen = self.download(data, good)
        name = f"target-{build_android.PYTHON_PACKAGE}-stdlib.zip"
        self.assertEqual(dest, self.dir / "downloads" / name)
        self.assertEqual(dest.read_bytes(), data)
        urlopen.assert_called_once_with(f"{build_android.PYTHON_MAVEN}/{build_android.PYTHON_PACKAGE}/{name}")
        # Kept: the second time nothing is fetched.
        _, urlopen = self.download(b"", good)
        urlopen.assert_not_called()

    def test_download_refetches_a_damaged_copy_and_refuses_a_wrong_one(self):
        data = b"python for android"
        good = hashlib.sha256(data).hexdigest()
        dest = self.dir / "downloads" / f"target-{build_android.PYTHON_PACKAGE}-stdlib.zip"
        dest.parent.mkdir(parents=True)
        dest.write_bytes(b"damaged")
        self.assertEqual(self.download(data, good)[0].read_bytes(), data)
        dest.write_bytes(b"damaged")
        with self.assertRaises(SystemExit) as stop:
            self.download(b"something else", good)
        self.assertEqual(str(stop.exception),
                         f"{dest.name}: its SHA-256 is {hashlib.sha256(b'something else').hexdigest()}, not {good}")
        # What failed its check is not kept.
        self.assertEqual(dest.read_bytes(), b"damaged")


if __name__ == "__main__":
    unittest.main()
