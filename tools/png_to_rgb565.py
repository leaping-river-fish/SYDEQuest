#!/usr/bin/env python3
"""Convert PNGs to RGB565 C headers: sheet (grid of frames) or flat full image.

Terrain (e.g. FullTerrainSpriteSheet.png, 64x64, 4x4 tiles of 16px): use **sheet** mode with
four rows and four columns so each 16x16 cell becomes one frame in row-major order::

    python tools/png_to_rgb565.py sheet FullTerrainSpriteSheet.png Pico/assets/terrain_sprite.h terrain_sprite 4 4

That order matches DesktopRenderer (tile id as column/row in a 4x4 texture) and
PicoRenderer::drawTile, which passes the CSV tile id as a **frame index** into drawFrame
(linear frame-major ``uint16_t`` data), not runtime sampling of a flat 64x64 bitmap.

Do not use **flat** mode for terrain if the game indexes tiles 0..15 as separate frames;
flat mode emits one rectangle (frameCount=1) and breaks tile rendering.

Gaps between some terrain columns on device are usually transparent pixels inside specific
tile IDs in the art (edge/cap tiles), not a stride bug in conversion.
"""
import sys
from pathlib import Path
from PIL import Image

COLS = 4
ROWS = 3
# Magenta chroma key; must match PicoRenderer transparent pixel skip (0xF81F)
CHROMA_KEY = 0xF81F
ALPHA_THRESHOLD = 128


def rgb888_to_rgb565(r, g, b):
    """Convert 24-bit RGB to 16-bit RGB565."""
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def rgba_to_rgb565(r, g, b, a):
    """Opaque RGB565, or chroma key when alpha is below threshold."""
    if a < ALPHA_THRESHOLD:
        return CHROMA_KEY
    return rgb888_to_rgb565(r, g, b)


def load_rgba(path):
    """Open image as RGBA (adds opaque alpha for RGB-only sources)."""
    return Image.open(path).convert("RGBA")


def rgba_pixels(img):
    """Yield (r, g, b, a) per pixel, row-major (avoids deprecated getdata)."""
    w, h = img.size
    raw = img.tobytes()
    if img.mode != "RGBA":
        raise ValueError("rgba_pixels expects RGBA")
    for i in range(0, len(raw), 4):
        yield raw[i], raw[i + 1], raw[i + 2], raw[i + 3]


def spritesheet_to_single_header(png_path, output_path, array_name, cols=COLS, rows=ROWS):
    """Split a 4x3 spritesheet into frames and export all frames in one header."""
    img = load_rgba(png_path)
    sheet_width, sheet_height = img.size
    if sheet_width % cols != 0 or sheet_height % rows != 0:
        raise SystemExit(
            f"{png_path}: sheet {sheet_width}x{sheet_height} not divisible by {cols}x{rows}"
        )

    frame_width = sheet_width // cols
    frame_height = sheet_height // rows
    total_frames = cols * rows

    frames_pixels = []
    for row in range(rows):
        for col in range(cols):
            x0 = col * frame_width
            y0 = row * frame_height
            frame = img.crop((x0, y0, x0 + frame_width, y0 + frame_height))
            pixels = [rgba_to_rgb565(r, g, b, a) for r, g, b, a in rgba_pixels(frame)]
            frames_pixels.extend(pixels)

    upper = array_name.upper()
    with open(output_path, "w") as f:
        f.write(f"// Auto-generated from {png_path.name}\n")
        f.write("#pragma once\n#include <cstdint>\n\n")
        f.write(f"constexpr int {upper}_FRAME_WIDTH = {frame_width};\n")
        f.write(f"constexpr int {upper}_FRAME_HEIGHT = {frame_height};\n")
        f.write(f"constexpr int {upper}_FRAME_COUNT = {total_frames};\n\n")
        f.write(
            f"const uint16_t {array_name}[{total_frames}*{frame_width}*{frame_height}] = {{\n"
        )

        for i, color in enumerate(frames_pixels):
            f.write(f"0x{color:04X},")
            if (i + 1) % frame_width == 0:
                f.write("\n")

        f.write("};\n")

    print(
        f"Generated {output_path} ({total_frames} frames, {frame_width}x{frame_height} each)"
    )


def png_to_rgb565_header_flat(png_path, output_path, array_name):
    """Convert PNG to a single RGB565 array (full image, frameCount=1 in renderer)."""
    img = load_rgba(png_path)
    width, height = img.size
    pixels = [rgba_to_rgb565(r, g, b, a) for r, g, b, a in rgba_pixels(img)]

    upper = array_name.upper()
    with open(output_path, "w") as f:
        f.write(f"// Auto-generated from {png_path.name}\n")
        f.write("#pragma once\n")
        f.write("#include <cstdint>\n\n")
        f.write(f"constexpr int {upper}_WIDTH = {width};\n")
        f.write(f"constexpr int {upper}_HEIGHT = {height};\n\n")
        f.write(f"const uint16_t {array_name}[{width * height}] = {{\n")

        for i, rgb565 in enumerate(pixels):
            f.write(f"    0x{rgb565:04X},")
            if (i + 1) % 8 == 0:
                f.write("\n")

        f.write("\n};\n")

    print(f"Generated {output_path} (flat {width}x{height}, {len(pixels)} pixels)")


def main():
    if len(sys.argv) < 2:
        print(
            "Usage:\n"
            "  png_to_rgb565.py sheet <input.png> <output.h> <array_name>\n"
            "  png_to_rgb565.py flat  <input.png> <output.h> <array_name>"
        )
        sys.exit(1)

    mode = sys.argv[1].lower()
    if mode not in ("sheet", "flat"):
        print("First argument must be 'sheet' or 'flat'")
        sys.exit(1)
    if mode == "flat" and len(sys.argv) != 5:
        print(
            "Usage:\n"
            "  png_to_rgb565.py sheet <input.png> <output.h> <array_name> [cols rows]\n"
            "  png_to_rgb565.py flat  <input.png> <output.h> <array_name>"
        )
        sys.exit(1)
    if mode == "sheet" and len(sys.argv) not in (5, 7):
        print(
            "Usage:\n"
            "  png_to_rgb565.py sheet <input.png> <output.h> <array_name> [cols rows]\n"
            "  png_to_rgb565.py flat  <input.png> <output.h> <array_name>"
        )
        sys.exit(1)

    png_path = Path(sys.argv[2])
    output_path = Path(sys.argv[3])
    array_name = sys.argv[4]

    if mode == "sheet":
        cols, rows = COLS, ROWS
        if len(sys.argv) == 7:
            cols = int(sys.argv[5])
            rows = int(sys.argv[6])
        spritesheet_to_single_header(png_path, output_path, array_name, cols=cols, rows=rows)
    else:
        png_to_rgb565_header_flat(png_path, output_path, array_name)


if __name__ == "__main__":
    main()
