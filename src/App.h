// App.h — état de l'application et panneaux.
//
// Un seul objet App porte tout : le montage, le dernier résultat de calcul,
// l'outil courant, la vue et les réglages d'affichage. Les panneaux sont des
// fonctions libres qui prennent App& — pas d'héritage, pas de singleton :
// à cette échelle, une structure et des fonctions se lisent mieux.
#pragma once

#include "Circuit.h"
#include "Exemples.h"
#include "Solver.h"

#include "imgui.h"

#include <deque>
#include <string>
#include <vector>

namespace croc {

enum class Outil {
    Selection,
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

Kind kindDeLOutil(Outil o);   // Outil::Selection → Kind::COUNT

struct Reglages {
    // Mode simple (élève) : gros boutons, gros texte, cinq panneaux seulement
    // (Schéma, Palette, Propriétés, Lois/Mesures, Bibliothèque). Le mode
    // complet (professeur) ajoute Multimètre, Relevés, Graphique et Journal.
    // C'est la disposition « Élève » du § 7 du cahier des charges.
    bool modeSimple = true;

    bool afficherValeurs = true;   // U et I accrochés aux dipôles
    bool afficherNoms    = true;
    bool afficherNoeuds  = true;   // pastille sur les nœuds de dérivation
    bool animerCourant   = true;
    bool sensElectrons   = false;  // false = sens conventionnel (+ vers −)
    bool vuePaillasse    = false;  // réservé au lot V1 (bascule schéma/paillasse)
};

// Une ligne du tableau de relevés : ce que l'élève recopierait sur son cahier
// avant de le porter au tableur (§ 6.6 du cahier des charges).
struct Releve {
    std::string dipole;
    double u = 0, i = 0, r = 0, p = 0;
};

struct App {
    Circuit    circuit;
    Diagnostic diag;
    Reglages   reglages;

    Outil outil     = Outil::Selection;
    int   selection = -1;

    // Vue : origine (en pixels écran) et échelle du quadrillage.
    ImVec2 pan  = ImVec2(0, 0);
    float  zoom = 1.0f;
    bool   vueInitialisee = false;
    bool   ajusterVue     = true;   // recadrer sur le montage au prochain rendu

    // Tracé en cours (pose d'un dipôle par glisser).
    bool trace = false;
    Pt   traceDepart;
    Pt   traceFin;

    double tempsAnim = 0.0;   // horloge de l'animation du courant
    bool   simulationActive = true;

    std::vector<Releve> releves;
    std::string cheminCsv = "releves.csv";

    std::deque<std::string> journal;
    std::vector<Circuit>    historique;   // pile d'annulation
    std::string             fichierCourant;

    void note(const std::string& message);
    void memoriser();          // à appeler AVANT toute modification du montage
    void annuler();
    void recalculer();
    void relever();            // fige une ligne de mesures sur le dipôle choisi
};

// Panneaux (Panels.cpp)
void panneauSchema(App& app);
void panneauPalette(App& app);
void panneauProprietes(App& app);
void panneauMesures(App& app);
void panneauLois(App& app);
void panneauJournal(App& app);
void panneauBibliotheque(App& app);
void panneauMultimetre(App& app);
void panneauReleves(App& app);
void panneauGraphique(App& app);

} // namespace croc
