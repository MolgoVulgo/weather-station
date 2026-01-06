#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
Usage: rename-weather-icons.sh [options]

Renomme/copie des icones meteo (SVG) en suivant Tools/SVG2BIN/weather-icon.json.

Options:
  -s, --src DIR      Repertoire source des SVG (defaut: Tools/SVG2BIN/SVG)
  -d, --dest DIR     Repertoire de destination (defaut: Tools/SVG2BIN/SVG_RENAMED)
  -j, --json PATH    Fichier JSON de mapping (defaut: Tools/SVG2BIN/weather-icon.json)
  -n, --dry-run      Affiche les actions sans copier
  -h, --help         Affiche cette aide
USAGE
}

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
src_dir="${script_dir}/SVG"
dest_dir="${script_dir}/SVG_RENAMED"
json_path="${script_dir}/weather-icon.json"
dry_run=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    -s|--src)
      src_dir="$2"
      shift 2
      ;;
    -d|--dest)
      dest_dir="$2"
      shift 2
      ;;
    -j|--json)
      json_path="$2"
      shift 2
      ;;
    -n|--dry-run)
      dry_run=1
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Option inconnue: $1" >&2
      usage >&2
      exit 1
      ;;
  esac
done

if [[ ! -f "$json_path" ]]; then
  echo "JSON introuvable: $json_path" >&2
  exit 1
fi

if [[ ! -d "$src_dir" ]]; then
  echo "Repertoire source introuvable: $src_dir" >&2
  exit 1
fi

if [[ "$dry_run" -eq 0 ]]; then
  mkdir -p "$dest_dir"
fi

python3 - "$json_path" <<'PY' | while IFS=$'\t' read -r src_name dest_name; do
import json
import sys

with open(sys.argv[1], "r", encoding="utf-8") as f:
    data = json.load(f)

aliases = data.get("aliases", {})
for src, dest in sorted(aliases.items()):
    print(f"{src}\t{dest}")
PY
  src_path="${src_dir}/${src_name}.svg"
  dest_path="${dest_dir}/${dest_name}.svg"

  if [[ ! -f "$src_path" ]]; then
    echo "Manquant: $src_path" >&2
    continue
  fi

  if [[ "$dry_run" -eq 1 ]]; then
    echo "cp \"$src_path\" \"$dest_path\""
  else
    cp "$src_path" "$dest_path"
  fi
done
