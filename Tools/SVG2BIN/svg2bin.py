#!/usr/bin/env python3
import argparse
import io
import json
import struct
import sys
import zlib
from pathlib import Path
from typing import Optional, Tuple

INDEX_MAGIC = b"S2BI"
INDEX_VERSION = 1
INDEX_HEADER_SIZE = 8
INDEX_ENTRY_SIZE = 8

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


def load_weather_map(path: Path) -> list[dict]:
    data = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(data, dict):
        raise ValueError("Mapping JSON must be an object of OWM codes.")
    entries: list[dict] = []
    for code_key, payload in data.items():
        try:
            code = int(code_key)
        except (TypeError, ValueError) as exc:
            raise ValueError(f"Invalid OWM code: {code_key}") from exc
        if not isinstance(payload, dict):
            raise ValueError(f"Invalid mapping for code {code_key}.")

        def add_icon(icon_value: Optional[str], variant: str) -> bool:
            if not icon_value:
                return False
            if not isinstance(icon_value, str):
                raise ValueError(f"Invalid icon for code {code_key}.")
            icon_name = Path(icon_value).stem
            entries.append({"code": code, "variant": variant, "icon": icon_name})
            return True

        saw_icon = False
        if "icon_day" in payload or "icon_night" in payload:
            saw_icon |= add_icon(payload.get("icon_day"), "day")
            saw_icon |= add_icon(payload.get("icon_night"), "night")
        if "icon" in payload:
            saw_icon |= add_icon(payload.get("icon"), "neutral")
        if not saw_icon:
            raise ValueError(f"No icon defined for code {code_key}.")
    if not entries:
        raise ValueError("Mapping JSON is empty.")
    return entries


def normalize_weather_entries(entries: list[dict]) -> list[dict]:
    unique: dict[Tuple[int, str], str] = {}
    for entry in entries:
        key = (int(entry["code"]), str(entry["variant"]))
        icon = str(entry["icon"])
        if key in unique and unique[key] != icon:
            raise ValueError(
                f"Duplicate mapping for code {key[0]} variant {key[1]}."
            )
        unique[key] = icon
    order = {"day": 0, "night": 1, "neutral": 2}
    result: list[dict] = []
    for key in sorted(unique.keys(), key=lambda k: (k[0], order.get(k[1], 99))):
        result.append({"code": key[0], "variant": key[1], "icon": unique[key]})
    return result


def build_index_entries(weather_entries: list[dict], asset_entries: dict[str, dict]) -> list[dict]:
    index_entries: list[dict] = []
    for weather_entry in weather_entries:
        icon_name = weather_entry["icon"]
        asset = asset_entries.get(icon_name)
        if not asset:
            raise ValueError(f"Missing asset for icon '{icon_name}'.")
        index_entries.append(
            {
                "name": icon_name,
                "code": weather_entry["code"],
                "variant": weather_entry["variant"],
                "offset": asset["offset"],
                "entry_size": asset["entry_size"],
                "width": asset["width"],
                "height": asset["height"],
                "compressed": asset["compressed"],
                "data_len": asset["data_len"],
            }
        )
    return index_entries


def build_index_blob(index_entries: list[dict], data_start: int) -> tuple[bytes, list[dict]]:
    variant_map = {"day": 0, "night": 1, "neutral": 2}
    blob = bytearray()
    abs_entries: list[dict] = []
    for entry in index_entries:
        variant_name = entry.get("variant")
        variant = variant_map.get(variant_name, 2)
        offset = int(entry["offset"]) + data_start
        blob.extend(struct.pack("<HBBI", int(entry["code"]), int(variant), 0, offset))
        abs_entry = dict(entry)
        abs_entry["offset"] = offset
        abs_entries.append(abs_entry)
    return bytes(blob), abs_entries


