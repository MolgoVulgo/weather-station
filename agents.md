# agent-esp.md

## Contexte d’utilisation

Ce fichier définit le cadre de fonctionnement d’un **agent Codex (extension VS Code)** pour des projets **ESP / ESP32**.

* Cibles : **ESP8266 / ESP32**
* Frameworks : **Arduino** ou **ESP-IDF**
* Environnement typique : **PlatformIO**

---

## Contraintes matérielles et logicielles

* Systèmes **embarqué temps réel**, ressources limitées (RAM / Flash / CPU).
* Boucle principale critique (Arduino `loop()` / IDF tasks).
* Toute modification doit respecter :

  * contraintes mémoire,
  * latence,
  * stabilité runtime.

---

## Objectifs prioritaires

1. **Fonctionnement correct sur cible réelle**
2. **Stabilité (pas de reset, pas de WDT)**
3. **Lisibilité du code embarqué**
4. **Modifications minimales et localisées**

---

## Style et posture de l’agent

* Direct, orienté exécution.
* Peu de texte, code en priorité.
* Aucune pédagogie.
* Questionner **uniquement** si une ambiguïté bloque l’implémentation.
* **Toujours répondre en français.**

---

## Spécificités du projet

* Framework actif : **ESP-IDF** via PlatformIO (`platformio.ini`).
* Point d’entrée : `src/weatherStation.c` (fonction `app_main()`).
* UI LVGL : backend sélectionné par flag PlatformIO.
  * `-D UI_BACKEND_EEZ=1` utilise `src/ui/` (EEZ Studio).
* Wrapper neutre UI : `src/ui_backend.h` (évite les noms spécifiques UI dans le code applicatif).
* Documentation technique : `DOCUMENTATION.md` (à maintenir à jour).

---

## Lecture du dépôt (ordre imposé)

1. `platformio.ini`
2. Arborescence `src/`, `lib/`, `components/`
3. Fichiers de configuration spécifiques (pins, sdkconfig, partitions)
4. README / `docs/` si présents et `/docs` s’ils existent
5. Vérifier la cohérence des types entre `src/ui/vars.h` et `src/ui/screens.c` (même type attendu pour chaque `get_var_*`) avant toute analyse.

---

## Check-list de validation (rapide)

* Types `get_var_*` cohérents entre `src/ui/vars.h` et `src/ui/screens.c`
* Aucune modification directe dans `src/ui/` (fichiers générés)
* Framework confirmé via `platformio.ini`
* Logs activables via flag `-DDEBUG_LOG`

---

## Politique de modification du code

* **Exécuter immédiatement** la solution la plus probable.
* **Aucune discussion** tant que le code compile et tourne.
* Adapter le code au framework actif (Arduino **ou** ESP-IDF, jamais hybride).
* **Interdiction absolue** de modifier les fichiers dans `src/ui/` (générés).
* Si un problème est détecté dans `src/ui/`, **l’expliquer clairement** sans modifier ces fichiers.

---

## Tests

* **Aucun test unitaire ou automatisé**.
* La validation se fait **sur la carte réelle** uniquement.

---

## Debug et logs (obligatoire)

* **Toujours implémenter un système de logs**.

### Arduino

* Utiliser `Serial` ou `Serial.printf`.
* Initialisation explicite dans `setup()`.

### ESP-IDF

* Utiliser `ESP_LOGx` (`ESP_LOGI`, `ESP_LOGW`, `ESP_LOGE`).
* Respecter les tags par module.

### PlatformIO

* Le **debug log doit être activable/désactivable par flag**.
* Exemple :

  ```ini
  build_flags =
    -DDEBUG_LOG
  ```
* Les logs doivent être **désactivables sans modifier le code fonctionnel**.

---

## Qualité et conventions

* Pas d’allocation dynamique inutile.
* Pas de blocage long dans la boucle principale.
* Gestion explicite des erreurs matérielles.
* Code lisible, structuré, prévisible.

---

## Formats de sortie attendus

* Fichiers prêts à compiler et flasher.
* Le framework ciblé (**Arduino** ou **ESP-IDF**) est déduit de `platformio.ini` ; s’il est absent ou ambigu, **demander explicitement**.
* Flags PlatformIO requis.

---

## Structure de réponse attendue

* Constat technique
* Action(s) appliquée(s)
* Indication de validation sur cible
* Hypothèses si nécessaires
* Après toute génération ou modification de code, analyser le diff réel et les fichiers impactés afin de produire un message de commit en anglais, à l’impératif, reflétant l’intention principale du changement, sa portée fonctionnelle ou technique, et conforme aux conventions usuelles de Git (pas de résumé vague ou tautologique).

---

## Commits Git

* Faire des commits par unité logique cohérente, compilable, et validable sur carte.
* Ne jamais faire de commit automatiquement sans demande explicite de l’utilisateur.
* Ne jamais pousser soi-même.
* Messages de commit en anglais, à l’impératif, alignés strictement sur le diff réel.
* Conventional Commit recommandé :

  ```text
  <type>(<scope>): <summary>
  ```

Scopes recommandés dans ce projet :

* `firmware` : logique applicative principale (`src/weatherStation.c`, services, orchestration).
* `lvgl` : intégration UI LVGL hors fichiers générés (`src/lv_*`, `src/ui_screen.*`, `src/ui_backend.h`, `src/vars.c`).
* `weather` : récupération/cache/normalisation météo (`libraries/weather/`, `src/weather_*`, `src/hourly_strip.*`).
* `wifi` : connectivité et portail (`src/wifi_*`, `src/time_sync.*`).
* `bsp` : drivers écran/tactile/carte (`src/esp_bsp.*`, `src/esp_lcd_*`, `src/display.h`, `src/boot_progress.*`).
* `build` : configuration build/flash (`platformio.ini`, `partitions.csv`, `sdkconfig*`, `CMakeLists.txt`).
* `docs` : documentation (`README*`, `docs/*`, `DOCUMENTATION.md`).

Le message doit décrire exactement le diff réel après vérification de :

* `git status --short`
* `git diff --stat HEAD`
* `git diff HEAD`

Mentionner dans le corps :

* changements fonctionnels ;
* robustesse / gestion d’erreurs (Wi-Fi, réseau, matériel, watchdog) ;
* impact contrat de données météo (JSON parsé/champs attendus) ;
* impact build/config (flags, partitions, sdkconfig) ;
* validation effectuée : build PlatformIO et test sur carte réelle ;
* docs mises à jour.

Template :

```text
<type>(<scope>): <summary>

- ...
- ...

Tests: pio run -e LVGL-320-480 (<result>)
Hardware: <validated on target | not run>
Docs: <files list or "none">
```
