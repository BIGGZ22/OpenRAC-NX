"""tools/android_keystore.py: the key it asks keytool for, and the secrets it prints."""

import base64
import os
import shutil
import subprocess
import tempfile
import unittest

from tools import android_keystore


class AndroidKeystoreTest(unittest.TestCase):
    def test_secrets_are_what_the_workflow_reads(self):
        values = android_keystore.secret_values(b"\x00key", "pw")
        self.assertEqual(
            set(values), {"ANDROID_KEYSTORE", "ANDROID_KEYSTORE_PASSWORD", "ANDROID_KEY_ALIAS", "ANDROID_KEY_PASSWORD"}
        )
        self.assertEqual(base64.b64decode(values["ANDROID_KEYSTORE"]), b"\x00key")
        self.assertEqual(values["ANDROID_KEY_ALIAS"], "openrac")
        workflow = os.path.join(os.path.dirname(__file__), "..", ".github", "workflows", "launcher.yml")
        with open(workflow, encoding="utf-8") as f:
            text = f.read()
        for name in values:
            self.assertIn(f"secrets.{name} ", text.replace("}}", " }}"))

    @unittest.skipIf(shutil.which("keytool") is None, "no keytool")
    def test_makes_a_keystore(self):
        with tempfile.TemporaryDirectory() as d:
            out = os.path.join(d, "k.jks")
            subprocess.run(android_keystore.keytool_command(out, "secret-pw"), check=True,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            listed = subprocess.run(["keytool", "-list", "-keystore", out, "-storepass", "secret-pw"],
                                    check=True, capture_output=True, text=True).stdout
            self.assertIn("openrac", listed)


if __name__ == "__main__":
    unittest.main()
