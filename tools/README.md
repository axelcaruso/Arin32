# Arin32 Tools & Utilities

This directory contains developer tools and asset preparation utilities for Arin32.

---

## `font_extractor.py`

A Python 3 utility to rasterize TrueType / OpenType font glyphs (printable ASCII 32 to 126) into a compact 2D OpenGL luminance texture atlas (`GL_RED` / `GL_R8`) with proportional glyph advances, bounding boxes, and normalized UV coordinates.

The generated header (`src/font_atlas_data.inl`) embeds the typography directly into the Arin32 static binary, ensuring zero external asset or filesystem dependencies at runtime.

### Requirements
- Python 3.8+
- Pillow (`pip install Pillow`)

### Usage

```bash
# Bake Open Sans SemiBold at 12px (Default)
python3 tools/font_extractor.py

# Bake custom font and size
python3 tools/font_extractor.py --font path/to/MyFont.ttf --size 14 --output src/font_atlas_data.inl
```

### Options
- `--font <path>`: Path to `.ttf` or `.otf` font file (default: `assets/fonts/OpenSans-SemiBold.ttf`)
- `--size <int>`: Point / pixel size to render glyphs at (default: `12`)
- `--output <path>`: Destination C++ inline header (default: `src/font_atlas_data.inl`)
