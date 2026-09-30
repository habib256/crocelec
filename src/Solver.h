// Solver.h — analyse nodale modifiée (MNA) en régime continu.
//
// Taille du problème : (nœuds − 1) + (piles idéales). Une trentaine de dipôles
// donne une matrice d'une vingtaine de lignes — résolue par élimination de
// Gauss avec pivot partiel, sans dépendance externe, en quelques microsecondes
// même sur le Cortex-A7 de la Galaxy Tab 4 (§ 8.1 du cahier des charges).
//
// Deux dipôles ne sont pas linéaires et imposent une boucle de point fixe :
// la lampe (sa résistance monte avec la température, c'est ce qui rend sa
// caractéristique U(I) non rectiligne — 3e ch.6) et la DEL (tension de seuil).
#pragma once

#include <string>
#include <vector>

namespace croc {

struct Circuit;
struct Elem;

struct Diagnostic {
    bool boucleFermee  = false;  // un courant circule quelque part
    bool courtCircuit  = false;  // une pile débite au-delà du raisonnable
    bool singulier     = false;  // montage que le solveur ne sait pas résoudre
    int  iterations    = 0;      // tours de point fixe (lampes, DEL)
    int  taille        = 0;      // dimension du système résolu
    double dureeMs     = 0;
    std::vector<std::string> messages; // pour le panneau Journal
};

// Résout le circuit et écrit u, i, p, rEff dans chaque dipôle.
Diagnostic resoudre(Circuit& c);

// Résistances de modélisation, exposées pour les tests et l'interface.
namespace modele {
constexpr double kRFil          = 1e-4;   // 0,1 mΩ : invisible à 3 chiffres
constexpr double kROuvert       = 1e12;   // interrupteur ouvert, lampe grillée
constexpr double kRVoltmetre    = 1e7;    // 10 MΩ, comme un vrai multimètre
constexpr double kRAmperemetre  = 1e-4;
constexpr double kRDelPassante  = 0.5;   // la DEL tient son seuil (cf. § 11 critère 7)
constexpr double kSeuilCourtCircuit = 10.0; // A
constexpr double kRShunt        = 0.01;  // shunt derrière la borne 10 A
constexpr double kRShuntMilli   = 0.1;   // shunt du calibre mA, sur la borne V/Ω
constexpr double kIFusible      = 10.0;  // fusible de la borne 10 A
constexpr double kIFusibleMilli = 0.5;   // fusible du calibre mA
} // namespace modele

// Résistance d'une lampe à la puissance dissipée donnée. Sortie du modèle :
// à froid elle vaut le quart de sa résistance nominale, ce qui reproduit
// l'appel de courant à l'allumage et la courbure de la caractéristique.
double resistanceLampe(double tensionNom, double puissanceNom, double puissance);

// Résistance que le solveur retient pour un dipôle dans son état courant.
double resistanceModele(const Elem& e);

// Résistance vue entre deux nœuds, générateurs éteints — c'est exactement ce
// que lit un ohmmètre posé sur un circuit hors tension (4e ch.7). Rend une
// valeur très grande si les deux nœuds ne communiquent pas.
double resistanceEntre(const Circuit& c, int na, int nb);

} // namespace croc
