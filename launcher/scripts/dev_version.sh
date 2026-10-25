#!/usr/bin/env bash
# The version a build of a branch carries before its -dev.<run> (.github/workflows/launcher.yml):
# tauri.conf.json's, or, when that version is already released (its tag launcher-v<version> is in
# REPOSITORY or in OpenRAC's own), the next patch that is not. A build of a branch is then always
# newer than the release a launcher may be on, so the launcher can switch to Development and update
# (docs/RELEASING.md). A repository that cannot be asked (a private fork) counts as having no tag.
#
#   scripts/dev_version.sh CONF REPOSITORY      # scripts/dev_version.sh src-tauri/tauri.conf.json someone/OpenRAC
set -euo pipefail

version=$(node -p "require(require('path').resolve(process.argv[1])).version" "$1")

released() {
    local repo
    for repo in "$2" OpenRAC/OpenRAC; do
        git ls-remote --exit-code --tags "https://github.com/$repo" "refs/tags/launcher-v$1" > /dev/null 2>&1 && return 0
    done
    return 1
}

while released "$version" "$2"; do
    IFS=. read -r major minor patch <<< "$version"
    version="$major.$minor.$((patch + 1))"
done
echo "$version"
