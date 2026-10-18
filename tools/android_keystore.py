#!/usr/bin/env python3
"""
Makes the launcher's Android signing key, once, for a repository's secrets.

  python3 tools/android_keystore.py [OUT.jks]

Android installs an update only over an APK signed with the same key. A CI
run without the ANDROID_KEYSTORE secrets signs with the runner's own debug
key, a new one each run, so a build never installs over an earlier one
("App not installed": uninstall first, losing the set-up). This makes a key
with keytool (a JDK's) and prints the four secrets for the repository's
Settings > Secrets and variables > Actions; launcher.yml signs every APK with
them from then on (launcher/docs/RELEASING.md).

Keep OUT.jks (default: openrac-android.jks here) and the passwords safe and
out of the repository: whoever has them can sign an APK that installs over
the launcher. Nothing here is uploaded anywhere.
"""

import argparse
import base64
import os
import secrets
import shutil
import subprocess
import sys

ALIAS = "openrac"


def keytool_command(out: str, password: str, alias: str = ALIAS) -> list[str]:
    # PKCS12, one password for the store and the key (PKCS12 keeps one), valid 30 years.
    return [
        "keytool", "-genkeypair", "-keystore", out, "-storetype", "PKCS12",
        "-alias", alias, "-keyalg", "RSA", "-keysize", "4096", "-validity", "10950",
        "-storepass", password, "-keypass", password,
        "-dname", "CN=OpenRAC Launcher",
    ]


def secret_values(keystore: bytes, password: str, alias: str = ALIAS) -> dict[str, str]:
    return {
        "ANDROID_KEYSTORE": base64.b64encode(keystore).decode("ascii"),
        "ANDROID_KEYSTORE_PASSWORD": password,
        "ANDROID_KEY_ALIAS": alias,
        "ANDROID_KEY_PASSWORD": password,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("out", nargs="?", default="openrac-android.jks", help="where the keystore goes")
    args = parser.parse_args()
    if os.path.exists(args.out):
        print(f"{args.out} exists already: an existing key is kept, not replaced", file=sys.stderr)
        return 1
    if shutil.which("keytool") is None:
        print("keytool is not on the PATH: it comes with a JDK", file=sys.stderr)
        return 1
    password = secrets.token_urlsafe(24)
    subprocess.run(keytool_command(args.out, password), check=True, stdout=subprocess.DEVNULL)
    with open(args.out, "rb") as f:
        values = secret_values(f.read(), password)
    print(f"Wrote {args.out}. Add these as repository secrets (Settings > Secrets and variables > Actions):\n")
    for name, value in values.items():
        print(f"{name}\n{value}\n")
    print("A launcher already installed with another key has to be uninstalled once; every build after it updates.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
