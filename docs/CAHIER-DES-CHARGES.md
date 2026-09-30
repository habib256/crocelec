# CrocElec — Cahier des charges

> Simulateur de circuits électriques pour le collège
> M. VERHILLE — Physique-Chimie · Collège · La Réunion
> Version 0.1 du cahier des charges — 19/08/2026

---

## 1. Fiche projet

| | |
|---|---|
| **Nom** | CrocElec |
| **Nature** | Logiciel de bureau (macOS / Windows / Linux) + version web (WASM) |
| **Utilisateurs** | Professeur de physique-chimie (vidéoprojection, préparation) et élèves de 6e à 3e (salle informatique, maison) |
| **Objet** | Construire, simuler, mesurer et exploiter **tous** les circuits électriques du programme de collège (cycle 3 fin + cycle 4) |
| **Phrase-repère** | « Tout ce que l'élève monte sur la paillasse, il doit pouvoir le monter, le mesurer et le rater sans danger dans CrocElec — avec **les mêmes symboles, les mêmes appareils et les mêmes unités** que dans le cours. » |

---

## 2. Contexte et justification

Les TP utilisent aujourd'hui **PhET Circuit Construction Kit (DC)** (cf. 4e ch.8, 5e ch.8) et TinkerCAD. Ces outils sont excellents mais laissent quatre manques identifiés dans les cours :

1. **Le multimètre réel n'est pas modélisé** : bornes COM / V / mA / 10A, sélecteur de calibre, affichage « 1. » de dépassement, interdit de l'ampèremètre en dérivation. Or le chapitre 4e ch.7 porte **entièrement** là-dessus, et l'évaluation est un TP de mesure.
2. **Les symboles ne sont pas ceux du cahier** : PhET dessine des piles « réalistes » ; le cours exige le **schéma normalisé** (5e ch.7 : « Lire et réaliser un schéma de circuit normalisé »).
3. **Pas d'exploitation des mesures** : le TP « caractéristique U = f(I) » (4e ch.9, 3e ch.6) demande un tableau de valeurs, un tracé, une droite de tendance, une pente = R. Aujourd'hui il faut sortir du simulateur, recopier à la main et passer au tableur.
4. **Connexion Internet obligatoire** et interface partiellement anglophone. CrocElec doit tourner **hors ligne**, en français, avec **virgule décimale**.

CrocElec ne remplace pas la paillasse : il sert à **préparer**, **vérifier**, **rejouer** et **différencier** ce qui y est fait, et à faire l'expérience de ce qui est trop dangereux ou trop lent en vrai (court-circuit, ampèremètre grillé, 230 V, guirlande de 20 lampes).

### 2.1 Référence d'inspiration : Crocodile Clips / Yenka

Le modèle d'interaction visé est celui de **Crocodile Physics / Crocodile Technology** (aujourd'hui Yenka), qui reste la meilleure ergonomie jamais proposée pour l'électricité au collège. Ce qu'on en reprend, et ce qu'on écarte :

| Trait de Crocodile Clips | Décision |
|---|---|
| **Double vue** : le même circuit en composants réalistes **ou** en symboles normalisés, bascule instantanée | **Repris — fonction centrale.** L'élève monte en vue « paillasse » puis fait apparaître le schéma normalisé : c'est exactement la compétence « lire et réaliser un schéma normalisé » (5e ch.7), rendue visible |
| **Simulation toujours vivante** : pas de bouton « lancer », tout réagit pendant qu'on manipule | **Repris.** Aucun mode « édition / exécution » séparé |
| **Valeurs affichées à même les composants** (petit afficheur accroché au dipôle) | **Repris**, avec un réglage prof (tout / sélection / aucun) car en évaluation on veut pouvoir les masquer |
| **Glisser une grandeur vers une fenêtre graphique** pour la tracer en direct | **Repris.** C'est ce qui rend le TP « caractéristique U = f(I) » naturel |
| **Bibliothèque arborescente** de composants à gauche, glisser-déposer sur la scène | **Repris**, avec en plus un filtre par niveau (5e / 4e / 3e) |
| **Connexion directe** : deux composants posés bout à bout se relient sans tirer de fil | **Repris.** Le fil reste possible, mais n'est pas obligatoire |
| **Curseurs sur toutes les valeurs** (tension du générateur, résistance variable) manipulables en cours de simulation | **Repris** |
| **Mode leçon** : annotations, textes, flèches sur la scène, composants verrouillés, document distribué aux élèves | **Repris** (lot V4), c'est la base des activités guidées |
| **Multi-domaines** (mécanique, optique, ondes, électronique, PIC, 3D) | **Écarté.** CrocElec fait l'électricité, et rien d'autre : les autres domaines sont déjà couverts par mes autres micromondes |
| Logiciel propriétaire, licence établissement, installation lourde | **Écarté.** Libre, hors ligne, portable, et une version web sans installation |
| Interface anglophone, symboles britanniques (résistance en zigzag, pile double trait) | **Écarté.** Français, symboles NF tels que dessinés dans le cahier |

### 2.2 État de l'art — ce qui existe de simple en ligne

