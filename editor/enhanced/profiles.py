"""Copy authored lighting settings into a native level export, shared by bake and runtime."""

from pathlib import Path
import shutil


PROFILE_ROOT = Path(__file__).resolve().parents[2] / "port/enhanced/lighting/profiles"


def write_bake_profile(level, directory: Path) -> None:
    """Copy only this game's, disc version's and level's profile into the new export."""
    source = PROFILE_ROOT / f"{level.game}-{level.version}" / f"level_{level.id:02}" / "probe-bake.json"
    # Unconfigured levels use the baker and runtime's shared built-in defaults.
    if source.is_file():
        shutil.copyfile(source, directory / source.name)
