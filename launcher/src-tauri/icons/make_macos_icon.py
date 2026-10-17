"""Build OpenRAC's macOS icons in Apple's shape.

macOS draws app icons as given, so the file must carry Apple's own shape: an
824-pixel rounded square, centred on a 1024-pixel canvas, with a soft shadow
below it. With no arguments this builds the launcher's icon.icns: icon.png (the
full-bleed picture the other systems use) supplies the wrench, which is lifted
off its orange, set on the shaped tile and written in every size iconutil wants.

With --picture it shapes any full-bleed square picture the same way, whole
(scaled to the tile, its corners rounded), and writes a PNG: the native games
set their window icon at run time, which on macOS is the Dock icon, so each
port/game/<id>/icon.png has an icon-macos.png made from it.

    python3 make_macos_icon.py        # icon.icns; needs Pillow, macOS (iconutil)
    python3 make_macos_icon.py --picture ../../../port/game/rac1-pal/icon.png \\
        ../../../port/game/rac1-pal/icon-macos.png   # needs Pillow only
"""

import argparse
import pathlib
import shutil
import subprocess
import tempfile

from PIL import Image, ImageChops, ImageDraw, ImageFilter

HERE = pathlib.Path(__file__).resolve().parent
CANVAS = 1024
TILE = 824  # Apple's icon grid: the tile and its corner radius
RADIUS = 185
ORANGE = (229, 160, 19)
TOP, BOTTOM = (240, 176, 44), (219, 146, 10)
ART = 0.66  # the wrench's share of the tile
SS = 4  # supersampling for smooth edges


def tile_mask() -> Image.Image:
    big = Image.new("L", (TILE * SS, TILE * SS), 0)
    ImageDraw.Draw(big).rounded_rectangle(
        (0, 0, TILE * SS - 1, TILE * SS - 1), radius=RADIUS * SS, fill=255
    )
    return big.resize((TILE, TILE), Image.LANCZOS)


def gradient() -> Image.Image:
    g = Image.new("RGBA", (TILE, TILE))
    px = g.load()
    for y in range(TILE):
        t = y / (TILE - 1)
        c = tuple(round(a + (b - a) * t) for a, b in zip(TOP, BOTTOM)) + (255,)
        for x in range(TILE):
            px[x, y] = c
    return g


def wrench() -> Image.Image:
    """The wrench from icon.png, with its orange made transparent."""
    src = Image.open(HERE / "icon.png").convert("RGBA")
    out = Image.new("RGBA", src.size)
    # Stay clear of the old rounded outline, whose edge is not quite orange.
    inside = src.getchannel("A").point(lambda a: 255 if a == 255 else 0).filter(ImageFilter.MinFilter(13))
    sp, op, ip = src.load(), out.load(), inside.load()
    for y in range(src.height):
        for x in range(src.width):
            r, g, b, a = sp[x, y]
            d = ((r - ORANGE[0]) ** 2 + (g - ORANGE[1]) ** 2 + (b - ORANGE[2]) ** 2) ** 0.5
            k = max(0.0, min(1.0, (d - 18) / 70))
            if k == 0 or not ip[x, y]:
                continue
            # Take the orange back out of the edge pixels so no fringe is left.
            col = [
                max(0, min(255, round((c - o * (1 - k)) / k)))
                for c, o in zip((r, g, b), ORANGE)
            ]
            op[x, y] = (*col, round(255 * k))
    return out.crop(out.getbbox())


def on_canvas(tile: Image.Image) -> Image.Image:
    """A TILE-sized picture as Apple's icon: corners rounded, centred on the canvas, shadowed."""
    tile = tile.convert("RGBA")
    tile.putalpha(ImageChops.multiply(tile.getchannel("A"), tile_mask()))
    canvas = Image.new("RGBA", (CANVAS, CANVAS), (0, 0, 0, 0))
    off = (CANVAS - TILE) // 2
    drop = Image.new("RGBA", (CANVAS, CANVAS), (0, 0, 0, 0))
    drop_a = Image.new("L", (CANVAS, CANVAS), 0)
    drop_a.paste(tile_mask().point(lambda a: a * 0.32), (off, off + 12))
    drop.putalpha(drop_a.filter(ImageFilter.GaussianBlur(14)))
    canvas.alpha_composite(drop)
    canvas.alpha_composite(tile, (off, off))
    return canvas


def picture(src: pathlib.Path) -> Image.Image:
    """A full-bleed square picture, whole, in Apple's shape."""
    return on_canvas(Image.open(src).convert("RGBA").resize((TILE, TILE), Image.LANCZOS))


def build() -> Image.Image:
    tile = gradient()
    art = wrench()
    side = round(TILE * ART)
    scale = side / max(art.size)
    art = art.resize((round(art.width * scale), round(art.height * scale)), Image.LANCZOS)
    art = art.filter(ImageFilter.UnsharpMask(radius=2, percent=60, threshold=2))
    # A faint shadow under the wrench lifts it off the tile.
    at = ((TILE - art.width) // 2, (TILE - art.height) // 2)
    shade_a = Image.new("L", (TILE, TILE), 0)
    shade_a.paste(art.getchannel("A").point(lambda a: a * 0.28), (at[0], at[1] + 10))
    shade = Image.new("RGBA", (TILE, TILE), (0, 0, 0, 0))
    shade.putalpha(shade_a.filter(ImageFilter.GaussianBlur(10)))
    tile.alpha_composite(shade)
    tile.alpha_composite(art, at)
    return on_canvas(tile)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--picture", nargs=2, metavar=("SRC", "OUT"),
                        help="shape a full-bleed square picture and write it as a PNG")
    parser.add_argument("--size", type=int, default=512, help="the PNG's side (default 512)")
    args = parser.parse_args()
    if args.picture:
        src, out = map(pathlib.Path, args.picture)
        picture(src).resize((args.size, args.size), Image.LANCZOS).save(out, optimize=True)
        print("wrote", out)
        return
    icon = build()
    with tempfile.TemporaryDirectory() as tmp:
        iconset = pathlib.Path(tmp) / "icon.iconset"
        iconset.mkdir()
        for size in (16, 32, 128, 256, 512):
            for mult, suffix in ((1, ""), (2, "@2x")):
                px = size * mult
                icon.resize((px, px), Image.LANCZOS).save(iconset / f"icon_{size}x{size}{suffix}.png")
        subprocess.run(["iconutil", "-c", "icns", str(iconset), "-o", str(HERE / "icon.icns")], check=True)
    print("wrote", HERE / "icon.icns")


if __name__ == "__main__":
    main()