| Outil | Ce qu'il fait bien | Pourquoi il ne suffit pas ici |
|---|---|---|
| **PhET — Circuit Construction Kit (DC)** (HTML5, gratuit, hors ligne possible) | Manipulation très simple, courant animé, ampèremètre/voltmètre, adapté au collège. C'est la référence actuelle de mes TP | Pas de multimètre à calibres, symboles non normalisés NF, pas d'exploitation tableur intégrée, interface partiellement anglophone |
| **Falstad — Circuit Simulator (CircuitJS1)**, libre GPL, version hors ligne | Simulation temps réel, dots de courant, tension en couleur, oscilloscope intégré, plus de 1 000 circuits d'exemple, partage par simple lien | Pensé pour l'électronique (lycée/sup), très dense, vocabulaire anglais, aucune notion de multimètre scolaire ni de schéma NF |
| **DCACLab** | Vue « paillasse » réaliste avec multimètre à sélecteur, très proche du geste réel | En ligne, compte requis pour l'essentiel, orienté lycée/technique, payant au-delà de l'essai |
| **Tinkercad Circuits** | Breadboard réaliste, multimètre, Arduino, gratuit | Compte Autodesk obligatoire, lourd, en ligne uniquement, hors sujet pour le circuit simple de 5<sup>e</sup> |
| **EveryCircuit** | Excellent sur tablette, animation des charges très parlante | Payant, orienté électronique, pas de schéma normalisé NF |
| **Yenka / Crocodile** | Le modèle d'interaction de référence (§ 2.1) | Propriétaire, licence établissement, installation lourde, plus vraiment maintenu |
| **SimulIDE, Qucs-S** (bureau, libres) | Solveurs solides, gratuits | Interfaces d'électronicien, aucune adaptation collège |

**Ce que le tour d'horizon confirme** : personne ne couvre le créneau « collège français » — symboles NF, multimètre avec bornes et calibres, virgule décimale, lois du programme vérifiées à l'écran, export tableur. C'est exactement la niche de CrocElec, et elle se tient en un logiciel **simple** : une trentaine de dipôles suffit, pas mille.

**Ce qu'on leur emprunte concrètement** : les dots de courant animés et la tension en couleur (Falstad), la simplicité de prise en main et le rendu « bac à sable » (PhET), le multimètre manipulable au sélecteur (DCACLab), la fluidité tactile (EveryCircuit), la double vue et le mode leçon (Crocodile).

---

## 3. Utilisateurs et scénarios d'usage

| # | Scénario | Niveau | Ce que le logiciel doit permettre |
|---|---|---|---|
| U1 | **Cours vidéoprojeté** : je montre circuit ouvert/fermé, sens du courant, série vs dérivation | 5e ch.7-8 | Gros symboles lisibles au fond de la classe, animation du courant, un clic pour ouvrir/fermer l'interrupteur, mode plein écran sans distraction |
| U2 | **TP en salle informatique** : les élèves vérifient U₁ + U₂ = U_G et I = I₁ + I₂ | 4e ch.8 | Circuits pré-montés à compléter, voltmètres/ampèremètres à placer eux-mêmes, tableau de relevés à remplir |
| U3 | **TP multimètre** sans risque de griller le matériel | 4e ch.7 | Multimètre complet (bornes, calibres, dépassement), messages d'erreur pédagogiques, « fusible grillé » réversible |
| U4 | **TP caractéristique U = f(I)** d'une résistance puis d'une lampe | 4e ch.9 / 3e ch.6 | Générateur réglable, tableau de mesures, tracé intégré, pente = R, **export CSV** vers LibreOffice Calc |
| U5 | **Puissance et énergie** : P = U × I, E = P × t, coût en kWh | 3e ch.7 | Affichage P sur chaque dipôle, compteur d'énergie, prix du kWh paramétrable |
| U6 | **Remédiation / approfondissement** (chaque cours en prévoit) | 5e→3e | Exercices guidés avec vérification automatique + circuits « bac à sable » ouverts |
| U7 | **Révisions DNB à la maison** | 3e | Version web sur GitHub Pages, sans installation, sans compte |
| U8 | **Préparation de mes documents** | prof | Export **PNG/SVG** du schéma normalisé pour l'insérer directement dans mes cours Markdown/Obsidian |

---

## 4. Couverture du programme — traçabilité chapitre → fonction

