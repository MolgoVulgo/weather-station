#!/usr/bin/env python3
import argparse
import io
import struct
import sys
import zlib
from pathlib import Path

try:
    import cairosvg
    from PIL import Image
except Exception as exc:  # pragma: no cover - runtime dependency check
    print("Missing dependency: install cairosvg and pillow.", file=sys.stderr)
    raise


def parse_size(value: str) -> tuple[int, int]:
    if "x" not in value:
        raise argparse.ArgumentTypeError("Size must be WIDTHxHEIGHT, e.g. 128x64.")
    w_str, h_str = value.split("x", 1)
    try:
        w = int(w_str)
        h = int(h_str)
    except ValueError as exc:
        raise argparse.ArgumentTypeError("Size must be WIDTHxHEIGHT.") from exc
    if w <= 0 or h <= 0:
        raise argparse.ArgumentTypeError("Size must be positive.")
    return w, h


def render_svg_to_rgb565(svg_path: Path, size: tuple[int, int]) -> bytes:
    svg_bytes = svg_path.read_bytes()
    png_bytes = cairosvg.svg2png(
        bytestring=svg_bytes,
        output_width=size[0],
        output_height=size[1],
    )
    image = Image.open(io.BytesIO(png_bytes)).convert("RGB")

    data = bytearray()
    for r, g, b in image.getdata():
        rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
        data.extend(rgb565.to_bytes(2, byteorder="little"))
    return bytes(data)


def is_svg_content(path: Path) -> bool:
    try:
        head = path.read_bytes()[:2048]
    except OSError:
        return False
    lowered = head.lower()
    return b"<svg" in lowered


def build_merge_entry(
    name: str, size: tuple[int, int], payload: bytes, compressed: bool
) -> bytes:
    name_bytes = name.encode("utf-8")
    header = struct.pack(
        "<H", len(name_bytes)
    ) + name_bytes + struct.pack("<HHIB", size[0], size[1], len(payload), int(compressed))
    return header + payload


def iter_svg_files(input_path: Path) -> list[Path]:
    if input_path.is_file():
        return [input_path]
    return sorted(p for p in input_path.rglob("*.svg") if p.is_file())


def ensure_out_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def write_file(path: Path, data: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Convert SVG to RGB565 .bin with per-image headers."
    )
    parser.add_argument("input", type=Path, help="SVG file or directory of SVGs.")
    parser.add_argument(
        "--size",
        required=True,
        type=parse_size,
        help="Output size as WIDTHxHEIGHT.",
    )
    parser.add_argument(
        "--out",
        required=True,
        type=Path,
        help="Output .bin file (input file) or output directory (input dir).",
    )
    parser.add_argument(
        "--out-dir",
        type=Path,
        help="Optional directory for per-SVG .bin files when input is a directory.",
    )
    parser.add_argument(
        "--compress",
        action="store_true",
        help="Compress payloads with zlib (flag stored per entry).",
    )

    args = parser.parse_args()
    input_path = args.input

    if not input_path.exists():
        print(f"Input not found: {input_path}", file=sys.stderr)
        return 2

    svg_files = iter_svg_files(input_path)
    if not svg_files:
        print("No SVG files found.", file=sys.stderr)
        return 2

    if input_path.is_file():
        if args.out.suffix.lower() != ".bin":
            print("For a single input file, --out must be a .bin file.", file=sys.stderr)
            return 2
        if not is_svg_content(svg_files[0]):
            print(f"WARNING: invalid SVG content: {svg_files[0]}", file=sys.stderr)
            return 2
        data = render_svg_to_rgb565(svg_files[0], args.size)
        payload = zlib.compress(data) if args.compress else data
        entry = build_merge_entry(svg_files[0].stem, args.size, payload, args.compress)
        write_file(args.out, entry)
        return 0

    if args.out_dir:
        ensure_out_dir(args.out_dir)

    merged = bytearray()
    for svg_path in svg_files:
        if not is_svg_content(svg_path):
            print(f"WARNING: invalid SVG content: {svg_path}", file=sys.stderr)
            continue
        data = render_svg_to_rgb565(svg_path, args.size)
        payload = zlib.compress(data) if args.compress else data
        entry = build_merge_entry(svg_path.stem, args.size, payload, args.compress)
        merged.extend(entry)
        if args.out_dir:
            out_file = args.out_dir / (svg_path.stem + ".bin")
            write_file(out_file, entry)

    if args.out.suffix.lower() != ".bin":
        print("For a directory input, --out must be a .bin file.", file=sys.stderr)
        return 2
    write_file(args.out, bytes(merged))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
