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

Elem nomme(Elem e, const char* nom)
{
    e.nom = nom;
    return e;
}

Elem ouvert(Elem e)
{
    e.ferme = false;
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

// 6e ch.13 : le matériau à tester est pris entre deux pinces crocodiles.
void conducteurIsolant(Circuit& c)
{
    cadre(c, 4.5, 12, 6);
    c.ajouter(faire(Kind::Fil, {0, 0}, {3, 0}));
    c.ajouter(faire(Kind::Materiau, {3, 0}, {7, 0}, 0));   // cuivre pour commencer
    c.ajouter(faire(Kind::Fil, {7, 0}, {12, 0}));
    c.ajouter(faire(Kind::Lampe, {12, 0}, {12, 3}, 6.0, 0.6));
    c.ajouter(faire(Kind::Fil, {12, 3}, {12, 6}));
}

// 6e ch.13 / 4e ch.6 : le fusible protège la pile du court-circuit.
void fusibleProtection(Circuit& c)
{
    cadre(c, 4.5, 10, 6);
    c.ajouter(faire(Kind::Fusible, {0, 0}, {3, 0}, 1.0));
    c.ajouter(faire(Kind::Interrupteur, {3, 0}, {6, 0}));
    c.ajouter(faire(Kind::Fil, {6, 0}, {10, 0}));
    c.ajouter(faire(Kind::Lampe, {10, 0}, {10, 3}, 6.0, 0.6));
    c.ajouter(faire(Kind::Fil, {10, 3}, {10, 6}));
    // K2, ouvert : le fermer relie les deux fils de la lampe. Posé loin
    // d'elle, pour que leurs étiquettes ne se chevauchent pas.
    c.ajouter(faire(Kind::Fil, {6, 0}, {6, 2}));
    c.ajouter(ouvert(faire(Kind::Interrupteur, {6, 2}, {6, 5})));
    c.ajouter(faire(Kind::Fil, {6, 5}, {6, 6}));
}

// 4e ch.8 (approfondissement) : une résistance en série avec deux autres en
// dérivation. Valeurs choisies pour tomber juste : 9 V, 100 ohm puis
// 200 // 200 = 100 ohm, donc 4,5 V de chaque côté et 45 mA dans la pile.
void circuitMixte(Circuit& c)
{
    cadre(c, 9.0, 14, 6);
    c.ajouter(faire(Kind::Fil, {0, 0}, {2, 0}));
    c.ajouter(faire(Kind::Resistance, {2, 0}, {6, 0}, 100.0));
    c.ajouter(faire(Kind::Fil, {6, 0}, {10, 0}));
    c.ajouter(faire(Kind::Fil, {10, 0}, {14, 0}));
    c.ajouter(faire(Kind::Resistance, {10, 0}, {10, 6}, 200.0));
    c.ajouter(faire(Kind::Resistance, {14, 0}, {14, 6}, 200.0));
}

// 3e ch.6 (approfondissement) : même TP que le conducteur ohmique, avec une
// lampe. Les points relevés ne sont plus alignés.
void caracteristiqueLampe(Circuit& c)
{
    caracteristique(c);
    for (Elem& e : c.elems)
        if (e.kind == Kind::Resistance) {
            e.kind = Kind::Lampe;
            e.valeur = 6.0;
            e.valeur2 = 0.6;
            e.nom = c.nomDisponible(Kind::Lampe);
        }
    c.construireNetlist();
}

// Une boucle secteur : prise à gauche, appareil à droite. Sert aux
// applications du 4e ch.10 et du 3e ch.6.
void boucleSecteur(Circuit& c, int x0, double tension, Elem appareil)
{
    Elem prise = faire(Kind::Prise, {x0, 4}, {x0, 2}, tension, 16.0);
    c.ajouter(prise);
    c.ajouter(faire(Kind::Fil, {x0, 2}, {x0, 0}));
    c.ajouter(faire(Kind::Fil, {x0, 0}, {x0 + 6, 0}));
    appareil.a = {x0 + 6, 0};
    appareil.b = {x0 + 6, 6};
    c.ajouter(appareil);
    c.ajouter(faire(Kind::Fil, {x0 + 6, 6}, {x0, 6}));
    c.ajouter(faire(Kind::Fil, {x0, 6}, {x0, 4}));
}

// 4e ch.10 : trois appareils de la vie courante, trois circuits côte à côte.
void applications(Circuit& c)
{
    boucleSecteur(c, 0, 230.0, nomme(faire(Kind::Resistance, {}, {}, 46.0), "Lave-linge"));

    c.ajouter(nomme(faire(Kind::Pile, {10, 4}, {10, 2}, 5.0, 0.0), "Chargeur"));
    c.ajouter(faire(Kind::Fil, {10, 2}, {10, 0}));
    c.ajouter(faire(Kind::Fil, {10, 0}, {16, 0}));
    c.ajouter(nomme(faire(Kind::Resistance, {16, 0}, {16, 6}, 20.0), "Smartphone"));
    c.ajouter(faire(Kind::Fil, {16, 6}, {10, 6}));
    c.ajouter(faire(Kind::Fil, {10, 6}, {10, 4}));

    boucleSecteur(c, 20, 230.0, nomme(faire(Kind::Resistance, {}, {}, 115.0), "Bouilloire"));
}

// 3e ch.6 : le ventilateur de l'énoncé, 1 kohm sous 220 V.
void ventilateur(Circuit& c)
{
    Elem prise = faire(Kind::Prise, {0, 4}, {0, 2}, 220.0, 16.0);
    c.ajouter(prise);
    c.ajouter(faire(Kind::Fil, {0, 2}, {0, 0}));
    c.ajouter(faire(Kind::Interrupteur, {0, 0}, {4, 0}));
    c.ajouter(faire(Kind::Fil, {4, 0}, {8, 0}));
    c.ajouter(nomme(faire(Kind::Moteur, {8, 0}, {8, 6}, 1000.0), "Ventilateur"));
    c.ajouter(faire(Kind::Fil, {8, 6}, {0, 6}));
    c.ajouter(faire(Kind::Fil, {0, 6}, {0, 4}));
}

// Un appareil de la maison vu par le circuit : sa résistance équivalente
// R = U² / P sous 230 V.
Elem appareil(const char* nom, double watts)
{
    return nomme(faire(Kind::Resistance, {}, {}, 230.0 * 230.0 / watts), nom);
}

struct Branche {
    Elem recepteur;
    bool enMarche;
};

// Appareils en dérivation sur une prise, chacun derrière son interrupteur.
void tableauElectrique(Circuit& c, const std::vector<Branche>& branches)
{
    const int pas = 7;
    const int largeur = pas * (int)branches.size();
    c.ajouter(faire(Kind::Prise, {0, 6}, {0, 3}, 230.0, 16.0));
    c.ajouter(faire(Kind::Fil, {0, 3}, {0, 0}));
    c.ajouter(faire(Kind::Fil, {0, 6}, {0, 10}));
    c.ajouter(faire(Kind::Fil, {0, 0}, {largeur, 0}));
    c.ajouter(faire(Kind::Fil, {0, 10}, {largeur, 10}));
    int x = pas;
    for (const Branche& b : branches) {
        Elem k = faire(Kind::Interrupteur, {x, 0}, {x, 3});
        k.ferme = b.enMarche;
        c.ajouter(k);
        c.ajouter(faire(Kind::Fil, {x, 3}, {x, 4}));
        Elem r = b.recepteur;
        r.a = {x, 4};
        r.b = {x, 9};
        c.ajouter(r);
        c.ajouter(faire(Kind::Fil, {x, 9}, {x, 10}));
        x += pas;
    }
}

// 3e ch.7 : puissance, énergie et coût, sur des appareils de la maison.
void puissanceEnergie(Circuit& c)
{
    tableauElectrique(c, {{appareil("Sèche-cheveux", 2000.0), true},
                          {appareil("Cafetière", 1000.0), false},
                          {appareil("Chargeur", 10.0), false}});
}

// 4e ch.6 (approfondissement) : l'installation de la maison est en
// dérivation, et le disjoncteur surveille l'intensité totale. Lampe 60 W,
// puis les valeurs du cours : bouilloire 115 ohm (2 A) et lave-linge 46 ohm
// (5 A). Le radiateur, éteint, fera tout sauter.
void installationDomestique(Circuit& c)
{
    tableauElectrique(c, {{nomme(faire(Kind::Lampe, {}, {}, 230.0, 60.0), "Plafonnier"), true},
                          {nomme(faire(Kind::Resistance, {}, {}, 115.0), "Bouilloire"), true},
                          {nomme(faire(Kind::Resistance, {}, {}, 46.0), "Lave-linge"), true},
                          {appareil("Radiateur", 2500.0), false}});
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

    {"Test conducteur ou isolant", "6e ch.13 — Électricité et sécurité",
     "Change le matériau dans Propriétés. Cuivre et graphite : la lampe brille (moins avec le graphite). "
     "Eau salée : elle luit à peine. Plastique, bois, verre : elle reste éteinte, ce sont des isolants.",
     conducteurIsolant},

    {"Fusible contre le court-circuit", "6e ch.13 / 4e ch.6 — Sécurité",
     "Ferme K2 : la lampe est court-circuitée, le fusible de 1 A fond et ouvre le circuit avant que la "
     "pile ne chauffe. Rouvre K2 puis clique sur le fusible pour le remplacer.", fusibleProtection},

    {"Circuit mixte : série et dérivation", "4e ch.8 — Approfondissement",
     "R1 = 100 ohm en série avec R2 et R3 = 200 ohm en dérivation : 4,50 V aux bornes de R1 comme de "
     "R2 et R3, 45,0 mA dans R1 qui se partagent en 22,5 mA + 22,5 mA.", circuitMixte},

    {"Caractéristique U = f(I) d'une lampe", "3e ch.6 — Approfondissement",
     "Règle la pile de 1 V à 6 V et relève à chaque pas : les points ne sont pas alignés, la lampe "
     "n'est pas un conducteur ohmique. Sa résistance passe d'environ 15 ohm à 60 ohm.", caracteristiqueLampe},

    {"Applications : lave-linge, smartphone, bouilloire", "4e ch.10 — Applications",
     "Lave-linge 46 ohm sous 230 V : 5,00 A. Smartphone 20 ohm sous 5 V : 250 mA. "
     "Bouilloire 115 ohm sous 230 V : 2,00 A.", applications},

    {"Ventilateur sur le secteur", "3e ch.6 — Loi d'Ohm",
     "1 kohm sous 220 V : I = 220 / 1 000 = 0,22 A = 220 mA.", ventilateur},

    {"Puissance, énergie et coût", "3e ch.7 — Puissance et énergie",
     "Choisis le sèche-cheveux puis ouvre l'onglet Énergie : P = 2 000 W. En 15 min, "
     "E = 2 kW x 0,25 h = 0,5 kWh, soit 0,10 euro à 0,20 euro le kWh. Allume les autres appareils "
     "et lance le compteur.", puissanceEnergie},

    {"Installation domestique en dérivation", "4e ch.6 — Approfondissement",
     "Chaque appareil reçoit 230 V. Les intensités s'ajoutent : 0,26 + 2 + 5 = 7,26 A pour un "
     "disjoncteur de 16 A. Ferme le radiateur (10,9 A) : 18,1 A, le disjoncteur déclenche.",
     installationDomestique},
};

} // namespace

const std::vector<Exemple>& exemples() { return kExemples; }

} // namespace croc
