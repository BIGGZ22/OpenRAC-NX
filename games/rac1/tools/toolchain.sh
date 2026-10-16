# Sourced by the build scripts: where the SN toolchain lives and how to run it.
#
# The toolchain is 32-bit Windows programs. On Windows (Git Bash) they run
# directly and the build uses SN's own make.exe. Anywhere else they run
# through Wine and the build uses the host's GNU make, in parallel;
# tools/docker/ provides a ready-made 32-bit Wine environment for that.
# Override either choice with WINE=... or MAKE_SN=... in the environment.
TC=toolchain/sn-prodg-3.01/usr/local/sce/ee/gcc/bin
case "$(uname -s)" in
  MINGW*|MSYS*|CYGWIN*)
    : "${WINE=}"
    : "${MAKE_SN=$TC/make.exe}" ;;
  *)
    if [ -z "${WINE:-}" ]; then
      if command -v wine >/dev/null 2>&1; then
        WINE="wine"
      elif command -v wibo >/dev/null 2>&1; then
        WINE="wibo"
      elif [ -x "$PWD/toolchain/wibo" ]; then
        WINE="$PWD/toolchain/wibo"
      elif [ -x "$PWD/../../toolchains/wibo" ]; then
        WINE="$PWD/../../toolchains/wibo"
      elif [ -x "$HOME/.local/bin/wibo" ]; then
        WINE="$HOME/.local/bin/wibo"
      else
        wibo_bin="$PWD/toolchain/wibo"
        mkdir -p "$(dirname "$wibo_bin")"
        echo "Neither wine nor wibo found; downloading wibo..."
        if [ "$(uname -s)" = "Darwin" ]; then
          wibo_url="https://github.com/decompals/wibo/releases/download/1.2.0/wibo-macos"
        else
          wibo_url="https://github.com/decompals/wibo/releases/download/1.2.0/wibo-i686"
        fi
        if curl -sSL -o "$wibo_bin" "$wibo_url" && chmod +x "$wibo_bin"; then
          WINE="$wibo_bin"
        else
          WINE="wine"
        fi
      fi
    fi
    : "${MAKE_SN=make -j$(nproc 2>/dev/null || echo 4)}" ;;
esac

# Add local virtual environment to PATH if present so python/splat tools are available
if [ -d "$PWD/.venv/bin" ]; then
  export PATH="$PWD/.venv/bin:$PATH"
elif [ -d "$PWD/../../.venv/bin" ]; then
  export PATH="$PWD/../../.venv/bin:$PATH"
fi

# Parallel make starts many Wine processes at once. With no wineserver running
# yet they race to start one and some fail ("wine: chdir to
# /tmp/wine-.../server-...: No such file or directory"), so start it up front
# and keep it alive for a minute between waves. It exits 2 when a server is
# already running, which is fine (and must not trip `set -e` in a caller).
if [ -n "$WINE" ] && [ "$WINE" != "wibo" ] && [[ "$WINE" != *"wibo"* ]] && command -v wineserver >/dev/null 2>&1; then
  wineserver -p60 2>/dev/null || true
fi

# sn PROGRAM.exe ARGS...   run one toolchain program
sn() { $WINE "$@"; }

# make_sn ARGS...          run Makefile.sn with the right make and Wine
make_sn() { $MAKE_SN -f Makefile.sn WINE="$WINE" "$@"; }
