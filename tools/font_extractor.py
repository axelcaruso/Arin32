#!/usr/bin/env python3
"""
Arin32 Font Atlas Extractor & Generator

This utility extracts TrueType / OpenType font glyphs (ASCII 32 to 126)
and bakes them into a compressed 2D texture atlas byte array alongside
exact proportional metrics for compile-time embedding in Arin32.

Copyright (c) 2026, Arin32 & ArinOS Contributors
Licensed under the BSD 2-Clause License.
"""

import argparse
import sys
import os
from PIL import Image, ImageDraw, ImageFont


def bake_font_atlas(font_path: str, font_size: int, output_path: str):
    """
    Renders ASCII glyphs 32..126 into a 256x128 luminance texture atlas
    and writes out C++ inline header data.
    """
    if not os.path.exists(font_path):
        print(f"[Error] Font file not found: {font_path}", file=sys.stderr)
        sys.exit(1)

    print(f"Loading font: {font_path} at size {font_size}px...")
    try:
        font = ImageFont.truetype(font_path, font_size)
    except Exception as e:
        print(f"[Error] Failed to load TTF font: {e}", file=sys.stderr)
        sys.exit(1)

    tex_w = 256
    tex_h = 128
    cell_w = 14
    cell_h = 18

    # Create grayscale image canvas
    atlas_img = Image.new("L", (tex_w, tex_h), 0)
    draw = ImageDraw.Draw(atlas_img)

    glyphs = []

    for i in range(32, 127):
        char = chr(i)
        idx = i - 32
        col = idx % 16
        row = idx // 16
        cell_x = col * cell_w
        cell_y = row * cell_h

        bbox = draw.textbbox((0, 0), char, font=font)
        bw = bbox[2] - bbox[0]
        bh = bbox[3] - bbox[1]

        if char == ' ':
            adv = 4.5
        else:
            adv = float(bw + 1)

        draw_x = cell_x + 1
        draw_y = cell_y + 1
        draw.text((draw_x, draw_y), char, fill=255, font=font)

        u0 = float(draw_x) / tex_w
        v0 = float(draw_y) / tex_h
        u1 = float(draw_x + cell_w - 2) / tex_w
        v1 = float(draw_y + cell_h - 2) / tex_h

        glyphs.append({
            "char": char,
            "ascii": i,
            "advance": adv,
            "u0": u0,
            "v0": v0,
            "u1": u1,
            "v1": v1,
            "w": float(cell_w - 2),
            "h": float(cell_h - 2)
        })

    os.makedirs(os.path.dirname(os.path.abspath(output_path)), exist_ok=True)

    with open(output_path, "w", encoding="utf-8") as f:
        f.write("/*\n")
        f.write(" * Arin32 - Modern OpenGL Graphical User Interface Library\n")
        f.write(" *\n")
        f.write(" * Copyright (c) 2026, Arin32 & ArinOS Contributors\n")
        f.write(" * Licensed under BSD 2-Clause License.\n")
        f.write(" *\n")
        f.write(f" * AUTO-GENERATED FILE via tools/font_extractor.py\n")
        f.write(f" * Source Font: {os.path.basename(font_path)} (Size: {font_size}px)\n")
        f.write(" * DO NOT EDIT DIRECTLY.\n")
        f.write(" */\n\n")
        f.write("#pragma once\n#include <cstdint>\n\n")
        f.write("namespace arin::internal {\n\n")
        f.write(f"constexpr int FONT_ATLAS_WIDTH = {tex_w};\n")
        f.write(f"constexpr int FONT_ATLAS_HEIGHT = {tex_h};\n")
        f.write(f"constexpr float FONT_LINE_HEIGHT = 14.0f;\n\n")

        f.write("struct GlyphMetric {\n")
        f.write("    float advance;\n")
        f.write("    float u0, v0, u1, v1;\n")
        f.write("    float width, height;\n")
        f.write("};\n\n")

        f.write("static const GlyphMetric s_glyph_metrics[95] = {\n")
        for g in glyphs:
            f.write(f'    /* {repr(g["char"])} ({g["ascii"]}) */ {{ {g["advance"]}f, {g["u0"]}f, {g["v0"]}f, {g["u1"]}f, {g["v1"]}f, {g["w"]}f, {g["h"]}f }},\n')
        f.write("};\n\n")

        f.write("static const uint8_t s_font_atlas_pixels[] = {\n")
        # Get raw bytes
        raw_bytes = list(atlas_img.tobytes())
        for idx in range(0, len(raw_bytes), 24):
            chunk = raw_bytes[idx:idx + 24]
            f.write("    " + ", ".join(f"{b}" for b in chunk) + ",\n")
        f.write("};\n\n")
        f.write("} // namespace arin::internal\n")

    print(f"Successfully generated font atlas: {output_path}")


def main():
    parser = argparse.ArgumentParser(description="Extract TTF font to Arin32 font atlas header")
    parser.add_argument("--font", default="assets/fonts/OpenSans-SemiBold.ttf", help="Path to TTF font file")
    parser.add_argument("--size", type=int, default=12, help="Font point size (default: 12)")
    parser.add_argument("--output", default="src/font_atlas_data.inl", help="Destination C++ header path")

    args = parser.parse_args()
    bake_font_atlas(args.font, args.size, args.output)


if __name__ == "__main__":
    main()
