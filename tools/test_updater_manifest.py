import os
import tempfile
import unittest

from tools.updater_manifest import generate_manifest


class TestUpdaterManifest(unittest.TestCase):
    def test_generate_manifest(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            # Create mock signatures
            win_sig = os.path.join(tmpdir, "openrac-launcher_0.5.0_x64-setup.nsis.zip.sig")
            with open(win_sig, "w", encoding="utf-8") as f:
                f.write("mock_win_sig\n")

            # File with spaces to verify GitHub asset name sanitization (spaces -> dots)
            linux_sig = os.path.join(tmpdir, "OpenRAC Launcher_0.5.0_amd64.AppImage.tar.gz.sig")
            with open(linux_sig, "w", encoding="utf-8") as f:
                f.write("mock_linux_sig\n")

            linux_arm_sig = os.path.join(tmpdir, "OpenRAC Launcher_0.5.0_aarch64.AppImage.tar.gz.sig")
            with open(linux_arm_sig, "w", encoding="utf-8") as f:
                f.write("mock_linux_arm_sig\n")

            manifest = generate_manifest(tmpdir, "launcher-v0.5.0", pub_date="2026-10-16T12:00:00Z")
            self.assertEqual(manifest["version"], "0.5.0")
            self.assertEqual(manifest["pub_date"], "2026-10-16T12:00:00Z")
            self.assertIn("windows-x86_64-nsis", manifest["platforms"])
            self.assertEqual(manifest["platforms"]["windows-x86_64-nsis"]["signature"], "mock_win_sig")
            self.assertIn("linux-x86_64-appimage", manifest["platforms"])
            self.assertEqual(manifest["platforms"]["linux-x86_64-appimage"]["signature"], "mock_linux_sig")
            self.assertTrue("OpenRAC.Launcher" in manifest["platforms"]["linux-x86_64-appimage"]["url"])
            self.assertIn("linux-aarch64-appimage", manifest["platforms"])
            self.assertEqual(manifest["platforms"]["linux-aarch64-appimage"]["signature"], "mock_linux_arm_sig")
            self.assertTrue("OpenRAC.Launcher" in manifest["platforms"]["linux-aarch64-appimage"]["url"])


if __name__ == "__main__":
    unittest.main()
