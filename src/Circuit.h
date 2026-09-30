// Circuit.h — le montage : dipôles posés sur la grille, et la netlist qu'on
// en extrait (nœuds, degrés, branches parallèles).
//
// Modèle volontairement minimal, dicté par le § 8.1 du cahier des charges :
// tout doit tenir dans 128 Mo et se résoudre en quelques millisecondes sur un
// Cortex-A7. Un dipôle = deux extrémités sur des points entiers de la grille ;
// deux extrémités au même point sont électriquement le même nœud. C'est le
// modèle de la paillasse : on relie en posant les bornes au même endroit.
#pragma once

#include <string>
#include <vector>

namespace croc {

enum class Kind {
    Fil,
    Pile,
    Interrupteur,
    Lampe,
    Resistance,
    Moteur,
    Del,
    Voltmetre,
    Amperemetre,
    Multimetre,
    COUNT
};

// Modes du multimètre — trois appareils en un, comme le dit le cours 4e ch.7.
enum class ModeMultimetre { Tension = 0, Intensite = 1, Resistance = 2 };

// Bornes où l'élève enfonce le cordon rouge. Le cordon noir est toujours sur
// COM. Se tromper de borne fait partie de l'apprentissage : l'appareil ne
// mesure alors rien, et le dit.
enum class BorneMultimetre { VOhm = 0, DixAmperes = 1 };

const char* nomKind(Kind k);        // "Résistance", pour l'interface
const char* prefixeKind(Kind k);    // "R", pour l'étiquetage automatique
const char* aideKind(Kind k);       // une ligne d'explication pour l'élève

struct Pt {
    int x = 0, y = 0;
};
inline bool operator==(Pt a, Pt b) { return a.x == b.x && a.y == b.y; }
inline bool operator!=(Pt a, Pt b) { return !(a == b); }

struct Elem {
    Kind kind = Kind::Fil;
    Pt   a, b;              // extrémités, en coordonnées de grille
    std::string nom;        // L1, R2, G…  (attribué à la pose)

    // Paramètres. Le sens de chaque champ dépend du dipôle :
    //   Pile         valeur = f.é.m. E (V)      valeur2 = résistance interne r
    //   Resistance   valeur = R (Ω)
    //   Moteur       valeur = R (Ω)
    //   Lampe        valeur = tension nominale  valeur2 = puissance nominale
    //   Del          valeur = tension de seuil  valeur2 = intensité maximale
    double valeur  = 0;
    double valeur2 = 0;

    bool ferme   = true;    // interrupteur : fermé ?
    bool grillee = false;   // lampe / DEL détruite, fusible du multimètre fondu

    // Multimètre : réglages de la face avant et état de l'afficheur.
    ModeMultimetre  mode  = ModeMultimetre::Tension;
    BorneMultimetre borne = BorneMultimetre::VOhm;
    double calibre     = 20.0;    // pleine échelle, dans l'unité du mode
    double lecture     = 0.0;     // ce que montre l'afficheur (rempli au calcul)
    bool   depassement = false;   // « 1. » : la mesure dépasse le calibre

    // Résultats du dernier calcul. Convention unique : u = V(a) − V(b), et i
    // est le courant qui entre en `a` et ressort en `b`. Le signe porte le
    // sens du courant, l'affichage collège en prend la valeur absolue.
    double u = 0, i = 0, p = 0;
    double rEff = 0;        // résistance réellement retenue (lampe : elle varie)
    bool   allume = false;  // lampe/DEL qui éclaire

    int na = -1, nb = -1;   // nœuds attribués par la netlist
};

// Netlist : le circuit vu comme un graphe. Reconstruite à chaque modification
// du montage (c'est instantané à cette échelle, et ça évite tout cache faux).
struct Netlist {
    int nbNoeuds = 0;
    std::vector<Pt>  position;   // un point représentatif par nœud
    std::vector<int> degre;      // nombre d'extrémités de dipôles (fils exclus)
    std::vector<int> degreTotal; // fils compris — sert au dessin du point de jonction
};

struct Circuit {
    std::vector<Elem> elems;
    Netlist net;

    int  ajouter(const Elem& e);            // rend l'indice, nomme le dipôle
    void supprimer(int index);
    void vider();
    void construireNetlist();               // remplit net et les na/nb

    // Un nœud est un « nœud » au sens du cours (5e ch.8) quand au moins trois
    // fils s'y rejoignent : c'est là que le courant se divise.
    bool estNoeudDeDerivation(int noeud) const;
    int  nbNoeudsDeDerivation() const;

    // Vrai quand le montage ne comporte aucun nœud de dérivation : une seule
    // boucle, donc « circuit en série » dans les mots du programme.
    bool estEnSerie() const;

    // Dipôles branchés entre les deux mêmes nœuds : les branches en
    // dérivation dont le cours dit qu'elles ont la même tension.
    std::vector<std::vector<int>> groupesParalleles() const;

    std::string nomDisponible(Kind k) const;
};

} // namespace croc