| Chapitre du cours | Notions | Fonction CrocElec exigée |
|---|---|---|
| **6e ch.13** Électricité et sécurité | conducteur/isolant, circuit simple, dangers | Test conducteur/isolant (matériau à insérer dans le circuit), scénarios de danger commentés |
| **5e ch.7** Circuit électrique simple | générateur, récepteur, interrupteur, boucle, sens conventionnel, symboles | Palette de symboles normalisés, circuit ouvert/fermé, animation du sens conventionnel **et** du sens réel des électrons (bascule) |
| **5e ch.8** Série / dérivation | nœud, branche, unicité et additivité, panne | Détection automatique série/dérivation, surlignage des nœuds et des branches, **mode panne** (une lampe grillée → observer l'effet) |
| **4e ch.6** Rappels | symboles, sens du courant, court-circuit | Alerte court-circuit explicite (« pile en court-circuit → échauffement, incendie ») |
| **4e ch.7** Multimètre | voltmètre en dérivation, ampèremètre en série, ohmmètre hors circuit, calibres, bornes, sécurité | **Multimètre simulé fidèle** (§ 6.3), refus/effets réalistes des mauvais branchements |
| **4e ch.8** Lois de l'électricité | U_G = U₁ + U₂ ; U_G = U₁ = U₂ ; I = I₁ + I₂ ; unicité de I | Panneau « Lois » vérifiant en direct les sommes, circuits de référence du TP PhET rejouables |
| **4e ch.9** Résistance et loi d'Ohm | U = R × I, triangle URI, ohmmètre, code couleurs, caractéristique | Résistance à code couleurs, ohmmètre, tracé U = f(I), pente = R |
| **4e ch.10** Applications | conversions mA↔A, cas concrets, P = U × I | Affichage double unité (0,25 A / 250 mA), circuits « machine à laver / smartphone / bouilloire » |
| **3e ch.6** Loi d'Ohm | 3 formes, kΩ↔Ω, coefficient directeur, conducteur ohmique vs non ohmique | Comparaison côte à côte résistance (droite) / lampe (courbe) |
| **3e ch.7** Puissance et énergie | P = U × I, E = P × t, kWh, coût | Wattmètre, compteur d'énergie, calcul de facture |

> **Règle de recette générale** : chaque valeur numérique affichée par CrocElec doit coïncider, aux arrondis du cours près, avec le résultat que l'élève obtient au tableau par les lois du programme.

---

## 5. Principes didactiques imposés

Ces règles priment sur toute considération technique ou esthétique.

1. **Double vue, schéma normalisé par défaut.** Le circuit existe en deux représentations du même montage : la **vue schéma** (symboles NF C 03-201 tels que dessinés dans le cahier), affichée par défaut, et la **vue paillasse** (pile, douilles, fils souples) obtenue par une bascule. Monter en paillasse puis basculer en schéma est l'exercice de traduction que demande le programme (5e ch.7).
2. **Notation française** : virgule décimale (7,26 V), espace comme séparateur de milliers (1 540 Ω), unités toujours affichées.
3. **Unités du niveau** : bascule mA ↔ A et Ω ↔ kΩ visible en un clic, car la conversion est l'erreur n°1 du cours.
4. **Chiffres significatifs raisonnables** : 3 chiffres par défaut, comme un multimètre 3½ digits.
5. **Sens conventionnel du courant par défaut** (du + vers le − à l'extérieur du générateur), avec option « montrer les électrons » qui va en sens inverse — l'erreur fréquente citée en 5e ch.7 et 4e ch.6.
6. **Vocabulaire du cours** : nœud, branche, branche principale, branche dérivée, dipôle, générateur, récepteur, conducteur ohmique.
7. **Aucun contenu hors programme imposé** : condensateurs, bobines, alternatif, transistors restent masqués par défaut (voir § 12, mode « Lycée »).
8. **L'erreur est autorisée et commentée** : brancher un ampèremètre en dérivation doit « griller le fusible » avec un message qui explique, jamais un simple refus silencieux.

---

## 6. Périmètre fonctionnel

### 6.1 Éditeur de schéma

- Canevas quadrillé, **placement magnétique** sur la grille ; zoom (molette) et déplacement (clic milieu / espace).
- **Bibliothèque arborescente** de dipôles par catégorie (générateurs, récepteurs, commande, mesure, matériaux), avec **filtre par niveau** (5e / 4e / 3e / tout) — glisser-déposer depuis l'arbre vers la scène.
- **Bascule vue schéma / vue paillasse** (§ 5.1) sur le même circuit, sans le redessiner, avec position des composants conservée.
- Pose d'un dipôle par clic ou glisser-déposer ; rotation 90° (R), miroir, suppression (Suppr).
- **Fils** : tracé orthogonal, jonction automatique aux extrémités, création d'un **nœud** visible (point plein) dès que 3 fils se rejoignent.
- **Connexion directe** : deux dipôles posés bout à bout se relient sans qu'il soit nécessaire de tirer un fil (geste de Crocodile Clips) ; le fil reste disponible mais jamais obligatoire.
- Sélection multiple, déplacement en bloc, copier/coller, **annuler/refaire illimité**.
- Double-clic sur un dipôle → panneau Propriétés (valeur, nom, tolérance, état).
- Contrôle de cohérence permanent : boucle non fermée, dipôle en l'air, deux dipôles superposés → signalés visuellement, non bloquants.
- **Étiquetage automatique** : L1, L2, R1, R2, G, K1… conforme aux usages du cahier ; renommable.

### 6.2 Bibliothèque de dipôles et modèles physiques

| Dipôle | Niveau | Paramètres | Modèle de simulation |
|---|---|---|---|
| **Pile / générateur continu** | 5e | f.é.m. E (1,5 / 4,5 / 6 / 9 / 12 V, libre), résistance interne r (0 par défaut) | source de tension idéale + r série |
| **Générateur réglable (alimentation de labo)** | 4e | 0–12 V au curseur, limite de courant | idem, E pilotable en direct |
| **Fil de connexion** | 5e | R = 0 (option : R non nulle pour l'exercice « fil de 100 m ») | conducteur parfait ou résistance |
| **Interrupteur** | 5e | ouvert / fermé, cliquable **en cours de simulation** | R ≈ 0 / circuit ouvert |
| **Lampe** | 5e | tension et puissance nominales (ex. 6 V / 0,1 A), R à froid / à chaud | **non ohmique** : R croît avec la puissance dissipée (justifie la courbe non linéaire du 3e ch.6). Rendu : éclat proportionnel à P, grille si P > 1,5 × P_nom |
| **Moteur** | 5e | tension nominale | résistance + force contre-électromotrice simplifiée ; rotation animée |
| **Résistance (conducteur ohmique)** | 4e | R en Ω/kΩ, **code couleurs** affiché et modifiable par bandes | U = R × I strict |
| **Résistance variable / potentiomètre** | 4e | R max, curseur | résistance pilotable en direct |
| **Photorésistance / thermistance** | 4e (bonus) | grandeur d'entrée au curseur | résistance pilotée par le curseur (illustre « capteur ») |
| **Diode** | 5e | sens | seuil ~0,7 V, blocage inverse |
| **DEL (LED)** | 5e | couleur, seuil (~1,8–2 V), I nominal 20 mA | seuil + résistance dynamique ; s'allume si I > seuil, **grille** si I > I_max (exercice DEL + résistance de protection du 4e ch.9) |
| **Fusible** | 6e/4e | I_max | ouvre le circuit au-delà, réarmable |
| **Buzzer / sonnette** | 5e | — | résistance + retour visuel |
| **Matériau test (conducteur/isolant)** | 6e | choix : cuivre, plastique, graphite, eau salée, bois, verre | résistance associée, très grande pour les isolants |
| **Prise secteur 230 V** | 3e | 230 V, disjoncteur | pour les exercices « machine à laver / bouilloire », avec bandeau d'avertissement danger |

### 6.3 Instruments de mesure

**Deux modes**, commutables par le professeur :

- **Mode simplifié (5e)** : voltmètre et ampèremètre idéaux, affichage direct, branchement guidé.
- **Mode multimètre (4e/3e)** : appareil complet, **exigence forte du chapitre 4e ch.7** :
  - trois bornes **COM (noir)**, **V/Ω (rouge)**, **10A** ;
  - sélecteur rotatif : V⎓ 2 / 20 / 200 / 1000 ; A 2 mA / 20 mA / 200 mA / 10 A ; Ω 200 / 2k / 20k ;
  - affichage 3½ digits, **« 1. » en cas de dépassement de calibre** ;
  - **voltmètre en série** → mesure faussée / quasi nulle, message explicatif ;
  - **ampèremètre en dérivation** → court-circuit, fusible grillé, message explicatif, réarmement d'un clic ;
  - **ohmmètre en circuit alimenté** → valeur aberrante + rappel « hors circuit, courant coupé » ;
  - résistances internes non idéales activables (voltmètre 10 MΩ, ampèremètre 0,1 Ω) pour l'approfondissement.
- **Wattmètre** et **compteur d'énergie** (3e ch.7) : P instantanée, E cumulée en J et en kWh, durée paramétrable, coût au tarif saisi.
- **Sondes rapides** : survoler un dipôle affiche U, I, R, P sans rien brancher (mode prof / mode « corrigé » — désactivable en mode élève).

### 6.4 Moteur de simulation

- **Régime continu** (le seul du programme) : point de fonctionnement résolu en permanence, ~60 fois par seconde.
- Méthode : **analyse nodale modifiée (MNA)** + itérations de Newton-Raphson pour les dipôles non linéaires (lampe, diode, DEL).
- Simulation **temps réel** : toute modification (interrupteur, curseur de générateur, potentiomètre) est prise en compte immédiatement, sans bouton « lancer ».
- Cas dégradés traités **pédagogiquement, jamais par un plantage** :
  - circuit ouvert → I = 0 partout, message « la boucle n'est pas fermée » ;
  - court-circuit du générateur → courant très élevé, alerte rouge, échauffement animé ;
  - absence de masse/référence → choix automatique d'un potentiel de référence ;
  - système non convergent → message « circuit impossible à résoudre » + surlignage de la zone fautive.
- **Détection topologique** utile au cours : identification automatique des **nœuds**, des **branches**, des dipôles **en série** et **en dérivation** ; affichage à la demande (surbrillance colorée).

### 6.5 Visualisation

- Animation du **sens du courant** par des porteurs (dots) se déplaçant à vitesse proportionnelle à I (plafonnée) ; bascule conventionnel / électrons.
- **Valeurs accrochées aux dipôles** : petit afficheur U / I collé au composant, comme dans Crocodile Clips, réglable par le professeur (tous / sélection / aucun) pour pouvoir évaluer sans souffler les réponses.
- **Potentiel en couleur** (option, façon Falstad) : dégradé sur les fils du + vers le −, pour visualiser « où est la tension ».
- **Épaisseur ou couleur** du fil proportionnelle à I (option), lampe dont l'éclat suit P.
- Étiquettes U et I affichables sur chaque dipôle (tout / sélection / aucun).
- **Panneau Lois** : pour le circuit courant, affichage vérifiable en direct
  - série : « I identique partout : 0,19 A », « U_G = U₁ + U₂ : 9,0 = 4,5 + 4,5 ✔ » ;
  - dérivation : « U identique : 9,0 V », « I = I₁ + I₂ : 0,30 = 0,18 + 0,12 ✔ ».
- **Thème clair haute lisibilité** pour vidéoprojecteur + taille de police globale réglable (jusqu'à ×2), palette compatible daltonisme.

### 6.6 Mesures, tableaux et exploitation

- **Tableau de relevés** : bouton « Relever » qui fige une ligne (U, I, R, P de chaque dipôle sélectionné + valeur du générateur).
- **Glisser une grandeur vers le graphique** : on attrape la valeur U ou I affichée sur un dipôle et on la lâche sur le panneau Graphique, qui la trace en direct (geste emprunté à Crocodile Clips) ; jusqu'à quatre courbes simultanées.
- **Tracé intégré** : U = f(I) (aussi I = f(U), P = f(U)), points expérimentaux, **droite de tendance et pente affichée = R**, indication « passe par l'origine → conducteur ohmique » ou « ne passe pas par l'origine / courbe → non ohmique ».
- **Export CSV** (séparateur point-virgule, virgule décimale) directement ouvrable dans LibreOffice Calc — le TP 4e ch.9 reste un TP tableur, CrocElec fournit les données.
- **Export PNG et SVG** du schéma seul (fond transparent) pour insertion dans les cours.
- **Copie presse-papier** du tableau au format Markdown, pour coller dans le vault Obsidian.

### 6.7 Sécurité, pannes et « effet wahou »

- Court-circuit de pile : échauffement, fumée, message « ne jamais relier le + au − ».
- Surintensité : fusible qui fond, disjoncteur qui déclenche, DEL qui grille, lampe qui grille.
- **Mode panne** : rendre un composant défaillant d'un clic droit (lampe grillée, fil coupé, pile usée) — support direct de l'activité « prédire l'effet d'une panne en série et en dérivation » (5e ch.8, différenciation).
- **Mode dépannage** : le logiciel introduit une panne cachée, l'élève doit la localiser au multimètre.
- Scénarios 230 V (3e) toujours accompagnés du bandeau de danger et jamais présentés comme reproductibles à la maison.

### 6.8 Activités guidées et évaluation

- **Mode leçon (annotations sur la scène)** : ajouter titres, textes, flèches et zones de commentaire sur le circuit, **verrouiller** tout ou partie des composants (valeur figée, déplacement interdit, suppression interdite) et distribuer le fichier aux élèves.
- Un **fichier d'activité** (texte + circuit de départ + questions + réponses attendues + tolérance) que je peux écrire moi-même, au format Markdown/JSON lisible.
- Types de questions : valeur numérique avec tolérance (ex. 1,80 V ± 0,05), choix multiple, « construis le circuit demandé » vérifié par comparaison topologique, « place correctement le voltmètre ».
- **Indices progressifs** et feedback rédigés par l'auteur de l'activité (je réutilise ceux de mes QCM H5P).
- **Mode élève** : palette restreinte, sondes de survol désactivées, circuit de départ non supprimable ; **mode prof** déverrouillé par un simple réglage (pas de mot de passe complexe).
- **Bilan de séance** exportable : réponses, essais, temps — en local, sans compte ni envoi réseau.

### 6.9 Bibliothèque de circuits fournis (livrés avec le logiciel)

Alignés sur mes chapitres, chacun avec ses valeurs attendues :

1. Pile + interrupteur + lampe (5e ch.7) — circuit ouvert/fermé.
2. Test conducteur/isolant (6e ch.13).
3. Deux lampes en série (5e ch.8) — même I partout, U partagée.
4. Deux lampes en dérivation (5e ch.8) — même U, I qui se divise.
5. Circuit mixte (série + dérivation) (4e ch.8, approfondissement).
6. **Pile 9 V + 5 lampes en série → 1,80 V chacune** (TP PhET de référence, 4e ch.8).
7. **Circuit mixte du TP : 4,50 V / 9,00 V / 3,00 V** (4e ch.8).
8. Mesure au multimètre : pile 4,5 V, I = 0,19 A = 190 mA (4e ch.7, photo du cours).
9. Trois résistances en série avec trois voltmètres (4e ch.7, photo du cours).
10. Caractéristique d'un conducteur ohmique (4e ch.9 / 3e ch.6) — table de valeurs du cours reproduite.
11. Caractéristique d'une lampe (3e ch.6, approfondissement) — courbe non linéaire.
12. DEL + résistance de protection : 9 V, DEL 2 V, 20 mA → **R = 350 Ω** (4e ch.9).
13. Résistance 270 Ω à code couleurs rouge/violet/marron/or (4e ch.9, photo du cours).
14. Applications 4e ch.10 : machine à laver (230 V, 5 A, 46 Ω), smartphone (5 V, 250 mA, 20 Ω), bouilloire (115 Ω, 2 A, 230 V).
15. Ventilateur 220 V / 1 kΩ → 0,22 A (3e ch.6).
16. Puissance et énergie (3e ch.7) : chargeur, cafetière, sèche-cheveux, avec coût en kWh.
17. Guirlande de Noël série vs dérivation, avec panne (5e ch.8).
18. Installation domestique simplifiée en dérivation (4e ch.6, approfondissement).

### 6.10 Fichiers

- Format natif **`.croc`** : JSON lisible et versionnable (composants, positions, valeurs, sondes, activité éventuelle).
- Ouverture par double-clic, glisser-déposer d'un fichier sur la fenêtre.
- **Compatibilité ascendante garantie** : un `.croc` v1 s'ouvre dans toutes les versions ultérieures.
- Exports : PNG, SVG, CSV, Markdown (tableau).
- Aucune écriture hors du dossier choisi par l'utilisateur ; aucun accès réseau.

---

## 7. Interface — organisation en panneaux dockés (Dear ImGui docking)

Panneaux :

| Panneau | Rôle |
|---|---|
| **Schéma** | canevas principal d'édition et de simulation (central, jamais fermé) |
| **Palette** | dipôles filtrés par niveau |
| **Propriétés** | paramètres du dipôle sélectionné (valeur, nom, état, panne) |
| **Mesures** | tableau U / I / R / P de tous les dipôles + relevés figés |
| **Multimètre** | appareil dessiné (bornes, sélecteur, afficheur) |
| **Graphique** | tracé U = f(I) et régression |
| **Lois** | vérification en direct des lois du niveau (§ 6.5) |
| **Activité** | énoncé, questions, indices, validation |
| **Journal** | messages du simulateur (alertes, erreurs de branchement, pannes) |
| **Bibliothèque** | circuits fournis, ouverts en un clic |

Dispositions (layouts) prédéfinies, changeables en un clic et mémorisées :

- **Cours** (vidéoprojection) : Schéma plein écran + Lois, grosse police, panneaux techniques masqués ;
- **TP mesures** : Schéma + Multimètre + Mesures ;
- **TP caractéristique** : Schéma + Mesures + Graphique ;
- **Élève** : Schéma + Activité + Palette restreinte ;
- **Libre** : l'utilisateur arrange comme il veut, la disposition est sauvegardée ;
- **Tablette** (imposée automatiquement sous 1 300 px de large, § 8.1) : un seul panneau plein écran à la fois, barre d'onglets tactile en bas, police agrandie, boutons ≥ 44 px — pas de fenêtres flottantes.

Ergonomie : tout en français, raccourcis affichés, barre d'outils explicite (icône + mot), aucune fonctionnalité cachée derrière un clic droit uniquement, réinitialisation de la disposition toujours accessible.

---

## 8. Exigences non fonctionnelles

| Domaine | Exigence |
|---|---|
| **Performance (bureau)** | 60 images/s avec 50 dipôles sur un PC de salle informatique ancien (GPU intégré, OpenGL 3.3) ; résolution du circuit < 5 ms |
| **Performance (tablette)** | **≥ 30 images/s avec 25 dipôles sur Galaxy Tab 4** (§ 8.1) — exigence dimensionnante, pas un objectif secondaire |
| **Démarrage** | fenêtre utilisable en moins de 2 s sur bureau ; page web jouable en moins de 10 s sur la tablette, Wi-Fi du collège |
| **Hors ligne** | 100 % des fonctions sans Internet ; aucune télémétrie, aucun compte ; la version web reste utilisable une fois chargée (cache applicatif) |
| **Plateformes** | macOS (Apple Silicon + Intel), Windows 10+, Linux ; **version web WASM** publiable sur GitHub Pages, utilisable au doigt sur tablette Android ancienne |
| **Encombrement** | binaire de bureau < 20 Mo ; **page web < 3 Mo compressés** (contrainte tablette + Wi-Fi partagé par 30 élèves) |
| **Données personnelles** | aucune collecte (RGPD par construction) ; les bilans restent des fichiers locaux |
| **Robustesse** | aucun plantage sur circuit aberrant ; sauvegarde de secours automatique du circuit en cours |
| **Langue** | français ; architecture prête pour une traduction ultérieure |
| **Accessibilité** | police jusqu'à ×2, contrastes élevés, palette lisible en cas de daltonisme, tout au clavier autant que possible |
| **Licence** | libre (MIT ou GPL) pour partage entre collègues |

### 8.1 Cible matérielle basse : Galaxy Tab 4 (2014)

La version web doit tourner **sur les tablettes du collège** : Samsung Galaxy Tab 4 — Snapdragon 400 (4 cœurs Cortex-A7 à 1,2 GHz), GPU Adreno 305, **1,5 Go de RAM**, Android 4.4 à 5.0, écran 1 280 × 800. C'est la machine de référence : **si ça tourne là, ça tourne partout.**

| Contrainte de la machine | Conséquence sur le logiciel |
|---|---|
| Chrome figé en version ancienne (81 sur Android 4.4, 106 sur Android 5) | WebAssembly **de base uniquement** : pas de SIMD, pas de `BigInt`, JavaScript de collage compilé en ES5 |
| GitHub Pages ne peut pas envoyer les en-têtes COOP/COEP | Pas de `SharedArrayBuffer` → **simulateur strictement mono-thread**, aucun *worker* de calcul |
| Pilote graphique OpenGL ES 2.0 | Rendu **WebGL 1** obligatoire (backend ImGui GLSL ES 100), pas d'ES 3, pas de VAO ; géométrie limitée à quelques milliers de triangles par image |
| 1,5 Go de RAM partagés avec Android | Tas WebAssembly plafonné à **128 Mo**, pas de croissance sauvage, aucune texture inutile |
| Quatre cœurs A7 lents, sans SIMD | Solveur dimensionné pour **≤ 60 dipôles**, matrice dense en `double` mais petite ; recalcul **à l'événement** (et au plus 20 fois par seconde en régime animé), pas 60 fois par seconde |
| Écran tactile sans survol | **Aucune fonction accessible au seul survol** : les sondes s'obtiennent par appui long ou sélection ; cibles tactiles ≥ 44 px ; pincement pour zoomer, deux doigts pour déplacer |
| Écran 1 280 × 800 sans HiDPI | **Disposition « Tablette »** dédiée : un panneau plein écran à la fois, barre d'onglets en bas, police agrandie — le dock multi-fenêtres reste réservé au bureau |
| Wi-Fi de collège saturé | Téléchargement initial < 3 Mo compressés, police et circuits chargés à la demande, mise en cache pour les visites suivantes |
| Batterie et chauffe | Boucle de rendu **à la demande** (on ne redessine pas si rien ne bouge), animation du courant à 30 images/s maximum |

Repli explicite : si le navigateur n'a ni WebAssembly ni WebGL, la page affiche un message clair en français et le lien vers la version de bureau — jamais un écran blanc.

---

## 9. Architecture technique proposée

- **C++17**, **Dear ImGui branche `docking`** épinglée à un commit (fichier `imgui_pin.env`, comme pom2), **GLFW + OpenGL 3**, **CMake ≥ 3.16**.
- Pas de dépendance lourde : solveur linéaire (LU avec pivot partiel) écrit dans le projet ; tracés dessinés au `ImDrawList` ; JSON minimal interne.
- **Build web** : Emscripten (emsdk déjà présent sur la machine), même code source, sortie `index.html` publiable sur GitHub Pages. Réglages imposés par la cible tablette (§ 8.1) :
  - `-Os` (taille avant vitesse : le téléchargement coûte plus cher que le calcul sur ce circuit), `--closure 1` ;
  - `-sMIN_WEBGL_VERSION=1 -sMAX_WEBGL_VERSION=1` + backend ImGui compilé en **GLSL ES 100** (`IMGUI_IMPL_OPENGL_ES2`) ;
  - `-sINITIAL_MEMORY=64MB -sMAXIMUM_MEMORY=128MB -sALLOW_MEMORY_GROWTH=1` ;
  - `-sMIN_CHROME_VERSION=57` (sortie JavaScript ES5, pas de `BigInt`), pas de *pthreads*, pas de SIMD ;
  - `-sASSERTIONS=0 -sFILESYSTEM=0` en production, circuits embarqués en JSON compressé ;
  - police unique embarquée en sous-ensemble (latin + symboles électriques nécessaires), pas d'atlas géant.
- **Boucle de rendu à la demande** (`ImGui` ne redessine que si une entrée ou la simulation a changé) et **solveur découplé du rendu** : c'est ce qui rend les 30 images/s tenables sur Cortex-A7.
- **Tests de performance automatisés** : temps de résolution mesuré sur les circuits de la bibliothèque, seuil d'échec si un circuit de 25 dipôles dépasse 3 ms sur la machine de développement (marge ×10 pour la tablette).
- Découpage :

```
src/
  main.cpp            fenêtre, boucle, DockSpace
  App.*               état global, layouts, menus
  Circuit.*           dipôles, fils, extraction de la netlist (nœuds, branches)
  Models.*            modèles physiques des dipôles (§ 6.2)
  Solver.*            MNA + Newton-Raphson + LU
  Topology.*          détection série / dérivation / nœuds
  Schematic.*         canevas, symboles normalisés, édition
  Instruments.*       multimètre, wattmètre, compteur d'énergie
  Panels.*            Mesures, Graphique, Lois, Journal, Bibliothèque
  Activity.*          activités guidées, validation, bilan
  Serialize.*         .croc, CSV, PNG/SVG, Markdown
tests/                tests du solveur (valeurs attendues du § 11)
circuits/             bibliothèque livrée (§ 6.9)
activites/            activités guidées
```

- **Tests automatisés** du solveur : chaque circuit de la bibliothèque a ses valeurs attendues ; la suite échoue si un écart dépasse la tolérance.

---

## 10. Lots de livraison

| Lot | Contenu | Cible |
|---|---|---|
| **V0 — socle** | fenêtre dockée, canevas, pile/fil/interrupteur/lampe, solveur continu, animation du courant, ouvert/fermé | démontrable en cours de 5e ch.7 |
| **V1 — 5e complet** | série/dérivation, nœuds/branches surlignés, **double vue schéma ⟷ paillasse**, voltmètre et ampèremètre simplifiés, panneau Lois, mode panne, circuits 1 à 5 et 17 | remplace PhET en 5e |
| **V2 — 4e complet + tablette** | résistance + code couleurs, loi d'Ohm, **multimètre complet avec calibres et erreurs de branchement**, DEL + résistance de protection, tableau de relevés, export CSV, circuits 6 à 14 — **et le premier build WASM validé sur Galaxy Tab 4** (§ 8.1) | remplace PhET en 4e, couvre le TP multimètre, tourne sur les tablettes |
| **V3 — 3e complet** | générateur réglable, glisser-déposer d'une grandeur vers le graphique, caractéristique U = f(I) avec régression, lampe non ohmique, wattmètre, énergie et coût kWh, circuits 15 à 16 | couvre 3e ch.6 et ch.7 |
| **V4 — pédagogie** | activités guidées, mode leçon (annotations, verrouillage), mode élève/prof, mode dépannage, bilan de séance, exports PNG/SVG | usage en autonomie et différenciation |
| **V5 — diffusion** | publication GitHub Pages, fiche prof, fiches élèves, paquets macOS/Windows/Linux | partage avec les collègues et les familles |

---

## 11. Critères d'acceptation (vérifiables)

Le logiciel est accepté si, sans intervention manuelle :

1. Pile 9 V + 5 lampes identiques en série → **1,80 V** aux bornes de chaque lampe (± 0,01 V).
2. Circuit mixte du TP 4e ch.8 → **4,50 V / 9,00 V / 3,00 V**.
3. R = 60 Ω sous 6 V → **I = 0,1 A**, affiché aussi comme **100 mA**.
4. U = 5 V, I = 250 mA → l'ohmmètre lit **20 Ω**.
5. R = 115 Ω, I = 2 A → **U = 230 V** ; wattmètre → **2 300 W** pour le fer à repasser (R = 23 Ω sous 230 V).
6. R = 1 kΩ sous 220 V → **0,22 A**.
7. DEL 2 V / 20 mA sur pile 9 V avec R = 350 Ω → I = **20 mA** ; avec R = 0 → DEL grillée + message.
8. Deux lampes en dérivation sur 9 V → U identique sur les deux branches, **I = I₁ + I₂** vérifié à 1 % près ; retirer une lampe n'éteint pas l'autre ; en série, la retirer éteint tout.
9. Ampèremètre branché en dérivation → fusible grillé + message pédagogique, aucune donnée perdue, réarmement en un clic.
10. Calibre 2 V sélectionné pour une mesure de 12,8 V → affichage **« 1. »**.
11. Tableau de 8 relevés sur une résistance → droite passant par l'origine, pente affichée **= R à 2 % près** ; les mêmes 8 relevés sur une lampe → courbe, message « non ohmique ».
12. Export CSV rouvert dans LibreOffice Calc : colonnes I et U exploitables telles quelles (virgule décimale reconnue).
13. Un circuit `.croc` enregistré puis rouvert est strictement identique (composants, valeurs, positions, sondes).
14. 60 images/s maintenues sur un circuit de 50 dipôles ; aucun plantage sur 30 min de manipulation aléatoire par des élèves.
15. La version web offre les mêmes fonctions que la version de bureau, hors accès aux fichiers du disque (téléchargement à la place).
16. **Sur une Galaxy Tab 4 réelle** : un circuit de 25 dipôles reste manipulable à **30 images/s au doigt**, sans saccade à l'ouverture/fermeture d'un interrupteur.
17. **Sur la même tablette** : page chargée et jouable en moins de 10 s sur le Wi-Fi du collège, puis **utilisable hors ligne** après la première visite ; total téléchargé < 3 Mo compressés.
18. **Aucune fonction inaccessible au doigt** : un élève sans souris peut poser un composant, le relier, ouvrir l'interrupteur, lire U et I, changer un calibre du multimètre et relever une mesure.
19. La bascule **vue schéma ⟷ vue paillasse** conserve le montage, les valeurs et les mesures à l'identique, dans les deux sens.

---

## 12. Hors périmètre (v1 → v5)

Volontairement exclus du programme de collège, mais l'architecture doit pouvoir les accueillir plus tard sous un mode **« Lycée »** masqué par défaut : courant alternatif et oscilloscope, condensateurs et bobines (régimes transitoires RC/RL), transistors et amplificateurs, logique numérique, triphasé, simulation SPICE avancée (analyse fréquentielle, Monte-Carlo).

Également hors périmètre : compte utilisateur en ligne, sauvegarde cloud, classe virtuelle, notation transmise à l'ENT.

---

## 13. Risques et points à arbitrer

| # | Point | Options | Proposition |
|---|---|---|---|
| A | Priorité bureau ou web | bureau d'abord / web d'abord / les deux en parallèle | **Bureau d'abord** (ImGui natif), build web dès la V2 — le même code, un simple `build_wasm.sh` |
| B | Vue « paillasse » réaliste | bonus tardif / fonction centrale | **Fonction centrale dès la V1** (révisé après examen de Crocodile Clips) : c'est la bascule paillasse → schéma qui fait travailler la compétence, le schéma normalisé restant l'affichage par défaut |
| C | Fidélité du multimètre | idéal simplifié / fidèle avec calibres | **Fidèle** dès la V2 : c'est tout l'objet du chapitre 4e ch.7 |
| D | Modèle de la lampe | résistance constante / non ohmique | **Non ohmique** : sans cela l'approfondissement du 3e ch.6 est impossible |
| E | Format des activités | interne / Markdown lisible | **Markdown + JSON** pour réutiliser mes énoncés et feedbacks H5P |
| F | Élèves : où stockent-ils leur travail ? | fichier local / navigateur / rien | **Fichier local + téléchargement web**, aucune donnée personnelle |
| G | Nom du logiciel | CrocElec / autre | à confirmer (référence à Crocodile Clips, à préciser côté droit des marques) |
| H | Niveau de fidélité de la vue paillasse | dessin vectoriel stylisé / photos de matériel | **Dessin vectoriel stylisé** : identifiable au vidéoprojecteur, léger pour la tablette, et modifiable sans séance photo |
| I | Cible tablette | tablettes du collège seulement / tout appareil récent | **Galaxy Tab 4 comme plancher** (§ 8.1) : elle impose WebGL 1, mono-thread et 128 Mo, ce qui garantit que tout le reste suit |
| J | Élèves sur tablette : combien de dipôles ? | même limite que le bureau / limite réduite | **Limite réduite affichée** (≈ 25 dipôles) avec avertissement clair plutôt qu'une lenteur inexpliquée |

---

## 14. Suite immédiate

1. Validation de ce cahier des charges (et arbitrage des points § 13).
2. Réalisation du **V0** : socle ImGui docké + solveur + pile/interrupteur/lampe, à montrer en 5e.
3. Recette du V0 sur les critères 8 et 14, puis enchaînement V1.
