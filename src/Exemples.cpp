// Exemples.cpp — la bibliothèque de montages livrée avec le logiciel.
//
// Chaque entrée reprend un circuit de mes cours, avec la valeur que l'élève
// doit retrouver (§ 6.9 du cahier des charges). Ces montages servent aussi de
// recette : si l'un d'eux n'affiche plus la valeur annoncée, le solveur a
// régressé.
#include "Exemples.h"

#include "Circuit.h"

namespace croc {
namespace {

Elem faire(Kind k, Pt a, Pt b, double v = 0, double v2 = 0)
{
    Elem e;
    e.kind = k;
    e.a = a;
    e.b = b;
    e.valeur = v;
    e.valeur2 = v2;
    return e;
}

// Boucle rectangulaire de base : pile à gauche, interrupteur en haut,
// récepteur à droite. Rend les points utiles pour compléter le montage.
void cadre(Circuit& c, double tensionPile, int largeur, int hauteur)
{
    c.ajouter(faire(Kind::Pile, {0, hauteur / 2 + 1}, {0, hauteur / 2 - 1}, tensionPile, 0.0));
    c.ajouter(faire(Kind::Fil, {0, hauteur / 2 - 1}, {0, 0}));
    c.ajouter(faire(Kind::Fil, {0, hauteur / 2 + 1}, {0, hauteur}));
    c.ajouter(faire(Kind::Fil, {0, hauteur}, {largeur, hauteur}));
}

void circuitSimple(Circuit& c)
{
    cadre(c, 4.5, 10, 6);
    c.ajouter(faire(Kind::Fil, {0, 0}, {3, 0}));
    c.ajouter(faire(Kind::Interrupteur, {3, 0}, {6, 0}));
    c.ajouter(faire(Kind::Fil, {6, 0}, {10, 0}));
    c.ajouter(faire(Kind::Lampe, {10, 0}, {10, 3}, 6.0, 0.6));
    c.ajouter(faire(Kind::Fil, {10, 3}, {10, 6}));
}

void deuxLampesSerie(Circuit& c)
{
    cadre(c, 9.0, 14, 6);
    c.ajouter(faire(Kind::Fil, {0, 0}, {2, 0}));
    c.ajouter(faire(Kind::Lampe, {2, 0}, {6, 0}, 6.0, 0.6));
    c.ajouter(faire(Kind::Fil, {6, 0}, {8, 0}));
    c.ajouter(faire(Kind::Lampe, {8, 0}, {12, 0}, 6.0, 0.6));
    c.ajouter(faire(Kind::Fil, {12, 0}, {14, 0}));
    c.ajouter(faire(Kind::Fil, {14, 0}, {14, 6}));
}

void deuxLampesDerivation(Circuit& c)
{
    cadre(c, 9.0, 12, 6);
    c.ajouter(faire(Kind::Fil, {0, 0}, {6, 0}));
    c.ajouter(faire(Kind::Fil, {6, 0}, {12, 0}));
    c.ajouter(faire(Kind::Lampe, {6, 0}, {6, 3}, 6.0, 0.6));
    c.ajouter(faire(Kind::Fil, {6, 3}, {6, 6}));
    c.ajouter(faire(Kind::Lampe, {12, 0}, {12, 3}, 6.0, 0.6));
    c.ajouter(faire(Kind::Fil, {12, 3}, {12, 6}));
}

// TP de référence du 4e ch.8 : pile 9 V et cinq lampes identiques en série.
// Attendu : 1,80 V aux bornes de chacune.
void cinqLampesSerie(Circuit& c)
{
    c.ajouter(faire(Kind::Pile, {0, 5}, {0, 3}, 9.0, 0.0));
    c.ajouter(faire(Kind::Fil, {0, 3}, {0, 0}));
    c.ajouter(faire(Kind::Fil, {0, 5}, {0, 8}));
    c.ajouter(faire(Kind::Fil, {0, 8}, {21, 8}));
    c.ajouter(faire(Kind::Fil, {21, 8}, {21, 0}));

    int x = 0;
    for (int k = 0; k < 5; ++k) {
        c.ajouter(faire(Kind::Fil, {x, 0}, {x + 1, 0}));
        c.ajouter(faire(Kind::Lampe, {x + 1, 0}, {x + 4, 0}, 6.0, 0.6));
        x += 4;
    }
    c.ajouter(faire(Kind::Fil, {20, 0}, {21, 0}));
}

// 4e ch.9 : mesurer U et I aux bornes d'un conducteur ohmique.
void loiDOhm(Circuit& c)
{
    c.ajouter(faire(Kind::Pile, {0, 5}, {0, 3}, 6.0, 0.0));
    c.ajouter(faire(Kind::Fil, {0, 3}, {0, 0}));
    c.ajouter(faire(Kind::Fil, {0, 0}, {2, 0}));
    c.ajouter(faire(Kind::Amperemetre, {2, 0}, {5, 0}));
    c.ajouter(faire(Kind::Fil, {5, 0}, {7, 0}));
    c.ajouter(faire(Kind::Resistance, {7, 0}, {11, 0}, 60.0));
    c.ajouter(faire(Kind::Fil, {11, 0}, {13, 0}));
    c.ajouter(faire(Kind::Fil, {13, 0}, {13, 8}));
    c.ajouter(faire(Kind::Fil, {13, 8}, {0, 8}));
    c.ajouter(faire(Kind::Fil, {0, 8}, {0, 5}));
    // Voltmètre en dérivation, aux bornes de la résistance.
    c.ajouter(faire(Kind::Fil, {7, 0}, {7, -3}));
    c.ajouter(faire(Kind::Voltmetre, {7, -3}, {11, -3}));
    c.ajouter(faire(Kind::Fil, {11, -3}, {11, 0}));
}

// 4e ch.9 : la DEL a besoin d'une résistance de protection.
// 9 V, DEL 2 V, 20 mA → R = (9 − 2) / 0,020 = 350 Ω.
void delProtegee(Circuit& c)
{
    c.ajouter(faire(Kind::Pile, {0, 5}, {0, 3}, 9.0, 0.0));
    c.ajouter(faire(Kind::Fil, {0, 3}, {0, 0}));
    c.ajouter(faire(Kind::Fil, {0, 0}, {2, 0}));
    c.ajouter(faire(Kind::Resistance, {2, 0}, {6, 0}, 350.0));
    c.ajouter(faire(Kind::Fil, {6, 0}, {8, 0}));
    c.ajouter(faire(Kind::Del, {8, 0}, {11, 0}, 2.0, 0.03));
    c.ajouter(faire(Kind::Fil, {11, 0}, {13, 0}));
    c.ajouter(faire(Kind::Fil, {13, 0}, {13, 8}));
    c.ajouter(faire(Kind::Fil, {13, 8}, {0, 8}));
    c.ajouter(faire(Kind::Fil, {0, 8}, {0, 5}));
}

// 4e ch.7 : le montage de la photo du cours — pile 4,5 V, ampèremètre en
// série, l'écran affiche 0,19 A = 190 mA.
void mesureAuMultimetre(Circuit& c)
{
    c.ajouter(faire(Kind::Pile, {0, 5}, {0, 3}, 4.5, 0.0));
    c.ajouter(faire(Kind::Fil, {0, 3}, {0, 0}));
    c.ajouter(faire(Kind::Fil, {0, 0}, {2, 0}));

    Elem mm = faire(Kind::Multimetre, {2, 0}, {6, 0});
    mm.mode = ModeMultimetre::Intensite;
    mm.borne = BorneMultimetre::DixAmperes;
    mm.calibre = 10.0;
    c.ajouter(mm);

    c.ajouter(faire(Kind::Fil, {6, 0}, {9, 0}));
    c.ajouter(faire(Kind::Lampe, {9, 0}, {12, 0}, 4.5, 0.85));   // 23,8 ohm à chaud
    c.ajouter(faire(Kind::Fil, {12, 0}, {14, 0}));
    c.ajouter(faire(Kind::Fil, {14, 0}, {14, 8}));
    c.ajouter(faire(Kind::Fil, {14, 8}, {0, 8}));
    c.ajouter(faire(Kind::Fil, {0, 8}, {0, 5}));
}

// 4e ch.9 / 3e ch.6 : le TP « caractéristique ». Le générateur se règle dans
// Propriétés ; chaque réglage donne un point à relever.
void caracteristique(Circuit& c)
{
    c.ajouter(faire(Kind::Pile, {0, 5}, {0, 3}, 2.0, 0.0));
    c.ajouter(faire(Kind::Fil, {0, 3}, {0, 0}));
    c.ajouter(faire(Kind::Fil, {0, 0}, {2, 0}));
    c.ajouter(faire(Kind::Amperemetre, {2, 0}, {5, 0}));
    c.ajouter(faire(Kind::Fil, {5, 0}, {7, 0}));
    c.ajouter(faire(Kind::Resistance, {7, 0}, {11, 0}, 82.0));
    c.ajouter(faire(Kind::Fil, {11, 0}, {13, 0}));
    c.ajouter(faire(Kind::Fil, {13, 0}, {13, 8}));
    c.ajouter(faire(Kind::Fil, {13, 8}, {0, 8}));
    c.ajouter(faire(Kind::Fil, {0, 8}, {0, 5}));
    c.ajouter(faire(Kind::Fil, {7, 0}, {7, -3}));
    c.ajouter(faire(Kind::Voltmetre, {7, -3}, {11, -3}));
    c.ajouter(faire(Kind::Fil, {11, -3}, {11, 0}));
}

// 4e ch.6 : la démonstration qu'on ne peut pas faire en vrai.
void courtCircuit(Circuit& c)
{
    circuitSimple(c);
    c.ajouter(faire(Kind::Fil, {0, 0}, {0, 6}));   // le + relié au − : voilà
}

// 5e ch.8 : la guirlande. Une lampe grille, tout s'éteint.
void guirlandeSerie(Circuit& c)
{
    c.ajouter(faire(Kind::Pile, {0, 5}, {0, 3}, 12.0, 0.0));
    c.ajouter(faire(Kind::Fil, {0, 3}, {0, 0}));
    c.ajouter(faire(Kind::Fil, {0, 5}, {0, 8}));
    c.ajouter(faire(Kind::Fil, {0, 8}, {25, 8}));
    c.ajouter(faire(Kind::Fil, {25, 8}, {25, 0}));
    int x = 0;
    for (int k = 0; k < 6; ++k) {
        c.ajouter(faire(Kind::Fil, {x, 0}, {x + 1, 0}));
        c.ajouter(faire(Kind::Lampe, {x + 1, 0}, {x + 4, 0}, 2.0, 0.2));
        x += 4;
    }
    c.ajouter(faire(Kind::Fil, {24, 0}, {25, 0}));
}

const std::vector<Exemple> kExemples = {
    {"Circuit simple : pile, interrupteur, lampe", "5e ch.7 — Le circuit électrique simple",
     "Interrupteur fermé : la lampe s'allume. Ouvert : plus rien ne circule.", circuitSimple},

    {"Deux lampes en série", "5e ch.8 — Circuits série et dérivation",
     "Même intensité partout, la tension de 9 V se partage : 4,50 V par lampe.", deuxLampesSerie},

    {"Deux lampes en dérivation", "5e ch.8 — Circuits série et dérivation",
     "Même tension de 9 V sur chaque branche, et I = I1 + I2 au nœud.", deuxLampesDerivation},

    {"Pile 9 V et cinq lampes en série", "4e ch.8 — Lois de l'électricité (TP de référence)",
     "1,80 V aux bornes de chaque lampe.", cinqLampesSerie},

    {"Loi d'Ohm : ampèremètre en série, voltmètre en dérivation", "4e ch.9 — Résistance et loi d'Ohm",
     "R = 60 ohm sous 6 V : I = 0,1 A = 100 mA.", loiDOhm},

    {"DEL avec résistance de protection", "4e ch.9 — Résistance et loi d'Ohm",
     "9 V, DEL 2 V, R = 350 ohm : I = 20 mA. Retire la résistance et la DEL grille.", delProtegee},

    {"Mesure au multimètre", "4e ch.7 — Utilisation du multimètre",
     "Ampèremètre en série, borne 10 A : l'écran affiche 0,19 A = 190 mA. Passe le sélecteur sur V "
     "sans changer de borne, ou branche-le en dérivation, et le fusible fond.", mesureAuMultimetre},

    {"Caractéristique U = f(I) d'un conducteur ohmique", "4e ch.9 / 3e ch.6 — TP tableur",
     "Règle la pile dans Propriétés, clique « Relever » à chaque valeur, puis lis le coefficient "
     "directeur dans Graphique : il vaut 82 ohm. Remplace la résistance par une lampe : la courbe "
     "n'est plus une droite.", caracteristique},

    {"Guirlande en série (6 lampes)", "5e ch.8 — Différenciation : la panne",
     "Fais griller une lampe : toute la guirlande s'éteint.", guirlandeSerie},

    {"Court-circuit de la pile", "4e ch.6 — Rappels : danger",
     "Le fil relie le + au − : la pile débite un courant énorme et la lampe s'éteint.", courtCircuit},
};

} // namespace

const std::vector<Exemple>& exemples() { return kExemples; }

} // namespace croc
