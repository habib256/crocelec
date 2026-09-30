# CrocElec

Simulateur de circuits électriques **pour le collège** — symboles normalisés du
cahier, unités françaises, lois du programme vérifiées à l'écran.

> Cahier des charges complet : [`docs/CAHIER-DES-CHARGES.md`](docs/CAHIER-DES-CHARGES.md)

## Construire et lancer

```bash
./setup_imgui.sh            # Dear ImGui (branche docking, commit épinglé) + GLFW
cmake -S . -B build
cmake --build build -j
./build/crocelec
```

## Recette

```bash
./build/crocelec_tests      # 90 vérifications chiffrées, tirées des cours
```

Chaque test reprend une valeur écrite dans un chapitre : 9 V et cinq lampes en
série → 1,80 V chacune ; R = 60 Ω sous 6 V → 100 mA ; DEL 2 V protégée par
350 Ω sur 9 V → 20 mA ; R = 115 Ω sous 230 V → 2 A. Si l'un tombe, une valeur
montrée à un élève est devenue fausse.

## Où en est le logiciel

| Lot | Contenu | État |
|---|---|---|
| **V0 — socle** | fenêtre dockée, canevas, solveur continu, animation du courant | **fait** |
| **V1 — 5e** | série/dérivation, nœuds, panneau Lois, mode panne, appareils simplifiés | **fait**, sauf la double vue schéma ⟷ paillasse |
| **V2 — 4e** | multimètre à calibres, relevés, export CSV, build tablette | **fait**, sauf le build WebAssembly |
| **V3 — 3e** | générateur réglable, caractéristique U = f(I), puissance et énergie | **fait**, sauf le glisser-déposer d'une grandeur vers le graphique |

Déjà utilisable en classe : les dix-huit montages de la **Bibliothèque** —
circuit simple, série, dérivation, TP 9 V / 5 lampes, loi d'Ohm avec voltmètre
et ampèremètre, mesure au multimètre, caractéristique U = f(I) d'une résistance
puis d'une lampe, DEL protégée, guirlande, court-circuit, test conducteur /
isolant, fusible, circuit mixte, applications (lave-linge, smartphone,
bouilloire), ventilateur, puissance et énergie, installation domestique.
Restent à saisir, faute du schéma exact du cours : le circuit mixte du TP
4,50 / 9,00 / 3,00 V, les trois résistances et trois voltmètres, la résistance
270 Ω à code couleurs.

## Ce qui est en place

- **Deux modes** (menu Mode) : **Simple (élève)**, actif au démarrage — cinq
  panneaux seulement (Schéma, Palette, Propriétés, Lois/Mesures en onglets,
  Bibliothèque), gros boutons, texte agrandi, dernier message affiché en
  bandeau au bas du schéma ; **Complet (professeur)** — s'y ajoutent
  Multimètre, Relevés, Graphique, Journal, la bascule Électrons et les
  réglages d'affichage.
- **Éditeur** : les trois gestes de Crocodile Clips — poser un dipôle au
  glisser, **tirer un fil depuis l'extrémité d'un dipôle** (tracé orthogonal,
  sans changer d'outil), effacer à la **Gomme** en passant dessus. Quart de
  tour avec R ou le bouton « Tourner » (utilisable au doigt), déplacement,
  annulation, zoom molette. Un clic sur un interrupteur le manœuvre en cours de
  simulation. Une extrémité posée au milieu d'un fil s'y raccorde (jonction en T).
- **Palette par familles** (générateurs, commande, récepteurs, sécurité et
  matériaux, mesure) avec un **filtre par niveau** : en 4e, on voit ce qui sert
  en 6e, 5e et 4e, pas la prise 230 V de 3e.
- **Solveur** : analyse nodale modifiée, régime continu, permanent (aucun bouton
  « lancer »). Lampe **non ohmique** (sa résistance monte avec la température),
  DEL à tension de seuil, court-circuit calculé au lieu de faire échouer le calcul.
- **Panneau Lois** : unicité de l'intensité en série, additivité des tensions,
  loi des nœuds, unicité de la tension en dérivation, loi d'Ohm — énoncées et
  vérifiées sur le montage de l'élève.
- **Multimètre** (4e ch.7) : bornes COM / V·Ω·mA / 10 A, sélecteur V–A–Ω,
  calibres 2 / 20 / 200 / 1000 V, 2 / 20 / 200 mA et 10 A, affichage **« 1. »**
  au dépassement. L'impédance suit la **borne**, pas le sélecteur : cordon rouge
  sur 10 A en dérivation, et le **fusible fond** avec l'explication. L'ohmmètre
  mesure la résistance vue générateurs éteints, et prévient si le circuit est
  encore alimenté.
- **Relevés et graphique** : bouton « Relever », tableau U/I/R/P, **export CSV**
  (point-virgule, virgule décimale) ouvrable tel quel dans LibreOffice Calc,
  tracé U = f(I) avec droite de régression, coefficient directeur = R et verdict
  « conducteur ohmique » ou non.
- **Puissance et énergie** (3e ch.7, panneau Énergie, mode complet) :
  wattmètre sur l'appareil choisi, E = P × t en joules et en kWh, coût au
  tarif saisi (0,20 €/kWh par défaut, à ajuster), compteur de l'installation
  en temps accéléré. Le panneau Lois ajoute P = U × I et vérifie que la
  puissance fournie égale la somme des puissances reçues.
- **Secteur** : prise 230 V (ou 220 V) avec disjoncteur réarmable (2 à 32 A),
  bandeau de danger permanent, courant animé en va-et-vient (alternatif).
- **6e ch.13** : matériau test entre deux pinces (cuivre, graphite, eau
  salée, plastique, bois sec, verre) et fusible réarmable.
- **Sécurité et pannes** : clic droit sur un dipôle pour le faire griller, le
  remplacer, inverser une pile ; lampe et DEL qui grillent ; court-circuit
  signalé et commenté.
- **Fichiers** : format `.croc` texte, lisible et versionnable.

## Structure

```
src/Circuit.*     dipôles, netlist (nœuds, branches, dérivations)
src/Solver.*      MNA + point fixe pour la lampe et la DEL + résolution LU
src/Schematic.cpp canevas : symboles normalisés, courant animé, édition
src/Panels.cpp    Palette, Propriétés, Mesures, Lois, Journal, Bibliothèque
src/Instruments.* multimètre, tableau de relevés, tracé U = f(I)
src/Exemples.cpp  montages du cours livrés avec le logiciel
src/Format.*      écriture française des grandeurs (virgule, mA, kΩ)
src/Serialize.*   fichiers .croc
tests/            recette chiffrée, sans interface
```

Dear ImGui n'est pas versionné : `setup_imgui.sh` le récupère au commit épinglé
dans `imgui_pin.env`.
