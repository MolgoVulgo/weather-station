#!/usr/bin/env python3
import io
import json
from pathlib import Path
import struct
import tkinter as tk
from tkinter import filedialog, messagebox, ttk
import xml.etree.ElementTree as ET

try:
    from PIL import Image, ImageTk
except Exception:
    Image = None
    ImageTk = None

try:
    import cairosvg
except Exception:
    cairosvg = None


class Svg2BinGui:
    def __init__(self, root: tk.Tk) -> None:
        self.root = root
        self.root.title("svg2bin GUI")
        self.app_dir = Path(__file__).resolve().parent
        self.config_path = self.app_dir / ".svg2bin_gui.json"
        self.data: dict[str, dict] = {}
        self.current_path: Path | None = None
        self.dirty = False
        self._updating_ui = False
        self._preview_refs: dict[str, ImageTk.PhotoImage] = {}
        self._svg_info_cache: dict[str, dict] = {}

        self.config = self._load_config()
        self._build_ui()
        self._load_last_or_skeleton()

    def _load_config(self) -> dict:
        default_dir = self.app_dir / "SVG"
        cfg = {
            "icons_dir": str(default_dir),
            "preview_size": 128,
            "last_json": "",
        }
        try:
            if self.config_path.exists():
                cfg.update(json.loads(self.config_path.read_text(encoding="utf-8")))
        except Exception:
            pass
        return cfg

    def _save_config(self) -> None:
        self.config_path.write_text(
            json.dumps(self.config, indent=2, ensure_ascii=True) + "\n",
            encoding="utf-8",
        )

    def _build_ui(self) -> None:
        self._build_menu()

        self.root.geometry("980x600")
        self.root.protocol("WM_DELETE_WINDOW", self._on_close)

        main = ttk.Frame(self.root, padding=8)
        main.pack(fill=tk.BOTH, expand=True)

        paned = ttk.PanedWindow(main, orient=tk.HORIZONTAL)
        paned.pack(fill=tk.BOTH, expand=True)

        left = ttk.Frame(paned)
        right = ttk.Frame(paned)
        paned.add(left, weight=1)
        paned.add(right, weight=3)

        self._build_left_panel(left)
        self._build_right_panel(right)

    def _build_menu(self) -> None:
        menu = tk.Menu(self.root)
        file_menu = tk.Menu(menu, tearoff=0)
        file_menu.add_command(label="New (weather_icon_skeleton)", command=self._new_from_skeleton)
        file_menu.add_command(label="Open...", command=self._open_file)
        file_menu.add_separator()
        file_menu.add_command(label="Save", command=self._save)
        file_menu.add_command(label="Save As...", command=self._save_as)
        file_menu.add_separator()
        file_menu.add_command(label="Generer BIN...", command=self._open_generate_dialog)
        file_menu.add_separator()
        file_menu.add_command(label="Settings", command=self._open_settings)
        file_menu.add_separator()
        file_menu.add_command(label="Exit", command=self._on_close)
        menu.add_cascade(label="File", menu=file_menu)
        self.root.config(menu=menu)

    def _build_left_panel(self, parent: ttk.Frame) -> None:
        ttk.Label(parent, text="Codes").pack(anchor=tk.W)
        frame = ttk.Frame(parent)
        frame.pack(fill=tk.BOTH, expand=True)

        self.listbox = tk.Listbox(frame, exportselection=False)
        scrollbar = ttk.Scrollbar(frame, orient=tk.VERTICAL, command=self.listbox.yview)
        self.listbox.configure(yscrollcommand=scrollbar.set)

        self.listbox.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)

        self.listbox.bind("<<ListboxSelect>>", self._on_select)

    def _build_right_panel(self, parent: ttk.Frame) -> None:
        self.code_var = tk.StringVar()
        self.name_var = tk.StringVar()
        self.use_day_for_night = tk.BooleanVar()
        self.day_icon_var = tk.StringVar()
        self.night_icon_var = tk.StringVar()
        self.day_frame_var = tk.IntVar(value=0)
        self.night_frame_var = tk.IntVar(value=0)

        header = ttk.Frame(parent)
        header.pack(fill=tk.X)
        ttk.Label(header, text="Code:").grid(row=0, column=0, sticky=tk.W)
        ttk.Label(header, textvariable=self.code_var).grid(row=0, column=1, sticky=tk.W)
        ttk.Label(header, text="Nom:").grid(row=1, column=0, sticky=tk.W)
        ttk.Label(header, textvariable=self.name_var).grid(row=1, column=1, sticky=tk.W)

        self.use_day_check = ttk.Checkbutton(
            parent,
            text="Utiliser l'image jour pour la nuit",
            variable=self.use_day_for_night,
            command=self._toggle_use_day_for_night,
        )
        self.use_day_check.pack(anchor=tk.W, pady=6)

        ttk.Separator(parent, orient=tk.HORIZONTAL).pack(fill=tk.X, pady=8)

        panels = ttk.Frame(parent)
        panels.pack(fill=tk.BOTH, expand=True)
        panels.columnconfigure(0, weight=1)
        panels.columnconfigure(1, weight=1)

        self.day_panel = self._build_icon_panel(
            panels, "Jour", self.day_icon_var, self.day_frame_var, "day", 0, self._choose_day_icon
        )
        self.night_panel = self._build_icon_panel(
            panels, "Nuit", self.night_icon_var, self.night_frame_var, "night", 1, self._choose_night_icon
        )

    def _build_icon_panel(
        self,
        parent: ttk.Frame,
        title: str,
        icon_var: tk.StringVar,
        frame_var: tk.IntVar,
        variant: str,
        column: int,
        choose_cb,
    ) -> ttk.Frame:
        panel = ttk.LabelFrame(parent, text=title, padding=6)
        panel.grid(row=0, column=column, sticky=tk.NSEW, padx=4)
        panel.columnconfigure(0, weight=1)

        img_label = ttk.Label(panel, text="Apercu indisponible", anchor=tk.CENTER)
        img_label.pack(fill=tk.BOTH, expand=True)
        path_label = ttk.Label(panel, textvariable=icon_var)
        path_label.pack(anchor=tk.W, pady=4)

        btn = ttk.Button(panel, text="Choisir image", command=choose_cb)
        btn.pack(anchor=tk.W)

        frame_row = ttk.Frame(panel)
        frame_label = ttk.Label(frame_row, text="Frame (SVG):")
        frame_label.pack(side=tk.LEFT)
        frame_scale = tk.Scale(
            frame_row,
            from_=0,
            to=0,
            orient=tk.HORIZONTAL,
            showvalue=True,
            length=180,
            variable=frame_var,
            command=lambda val, v=variant: self._on_frame_slider(v, val),
        )
        frame_scale.pack(side=tk.LEFT, padx=4, fill=tk.X, expand=True)

        panel._img_label = img_label
        panel._path_label = path_label
        panel._button = btn
        panel._frame_row = frame_row
        panel._frame_scale = frame_scale
        panel._frame_label = frame_label
        panel._frame_visible = False
        panel._frame_count = 0
        panel._variant = variant
        return panel

    def _set_dirty(self, value: bool = True) -> None:
        self.dirty = value
        self._update_title()

    def _update_title(self) -> None:
        name = self.current_path.name if self.current_path else "Untitled"
        suffix = "*" if self.dirty else ""
        self.root.title(f"svg2bin GUI - {name}{suffix}")

    def _new_from_skeleton(self) -> None:
        skeleton = self.app_dir / "weather_icon_skeleton.json"
        if not skeleton.exists():
            messagebox.showerror("Erreur", f"Fichier manquant: {skeleton}")
            return
        self._load_json(skeleton, mark_dirty=False, set_path=False)

    def _load_last_or_skeleton(self) -> None:
        last_path = self.config.get("last_json", "")
        if last_path:
            path = Path(last_path)
            if path.exists():
                self._load_json(path, mark_dirty=False, set_path=True)
                return
        self._new_from_skeleton()

    def _open_file(self) -> None:
        path = filedialog.askopenfilename(
            title="Ouvrir JSON",
            filetypes=[("JSON", "*.json"), ("Tous les fichiers", "*.*")],
            initialdir=str(self.app_dir),
        )
        if not path:
            return
        self._load_json(Path(path), mark_dirty=False, set_path=True)

    def _save(self) -> None:
        if not self.current_path:
            self._save_as()
            return
        self._write_json(self.current_path)

    def _save_as(self) -> None:
        path = filedialog.asksaveasfilename(
            title="Enregistrer JSON",
            defaultextension=".json",
            filetypes=[("JSON", "*.json")],
            initialdir=str(self.app_dir),
        )
        if not path:
            return
        self.current_path = Path(path)
        self.config["last_json"] = str(self.current_path)
        self._save_config()
        self._write_json(self.current_path)

    def _open_settings(self) -> None:
        win = tk.Toplevel(self.root)
        win.title("Settings")
        win.transient(self.root)
        win.grab_set()

        icons_var = tk.StringVar(value=self.config.get("icons_dir", ""))
        size_var = tk.StringVar(value=str(self.config.get("preview_size", 128)))

        ttk.Label(win, text="Dossier icones:").grid(row=0, column=0, sticky=tk.W, padx=8, pady=6)
        entry = ttk.Entry(win, textvariable=icons_var, width=60)
        entry.grid(row=0, column=1, padx=8, pady=6)

        def choose_dir() -> None:
            path = filedialog.askdirectory(initialdir=icons_var.get() or str(self.app_dir))
            if path:
                icons_var.set(path)

        ttk.Button(win, text="Choisir", command=choose_dir).grid(row=0, column=2, padx=8, pady=6)
        ttk.Label(win, text="Taille apercu:").grid(row=1, column=0, sticky=tk.W, padx=8, pady=6)
        size_entry = ttk.Entry(win, textvariable=size_var, width=10)
        size_entry.grid(row=1, column=1, sticky=tk.W, padx=8, pady=6)

        def apply_size(dynamic: bool = False) -> None:
            try:
                self.config["preview_size"] = min(500, max(16, int(size_var.get())))
            except ValueError:
                if not dynamic:
                    self.config["preview_size"] = 128
            self._refresh_preview()

        def save_cfg() -> None:
            self.config["icons_dir"] = icons_var.get().strip()
            apply_size(dynamic=False)
            self._save_config()
            win.destroy()
            self._refresh_preview()

        def on_size_change(_event=None) -> None:
            apply_size(dynamic=True)

        size_entry.bind("<KeyRelease>", on_size_change)
        size_entry.bind("<FocusOut>", on_size_change)

        ttk.Button(win, text="Enregistrer", command=save_cfg).grid(row=2, column=1, sticky=tk.E, padx=8, pady=8)

    def _open_generate_dialog(self) -> None:
        if not self.data:
            messagebox.showerror("Erreur", "Aucun JSON charge.")
            return

        win = tk.Toplevel(self.root)
        win.title("Generer BIN")
        win.transient(self.root)
        win.grab_set()

        size_w_var = tk.StringVar(value="128")
        size_h_var = tk.StringVar(value="128")
        fmt_var = tk.StringVar(value="RGB565")
        mode_var = tk.StringVar(value="merge")
        out_var = tk.StringVar(value=str(self.app_dir / "icons.bin"))

        ttk.Label(win, text="Taille (px):").grid(row=0, column=0, sticky=tk.W, padx=8, pady=6)
        ttk.Entry(win, textvariable=size_w_var, width=6).grid(row=0, column=1, sticky=tk.W, padx=4, pady=6)
        ttk.Label(win, text="x").grid(row=0, column=2, sticky=tk.W)
        ttk.Entry(win, textvariable=size_h_var, width=6).grid(row=0, column=3, sticky=tk.W, padx=4, pady=6)

        ttk.Label(win, text="Codage:").grid(row=1, column=0, sticky=tk.W, padx=8, pady=6)
        fmt_box = ttk.Combobox(
            win,
            textvariable=fmt_var,
            values=["RGB565", "RGB888", "ARGB8888", "A8", "A8A8"],
            state="readonly",
            width=12,
        )
        fmt_box.grid(row=1, column=1, columnspan=3, sticky=tk.W, padx=4, pady=6)

        ttk.Label(win, text="Mode:").grid(row=2, column=0, sticky=tk.W, padx=8, pady=6)
        ttk.Radiobutton(win, text="1 bin par fichier", variable=mode_var, value="per_file").grid(
            row=2, column=1, columnspan=3, sticky=tk.W, padx=4, pady=6
        )
        ttk.Radiobutton(win, text="Bin merge (toutes les images)", variable=mode_var, value="merge").grid(
            row=3, column=1, columnspan=3, sticky=tk.W, padx=4, pady=2
        )

        out_label = ttk.Label(win, text="Sortie:")
        out_label.grid(row=4, column=0, sticky=tk.W, padx=8, pady=6)
        out_entry = ttk.Entry(win, textvariable=out_var, width=50)
        out_entry.grid(row=4, column=1, columnspan=2, sticky=tk.W, padx=4, pady=6)

        def choose_out() -> None:
            if mode_var.get() == "per_file":
                path = filedialog.askdirectory(initialdir=str(self.app_dir))
                if path:
                    out_var.set(path)
            else:
                path = filedialog.asksaveasfilename(
                    title="Choisir .bin",
                    defaultextension=".bin",
                    filetypes=[("BIN", "*.bin")],
                    initialdir=str(self.app_dir),
                )
                if path:
                    out_var.set(path)

        ttk.Button(win, text="Choisir", command=choose_out).grid(row=4, column=3, padx=8, pady=6)

        def on_mode_change(*_args) -> None:
            if mode_var.get() == "per_file":
                out_label.configure(text="Dossier:")
                out_var.set(str(self.app_dir / "bin_out"))
            else:
                out_label.configure(text="Sortie:")
                out_var.set(str(self.app_dir / "icons.bin"))

        mode_var.trace_add("write", on_mode_change)

        def generate() -> None:
            try:
                size = (int(size_w_var.get()), int(size_h_var.get()))
            except ValueError:
                messagebox.showerror("Erreur", "Taille invalide.")
                return
            if size[0] <= 0 or size[1] <= 0:
                messagebox.showerror("Erreur", "Taille invalide.")
                return

            out_path = Path(out_var.get()).expanduser()
            if mode_var.get() == "per_file":
                if not out_path.exists():
                    try:
                        out_path.mkdir(parents=True, exist_ok=True)
                    except OSError as exc:
                        messagebox.showerror("Erreur", f"Creation dossier impossible: {exc}")
                        return
            else:
                if out_path.suffix.lower() != ".bin":
                    messagebox.showerror("Erreur", "Le fichier de sortie doit etre un .bin.")
                    return

            ok, msg = self._generate_bins(out_path, size, fmt_var.get(), mode_var.get())
            if ok:
                messagebox.showinfo("OK", msg)
                win.destroy()
            else:
                messagebox.showerror("Erreur", msg)

        ttk.Button(win, text="Generer", command=generate).grid(row=5, column=3, sticky=tk.E, padx=8, pady=10)

    def _load_json(self, path: Path, mark_dirty: bool, set_path: bool) -> None:
        try:
            data = json.loads(path.read_text(encoding="utf-8"))
        except Exception as exc:
            messagebox.showerror("Erreur", f"JSON invalide: {exc}")
            return
        if not isinstance(data, dict):
            messagebox.showerror("Erreur", "JSON invalide: objet attendu.")
            return
        self.data = data
        if set_path:
            self.current_path = path
            self.config["last_json"] = str(path)
            self._save_config()
        else:
            self.current_path = None
        self._populate_list()
        self._set_dirty(mark_dirty)

    def _write_json(self, path: Path) -> None:
        out = {}
        for code in self._sorted_codes():
            entry = self.data[code]
            if not isinstance(entry, dict):
                entry = {}
            ordered = self._order_entry(entry)
            out[str(code)] = ordered
        path.write_text(json.dumps(out, indent=2, ensure_ascii=True) + "\n", encoding="utf-8")
        self._set_dirty(False)
        self.current_path = path

    def _order_entry(self, entry: dict) -> dict:
        ordered = {}
        for key in ("name", "icon_day", "icon_night", "frame_day", "frame_night"):
            if key in entry:
                ordered[key] = entry[key]
        for key, value in entry.items():
            if key not in ordered:
                ordered[key] = value
        return ordered

    def _populate_list(self) -> None:
        self.listbox.delete(0, tk.END)
        self._code_index: list[str] = []
        for code in self._sorted_codes():
            entry = self.data.get(code, {})
            name = entry.get("name") if isinstance(entry, dict) else None
            if not name:
                name = "(sans nom)"
            self.listbox.insert(tk.END, f"{code} - {name}")
            self._code_index.append(code)
        if self._code_index:
            self.listbox.selection_set(0)
            self.listbox.event_generate("<<ListboxSelect>>")

    def _on_select(self, _event) -> None:
        if not self.listbox.curselection():
            return
        idx = int(self.listbox.curselection()[0])
        code = self._code_index[idx]
        entry = self.data.get(code, {})
        if not isinstance(entry, dict):
            entry = {}
            self.data[code] = entry
        self._updating_ui = True
        self.code_var.set(code)
        self.name_var.set(entry.get("name", ""))
        self.day_icon_var.set(entry.get("icon_day", ""))
        self.night_icon_var.set(entry.get("icon_night", ""))
        self.day_frame_var.set(self._safe_int(entry.get("frame_day"), 0))
        self.night_frame_var.set(self._safe_int(entry.get("frame_night"), 0))
        use_day = bool(entry.get("icon_day")) and entry.get("icon_day") == entry.get("icon_night")
        self.use_day_for_night.set(use_day)
        self._updating_ui = False
        self._refresh_preview()
        self._toggle_use_day_for_night()

    def _toggle_use_day_for_night(self) -> None:
        if self._updating_ui:
            return
        use_day = self.use_day_for_night.get()
        state = tk.DISABLED if use_day else tk.NORMAL
        self.night_panel._button.configure(state=state)
        self.night_panel._frame_scale.configure(state=state)
        if use_day:
            self._apply_day_to_night()
        self._refresh_preview()

    def _apply_day_to_night(self) -> None:
        code = self.code_var.get()
        if not code:
            return
        entry = self.data.get(code, {})
        entry["icon_night"] = entry.get("icon_day", "")
        if "frame_day" in entry:
            entry["frame_night"] = entry.get("frame_day")
        self.night_icon_var.set(entry.get("icon_night", ""))
        self.night_frame_var.set(self._safe_int(entry.get("frame_night"), 0))
        self._set_dirty(True)

    def _choose_day_icon(self) -> None:
        self._choose_icon("day")

    def _choose_night_icon(self) -> None:
        self._choose_icon("night")

    def _choose_icon(self, variant: str) -> None:
        initial_dir = self.config.get("icons_dir") or str(self.app_dir)
        path = filedialog.askopenfilename(
            title="Choisir image",
            initialdir=initial_dir,
            filetypes=[
                ("Images", "*.svg *.png *.jpg *.jpeg *.bmp"),
                ("Tous les fichiers", "*.*"),
            ],
        )
        if not path:
            return
        path = Path(path)
        filename = path.name
        code = self.code_var.get()
        if not code:
            return
        entry = self.data.get(code, {})
        icon_key = f"icon_{variant}"
        old_name = entry.get(icon_key, "")
        entry[icon_key] = filename
        self.data[code] = entry
        if variant == "day":
            self.day_icon_var.set(filename)
            if self.use_day_for_night.get():
                self._apply_day_to_night()
        else:
            self.night_icon_var.set(filename)
        if old_name and old_name != filename:
            self._propagate_icon_change(icon_key, old_name, filename)
        self._set_dirty(True)
        self._refresh_preview(selected_path=path, variant=variant)

    def _on_frame_slider(self, variant: str, value: str) -> None:
        if self._updating_ui:
            return
        code = self.code_var.get()
        if not code:
            return
        panel = self._get_panel(variant)
        if not panel or panel._frame_count <= 1:
            return
        entry = self.data.get(code, {})
        key = f"frame_{variant}"
        try:
            frame_value = int(float(value))
        except ValueError:
            return
        entry[key] = frame_value
        if variant == "day" and self.use_day_for_night.get():
            entry["frame_night"] = entry.get("frame_day")
            self.night_frame_var.set(self._safe_int(entry.get("frame_night"), 0))
        self.data[code] = entry
        self._set_dirty(True)
        icon_name = entry.get(f"icon_{variant}")
        if icon_name:
            self._propagate_frame_change(variant, str(icon_name), frame_value)
        self._refresh_preview()

    def _resolve_icon_path(self, filename: str) -> Path | None:
        if not filename:
            return None
        candidates = []
        icons_dir = self.config.get("icons_dir")
        if icons_dir:
            candidates.append(Path(icons_dir) / filename)
        if self.current_path:
            candidates.append(self.current_path.parent / filename)
        for candidate in candidates:
            if candidate.exists():
                return candidate
        return None

    def _refresh_preview(self, selected_path: Path | None = None, variant: str | None = None) -> None:
        day_file = self.day_icon_var.get()
        night_file = self.night_icon_var.get()
        day_path = selected_path if variant == "day" else self._resolve_icon_path(day_file)
        night_path = selected_path if variant == "night" else self._resolve_icon_path(night_file)

        day_frames = self._update_frame_state(self.day_panel, day_path, day_file)
        night_frames = self._update_frame_state(self.night_panel, night_path, night_file)
        if Image is None or ImageTk is None:
            self.day_panel._img_label.configure(text="Apercu indisponible")
            self.night_panel._img_label.configure(text="Apercu indisponible")
        else:
            self._set_preview(self.day_panel, day_path, self.day_frame_var.get(), day_frames)
            self._set_preview(self.night_panel, night_path, self.night_frame_var.get(), night_frames)

    def _sorted_codes(self) -> list[str]:
        def key_fn(value: str) -> tuple[int, str]:
            try:
                return (0, f"{int(value):06d}")
            except Exception:
                return (1, str(value))

        return sorted(self.data.keys(), key=key_fn)

    def _iter_entries(self) -> list[tuple[str, dict]]:
        items: list[tuple[str, dict]] = []
        for code_key, payload in self.data.items():
            if isinstance(payload, dict):
                items.append((code_key, payload))
        return items

    def _propagate_icon_change(self, icon_key: str, old_name: str, new_name: str) -> None:
        targets = [
            (code, entry)
            for code, entry in self._iter_entries()
            if entry.get("icon_day") == old_name or entry.get("icon_night") == old_name
        ]
        if len(targets) <= 1:
            return
        for _code, entry in targets:
            if entry.get("icon_day") == old_name:
                entry["icon_day"] = new_name
            if entry.get("icon_night") == old_name:
                entry["icon_night"] = new_name
        self._set_dirty(True)

    def _propagate_frame_change(self, variant: str, icon_name: str, frame_value: int) -> None:
        targets = [
            (code, entry)
            for code, entry in self._iter_entries()
            if entry.get("icon_day") == icon_name or entry.get("icon_night") == icon_name
        ]
        if len(targets) <= 1:
            return
        for _code, entry in targets:
            if entry.get("icon_day") == icon_name:
                entry["frame_day"] = frame_value
            if entry.get("icon_night") == icon_name:
                entry["frame_night"] = frame_value
        self._set_dirty(True)

    def _collect_mapping_entries(self) -> list[dict]:
        entries: list[dict] = []
        for code_key in self.data.keys():
            payload = self.data.get(code_key, {})
            if not isinstance(payload, dict):
                continue
            try:
                code = int(code_key)
            except Exception:
                continue

            def add(icon_key: str, variant: str, frame_key: str) -> None:
                icon_value = payload.get(icon_key)
                if not icon_value:
                    return
                frame_value = self._safe_int(payload.get(frame_key), 0)
                entries.append(
                    {
                        "code": code,
                        "variant": variant,
                        "icon": str(icon_value),
                        "frame": frame_value,
                    }
                )

            if "icon_day" in payload or "icon_night" in payload:
                add("icon_day", "day", "frame_day")
                add("icon_night", "night", "frame_night")
            if "icon" in payload:
                add("icon", "neutral", "frame")
        return entries

    def _generate_bins(self, out_path: Path, size: tuple[int, int], fmt: str, mode: str) -> tuple[bool, str]:
        if Image is None:
            return False, "Pillow manquant."
        if cairosvg is None:
            return False, "CairoSVG manquant."

        entries = self._collect_mapping_entries()
        if not entries:
            return False, "Aucune entree dans le JSON."

        variant_order = {"day": 0, "night": 1, "neutral": 2}
        entries.sort(key=lambda e: (e["code"], variant_order.get(e["variant"], 9)))

        render_cache: dict[tuple, dict] = {}
        render_list: list[dict] = []

        def build_render_key(icon_name: str, frame: int) -> tuple:
            return (icon_name, int(frame), int(size[0]), int(size[1]), fmt)

        for entry in entries:
            icon_name = entry["icon"]
            frame = int(entry.get("frame", 0))
            icon_path = self._resolve_icon_path(icon_name)
            if not icon_path:
                return False, f"Icone introuvable: {icon_name}"

            frame_count = self._detect_svg_frames(icon_path) if icon_path.suffix.lower() == ".svg" else 1
            if frame_count <= 1:
                frame = 0
                entry["frame"] = 0
            else:
                frame = max(0, min(frame, frame_count - 1))
                entry["frame"] = frame

            render_key = build_render_key(icon_name, frame)
            render_info = render_cache.get(render_key)
            if not render_info:
                image = self._render_image(icon_path, frame, size)
                if image is None:
                    return False, f"Rendu impossible: {icon_name}"
                data = self._encode_image(image, fmt)
                render_info = {
                    "icon": icon_name,
                    "frame": frame,
                    "data": data,
                    "width": size[0],
                    "height": size[1],
                }
                render_cache[render_key] = render_info
                render_list.append(render_info)
            entry["render_key"] = render_key

        if mode == "per_file":
            out_dir = out_path
            out_dir.mkdir(parents=True, exist_ok=True)
            bin_map: dict[tuple, str] = {}
            used_names: set[str] = set()
            for render_info in render_list:
                base = Path(render_info["icon"]).stem
                if render_info["frame"] > 0:
                    base = f"{base}_f{render_info['frame']}"
                filename = base + ".bin"
                if filename in used_names:
                    suffix = 1
                    while f"{base}_{suffix}.bin" in used_names:
                        suffix += 1
                    filename = f"{base}_{suffix}.bin"
                used_names.add(filename)
                bin_map[(render_info["icon"], render_info["frame"], size[0], size[1], fmt)] = filename
                (out_dir / filename).write_bytes(render_info["data"])

            index_entries = []
            for entry in entries:
                key = build_render_key(entry["icon"], entry["frame"])
                render_info = render_cache[key]
                index_entries.append(
                    {
                        "code": entry["code"],
                        "variant": entry["variant"],
                        "name": Path(entry["icon"]).stem,
                        "bin": bin_map[key],
                        "offset": 0,
                        "data_len": len(render_info["data"]),
                        "width": render_info["width"],
                        "height": render_info["height"],
                        "format": fmt,
                    }
                )
            self._write_index_header(out_dir / "index.h", index_entries)
            return True, f"BIN genere dans {out_dir}"

        if fmt != "RGB565":
            return False, "Mode merge: format RGB565 requis."

        index_entries = self._build_merge_index(entries, render_list, size, fmt)
        data_start = 8 + 8 * len(index_entries)
        index_blob = self._build_index_blob(index_entries, data_start)

        merged = bytearray()
        for render_info in render_list:
            name = Path(render_info["icon"]).stem
            entry = self._build_merge_entry(
                name, (render_info["width"], render_info["height"]), render_info["data"]
            )
            merged.extend(entry)

        out_path.parent.mkdir(parents=True, exist_ok=True)
        header = b"S2BI" + struct.pack("<HH", 1, len(index_entries))
        out_path.write_bytes(header + index_blob + bytes(merged))

        return True, f"BIN genere: {out_path}"

    def _render_image(self, path: Path, frame: int, size: tuple[int, int]):
        if Image is None:
            return None
        try:
            if path.suffix.lower() == ".svg":
                png_bytes = self._render_svg_frame(path, frame, size)
                if png_bytes is None:
                    return None
                image = Image.open(io.BytesIO(png_bytes)).convert("RGBA")
            else:
                image = Image.open(path).convert("RGBA")
                image = image.resize(size, Image.LANCZOS)
            return image
        except Exception:
            return None

    def _encode_image(self, image, fmt: str) -> bytes:
        width, height = image.size
        if fmt == "RGB565":
            rgb = image.convert("RGB")
            data = bytearray()
            for r, g, b in rgb.getdata():
                rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
                data.extend(rgb565.to_bytes(2, byteorder="little"))
            return bytes(data)
        if fmt == "RGB888":
            rgb = image.convert("RGB")
            return rgb.tobytes()
        if fmt == "ARGB8888":
            rgba = image.convert("RGBA")
            data = bytearray()
            for r, g, b, a in rgba.getdata():
                data.extend(bytes((a, r, g, b)))
            return bytes(data)
        if fmt == "A8":
            rgba = image.convert("RGBA")
            return bytes([a for (_, _, _, a) in rgba.getdata()])
        if fmt == "A8A8":
            rgba = image.convert("RGBA")
            data = bytearray()
            for r, g, b, a in rgba.getdata():
                luma = int(0.2126 * r + 0.7152 * g + 0.0722 * b)
                data.extend(bytes((a, luma)))
            return bytes(data)
        return image.convert("RGB").tobytes()

    def _build_merge_entry(self, name: str, size: tuple[int, int], payload: bytes) -> bytes:
        name_bytes = name.encode("utf-8")
        header = (
            struct.pack("<H", len(name_bytes))
            + name_bytes
            + struct.pack("<HHIB", size[0], size[1], len(payload), 0)
        )
        return header + payload

    def _build_index_blob(self, index_entries: list[dict], data_start: int) -> bytes:
        variant_map = {"day": 0, "night": 1, "neutral": 2}
        blob = bytearray()
        for entry in index_entries:
            variant_name = entry.get("variant")
            variant = variant_map.get(variant_name, 2)
            offset = int(entry["offset"]) + data_start
            blob.extend(struct.pack("<HBBI", int(entry["code"]), int(variant), 0, offset))
        return bytes(blob)

    def _build_merge_index(
        self,
        entries: list[dict],
        render_list: list[dict],
        size: tuple[int, int],
        fmt: str,
    ) -> list[dict]:
        index_entries: list[dict] = []
        offset_map: dict[tuple, int] = {}
        running = 0
        for render_info in render_list:
            render_key = (render_info["icon"], render_info["frame"], size[0], size[1], fmt)
            name = Path(render_info["icon"]).stem
            entry = self._build_merge_entry(name, size, render_info["data"])
            offset_map[render_key] = running
            running += len(entry)

        for entry in entries:
            key = (entry["icon"], int(entry.get("frame", 0)), size[0], size[1], fmt)
            index_entries.append(
                {
                    "code": entry["code"],
                    "variant": entry["variant"],
                    "offset": offset_map[key],
                }
            )
        return index_entries

    def _write_index_header(self, path: Path, entries: list[dict]) -> None:
        lines = [
            "/* Auto-generated by svg2bin_gui.py. */",
            "#pragma once",
            "",
            "#include <stddef.h>",
            "#include <stdint.h>",
            "",
            "typedef enum {",
            "    SVG2BIN_FMT_RGB565 = 0,",
            "    SVG2BIN_FMT_RGB888 = 1,",
            "    SVG2BIN_FMT_ARGB8888 = 2,",
            "    SVG2BIN_FMT_A8 = 3,",
            "    SVG2BIN_FMT_A8A8 = 4,",
            "} svg2bin_gui_fmt_t;",
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
            "    uint16_t width;",
            "    uint16_t height;",
            "    svg2bin_gui_fmt_t format;",
            "    uint32_t offset;",
            "    uint32_t data_len;",
            "    const char *name;",
            "    const char *bin;",
            "} svg2bin_gui_index_entry_t;",
            "",
            "static const svg2bin_gui_index_entry_t svg2bin_gui_index[] = {",
        ]
        for entry in entries:
            variant_value = entry.get("variant")
            variant_enum = "SVG2BIN_VARIANT_NEUTRAL"
            if variant_value == "day":
                variant_enum = "SVG2BIN_VARIANT_DAY"
            elif variant_value == "night":
                variant_enum = "SVG2BIN_VARIANT_NIGHT"
            fmt_enum = self._format_enum(entry.get("format", "RGB565"))
            lines.append(
                '    {%d, %s, %d, %d, %s, %d, %d, "%s", "%s"},'
                % (
                    entry.get("code", 0),
                    variant_enum,
                    entry.get("width", 0),
                    entry.get("height", 0),
                    fmt_enum,
                    entry.get("offset", 0),
                    entry.get("data_len", 0),
                    entry.get("name", ""),
                    entry.get("bin", ""),
                )
            )
        lines.extend(
            [
                "};",
                "",
                "static const size_t svg2bin_gui_index_count =",
                "    sizeof(svg2bin_gui_index) / sizeof(svg2bin_gui_index[0]);",
                "",
            ]
        )
        path.write_text("\n".join(lines), encoding="utf-8")

    def _format_enum(self, fmt: str) -> str:
        mapping = {
            "RGB565": "SVG2BIN_FMT_RGB565",
            "RGB888": "SVG2BIN_FMT_RGB888",
            "ARGB8888": "SVG2BIN_FMT_ARGB8888",
            "A8": "SVG2BIN_FMT_A8",
            "A8A8": "SVG2BIN_FMT_A8A8",
        }
        return mapping.get(fmt, "SVG2BIN_FMT_RGB565")

    def _set_preview(self, panel: ttk.Frame, path: Path | None, frame_index: int, frame_count: int) -> None:
        label = panel._img_label
        if not path or not path.exists():
            label.configure(image="", text="Apercu indisponible")
            return
        try:
            if path.suffix.lower() == ".svg":
                if cairosvg is None:
                    label.configure(image="", text="Apercu indisponible")
                    return
                if frame_count > 1:
                    png_bytes = self._render_svg_frame(path, frame_index)
                    if png_bytes is None:
                        label.configure(image="", text="Apercu indisponible")
                        return
                else:
                    png_bytes = cairosvg.svg2png(bytestring=path.read_bytes())
                image = Image.open(io.BytesIO(png_bytes))
            else:
                image = Image.open(path)
            size = int(self.config.get("preview_size", 128))
            image.thumbnail((size, size))
            photo = ImageTk.PhotoImage(image)
        except Exception:
            label.configure(image="", text="Apercu indisponible")
            return
        self._preview_refs[str(panel)] = photo
        label.configure(image=photo, text="")

    def _update_frame_state(self, panel: ttk.Frame, path: Path | None, filename: str) -> int:
        if not filename or not filename.lower().endswith(".svg") or not path or not path.exists():
            self._hide_frame_panel(panel)
            return 0

        frame_count = self._detect_svg_frames(path)
        if frame_count <= 1:
            self._hide_frame_panel(panel)
            return 1

        self._show_frame_panel(panel, frame_count)
        return frame_count

    def _show_frame_panel(self, panel: ttk.Frame, frame_count: int) -> None:
        if not panel._frame_visible:
            panel._frame_row.pack(fill=tk.X, pady=4)
            panel._frame_visible = True
        panel._frame_count = frame_count
        panel._frame_scale.configure(from_=0, to=frame_count - 1)
        current = self.day_frame_var.get() if panel._variant == "day" else self.night_frame_var.get()
        if current > frame_count - 1:
            current = frame_count - 1
            if panel._variant == "day":
                self.day_frame_var.set(current)
            else:
                self.night_frame_var.set(current)

    def _hide_frame_panel(self, panel: ttk.Frame) -> None:
        if panel._frame_visible:
            panel._frame_row.pack_forget()
            panel._frame_visible = False
        panel._frame_count = 0

    def _safe_int(self, value, default: int) -> int:
        try:
            return int(value)
        except Exception:
            return default

    def _get_panel(self, variant: str) -> ttk.Frame | None:
        if variant == "day":
            return self.day_panel
        if variant == "night":
            return self.night_panel
        return None

    def _localname(self, tag: str) -> str:
        return tag.split("}", 1)[-1] if "}" in tag else tag

    def _detect_svg_frames(self, path: Path) -> int:
        info = self._get_svg_info(path)
        return info["frames"]

    def _get_svg_info(self, path: Path) -> dict:
        key = str(path)
        try:
            mtime = path.stat().st_mtime
        except OSError:
            return {"frames": 0}
        cached = self._svg_info_cache.get(key)
        if cached and cached.get("mtime") == mtime:
            return cached

        frames = 0
        try:
            root = ET.fromstring(path.read_bytes())
        except Exception:
            frames = 0
        else:
            for elem in root.iter():
                if self._localname(elem.tag) in ("animate", "animateTransform", "set"):
                    values = elem.get("values")
                    if values:
                        parts = [v for v in values.split(";") if v.strip()]
                        frames = max(frames, len(parts))
                    else:
                        key_times = elem.get("keyTimes")
                        if key_times:
                            parts = [v for v in key_times.split(";") if v.strip()]
                            frames = max(frames, len(parts))
                        else:
                            frames = max(frames, 2)

        info = {"frames": frames, "mtime": mtime}
        self._svg_info_cache[key] = info
        return info

    def _render_svg_frame(
        self, path: Path, frame_index: int, size: tuple[int, int] | None = None
    ) -> bytes | None:
        try:
            root = ET.fromstring(path.read_bytes())
        except Exception:
            return None
        self._apply_svg_frame(root, frame_index)
        svg_bytes = ET.tostring(root, encoding="utf-8")
        try:
            if size:
                return cairosvg.svg2png(
                    bytestring=svg_bytes,
                    output_width=int(size[0]),
                    output_height=int(size[1]),
                )
            return cairosvg.svg2png(bytestring=svg_bytes)
        except Exception:
            return None

    def _apply_svg_frame(self, root: ET.Element, frame_index: int) -> None:
        for parent in root.iter():
            to_remove = []
            for child in list(parent):
                tag = self._localname(child.tag)
                if tag in ("animate", "animateTransform", "set"):
                    self._apply_anim_to_parent(parent, child, frame_index)
                    to_remove.append(child)
            for child in to_remove:
                parent.remove(child)

    def _apply_anim_to_parent(self, parent: ET.Element, anim: ET.Element, frame_index: int) -> None:
        tag = self._localname(anim.tag)
        attr = anim.get("attributeName")
        value = self._select_anim_value(anim, frame_index)
        if not value:
            return

        if tag == "animateTransform":
            transform_type = anim.get("type")
            if transform_type and "(" not in value:
                value = f"{transform_type}({value})"
            existing = parent.get("transform")
            if existing:
                parent.set("transform", f"{existing} {value}")
            else:
                parent.set("transform", value)
            return

        if not attr:
            return
        parent.set(attr, value)

    def _select_anim_value(self, anim: ET.Element, frame_index: int) -> str:
        values = anim.get("values")
        if values:
            parts = [v.strip() for v in values.split(";") if v.strip()]
            if not parts:
                return ""
            return parts[frame_index % len(parts)]
        from_value = anim.get("from")
        to_value = anim.get("to")
        if frame_index <= 0:
            return from_value or to_value or ""
        return to_value or from_value or ""

    def _on_close(self) -> None:
        if self.dirty:
            res = messagebox.askyesnocancel("Fermer", "Enregistrer les modifications ?")
            if res is None:
                return
            if res:
                self._save()
                if self.dirty:
                    return
        self.root.destroy()


def main() -> None:
    root = tk.Tk()
    app = Svg2BinGui(root)
    root.mainloop()


if __name__ == "__main__":
    main()