def write_index_header(bin_path: Path, entries: list[dict]) -> None:
    header_path = bin_path.parent / "index-icon.h"
    lines = [
        "/* Auto-generated by svg2bin.py. Do not edit. */",
        "#pragma once",
        "",
        "#include <stddef.h>",
        "#include <stdint.h>",
        "",
        f'#define SVG2BIN_INDEX_BIN "{bin_path.name}"',
        "",
        "typedef enum {",
        "    SVG2BIN_VARIANT_DAY = 0,",
        "    SVG2BIN_VARIANT_NIGHT = 1,",
        "    SVG2BIN_VARIANT_NEUTRAL = 2,",
        "} svg2bin_variant_t;",
        "",
        "typedef struct {",
        "    uint16_t code;",
        "    svg2bin_variant_t variant;",
        "    uint32_t offset;",
        "    uint32_t entry_size;",
        "    uint16_t width;",
        "    uint16_t height;",
        "    uint8_t compressed;",
        "    uint32_t data_len;",
        "} svg2bin_index_entry_t;",
        "",
        "static const svg2bin_index_entry_t svg2bin_index[] = {",
    ]
    for entry in entries:
        variant_value = entry.get("variant")
        variant_enum = "SVG2BIN_VARIANT_NEUTRAL"
        if variant_value == "day":
            variant_enum = "SVG2BIN_VARIANT_DAY"
        elif variant_value == "night":
            variant_enum = "SVG2BIN_VARIANT_NIGHT"
        lines.append(
            "    {%d, %s, %d, %d, %d, %d, %d, %d},"
            % (
                entry.get("code", 0),
                variant_enum,
                entry["offset"],
                entry["entry_size"],
                entry["width"],
                entry["height"],
                entry["compressed"],
                entry["data_len"],
            )
        )
    lines.extend(
        [
            "};",
            "",
            "static const size_t svg2bin_index_count =",
            "    sizeof(svg2bin_index) / sizeof(svg2bin_index[0]);",
            "",
        ]
    )
    header_path.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    default_map = Path(__file__).resolve().parent / "weather_icon.json"
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
    parser.add_argument(
        "--index-json",
        type=Path,
        help="Optional JSON file to update with per-entry offsets for merged .bin.",
    )
    parser.add_argument(
        "--map-json",
        type=Path,
        default=default_map,
        help="JSON mapping file for OWM codes (default: weather_icon.json).",
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

    weather_entries: list[dict] = []
    if args.map_json:
        try:
            weather_entries = normalize_weather_entries(load_weather_map(args.map_json))
        except FileNotFoundError:
            print(f"Mapping JSON not found: {args.map_json}", file=sys.stderr)
            return 2
        except (ValueError, json.JSONDecodeError) as exc:
            print(f"Invalid mapping JSON: {args.map_json} ({exc})", file=sys.stderr)
            return 2

    def filter_weather_entries(icon_name: str) -> list[dict]:
        return [entry for entry in weather_entries if entry["icon"] == icon_name]

    def update_index(entries: list[dict]) -> None:
        if not args.index_json:
            return
        index_path = args.index_json
        try:
            data = json.loads(index_path.read_text(encoding="utf-8"))
        except FileNotFoundError:
            data = {}
        except json.JSONDecodeError as exc:
            print(f"Invalid JSON: {index_path} ({exc})", file=sys.stderr)
            raise SystemExit(2)
        cleaned_entries = []
        for entry in entries:
            cleaned_entries.append({k: v for k, v in entry.items() if k != "name"})
        data["bin_index"] = cleaned_entries
        index_path.write_text(
            json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
        )

    entries: list[dict] = []

    if input_path.is_file():
        if args.out.suffix.lower() != ".bin":
            print("For a single input file, --out must be a .bin file.", file=sys.stderr)
            return 2
        if not is_svg_content(svg_files[0]):
            print(f"WARNING: invalid SVG content: {svg_files[0]}", file=sys.stderr)
            return 2
        data = render_svg_to_rgb565(svg_files[0], args.size)
        payload = zlib.compress(data) if args.compress else data
        entry_name = svg_files[0].stem
        if weather_entries:
            weather_entries = filter_weather_entries(entry_name)
            if not weather_entries:
                print(
                    f"Mapping JSON does not reference icon: {entry_name}",
                    file=sys.stderr,
                )
                return 2
        entry = build_merge_entry(entry_name, args.size, payload, args.compress)
        entries.append(
            {
                "name": entry_name,
                "offset": 0,
                "entry_size": len(entry),
                "width": args.size[0],
                "height": args.size[1],
                "compressed": int(args.compress),
                "data_len": len(payload),
            }
        )
        index_entries: list[dict] = []
        if weather_entries:
            index_entries = build_index_entries(weather_entries, {entry_name: entries[0]})
        data_start = INDEX_HEADER_SIZE + INDEX_ENTRY_SIZE * len(index_entries)
        index_blob, index_entries_abs = build_index_blob(index_entries, data_start)
        header = INDEX_MAGIC + struct.pack("<HH", INDEX_VERSION, len(index_entries))
        write_file(args.out, header + index_blob + entry)
        if index_entries_abs:
            update_index(index_entries_abs)
            write_index_header(args.out, index_entries_abs)
        return 0

    if args.out_dir:
        ensure_out_dir(args.out_dir)

    merged = bytearray()
    seen_names: set[str] = set()
    svg_by_name: dict[str, Path] = {}
    if weather_entries:
        for svg_path in svg_files:
            name = svg_path.stem
            if name not in svg_by_name:
                svg_by_name[name] = svg_path
            else:
                print(f"WARNING: duplicate SVG name '{name}', using first.", file=sys.stderr)
        required_icons = {entry["icon"] for entry in weather_entries}
        missing = sorted(required_icons - svg_by_name.keys())
        if missing:
            print(
                f"Mapping JSON references missing SVG(s): {', '.join(missing)}",
                file=sys.stderr,
            )
            return 2
        svg_files = [path for path in svg_files if path.stem in required_icons]
    for svg_path in svg_files:
        if not is_svg_content(svg_path):
            print(f"WARNING: invalid SVG content: {svg_path}", file=sys.stderr)
            continue
        entry_name = svg_path.stem
        if entry_name in seen_names:
            print(
                f"WARNING: duplicate mapped name '{entry_name}', skipping {svg_path}",
                file=sys.stderr,
            )
            continue
        seen_names.add(entry_name)
        data = render_svg_to_rgb565(svg_path, args.size)
        payload = zlib.compress(data) if args.compress else data
        entry = build_merge_entry(entry_name, args.size, payload, args.compress)
        entries.append(
            {
                "name": entry_name,
                "offset": len(merged),
                "entry_size": len(entry),
                "width": args.size[0],
                "height": args.size[1],
                "compressed": int(args.compress),
                "data_len": len(payload),
            }
        )
        merged.extend(entry)
        if args.out_dir:
            out_file = args.out_dir / (entry_name + ".bin")
            write_file(out_file, entry)

    if args.out.suffix.lower() != ".bin":
        print("For a directory input, --out must be a .bin file.", file=sys.stderr)
        return 2
    index_entries: list[dict] = []
    if weather_entries:
        entry_map = {entry["name"]: entry for entry in entries}
        index_entries = build_index_entries(weather_entries, entry_map)
    data_start = INDEX_HEADER_SIZE + INDEX_ENTRY_SIZE * len(index_entries)
    index_blob, index_entries_abs = build_index_blob(index_entries, data_start)
    header = INDEX_MAGIC + struct.pack("<HH", INDEX_VERSION, len(index_entries))
    write_file(args.out, header + index_blob + bytes(merged))
    if index_entries_abs:
        update_index(index_entries_abs)
        write_index_header(args.out, index_entries_abs)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
