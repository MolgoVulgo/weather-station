# Logique du chart `ui_hourlychart` (LVGL 8.4)

## 1) Objet et parent
- Le chart est cree via lv_chart_create(parent).
- Le parent est le conteneur EEZ `ui_detail_chart`.
- Le chart est recree si `ui_detail_chart` est recree.

## 2) Taille et position
- Taille = largeur du conteneur (max 400px) / hauteur 100px.
- Position x=10, y=0 puis alignement centre (pour centrer si le parent est plus large).

## 3) Type du chart
- lv_chart_set_type(..., LV_CHART_TYPE_LINE)
Le chart est un graphique en lignes.

## 4) Nombre de points
- lv_chart_set_point_count(..., 7)
Le chart reserve 7 points pour chaque serie.

## 5) Plages d'axes
- Plage initiale: 0..20 (initialisation lors de la creation du chart).
- Plage dynamique: recalcul automatique par `hourly_strip_chart_compute_range()`.
  -> Pas de 5°C ou 10°F, plage fixee a 20°C ou 40°F, ajustee autour des valeurs visibles.
- lv_chart_set_range(..., LV_CHART_AXIS_SECONDARY_Y, 0, 0)
  -> L'axe Y secondaire est neutralise (min = max).

## 6) Grille
- lv_chart_set_div_line_count(..., 5, 7)
  -> 5 lignes horizontales, 7 lignes verticales.

## 7) Ticks des axes
- lv_chart_set_axis_tick(..., LV_CHART_AXIS_PRIMARY_X, 10, 2, 7, 3, false, 50)
  * 10: longueur des ticks principaux
  * 2: longueur des ticks mineurs
  * 7: nombre de ticks principaux
  * 3: nombre de ticks mineurs par intervalle
  * false: libelles des ticks en dehors du chart (false = a l'interieur)
  * 50: espacement des libelles (en px)

- lv_chart_set_axis_tick(..., LV_CHART_AXIS_PRIMARY_Y, 10, 5, 5, 2, true, 50)
  * 10: longueur ticks principaux
  * 5: longueur ticks mineurs
  * 5: nombre de ticks principaux
  * 2: nombre de ticks mineurs par intervalle
  * true: libelles a l'exterieur
  * 50: espacement des libelles

- lv_chart_set_axis_tick(..., LV_CHART_AXIS_SECONDARY_Y, 0, 0, 0, 0, false, 25)
  -> Axe secondaire neutralise (ticks a 0)

## 8) Serie et donnees
- lv_chart_add_series(..., lv_color_hex(0xB7B7B7), LV_CHART_AXIS_PRIMARY_Y)
  -> Cree une serie gris clair, liee a l'axe Y principal.

- static lv_coord_t s_hourly_chart_series_1_array[] = { 2, 5, 8, 9, 7, 6, 3 };
- lv_chart_set_ext_y_array(...)
  -> Le chart pointe vers un tableau externe (statique) pour les valeurs Y.
  -> IMPORTANT: ce tableau doit rester en vie (static ou global).
  -> Le contenu est mis a jour par `hourly_strip_chart_sync()` en tenant compte de l'unite (°C/°F).

## 9) Style
- Fond interne: #141B1E.
- Cadre: #2A3336.
- Grille principale: #263034.
- Ticks/labels: #D0D0D0 avec la font ui_16.
- Points: #C2C2C2 avec contour #9FA4A6.

## 10) Ligne zero
- Un callback de dessin ajoute une ligne horizontale a la valeur 0 (tick principal Y).

## 11) Points d'attention pour re-integration
- Si tu changes lv_chart_set_point_count, adapte la taille du tableau.
- Si tu veux un axe secondaire, definis une vraie plage et des ticks.
- Pense a appeler lv_chart_refresh(chart) apres mise a jour du tableau si besoin.
