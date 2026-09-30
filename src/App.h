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
    Gomme,          // efface ce qu'elle touche, comme le crocodile de Crocodile Clips
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
    Prise,
    Fusible,
    Materiau,
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
    int  niveau          = 0;      // filtre de la Palette : 6, 5, 4, 3, ou 0 pour tout
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

    // Tracé en cours (pose d'un dipôle par glisser). `traceFil` : le tracé
    // est parti de l'extrémité d'un dipôle en mode Sélection, il donnera un
    // fil quel que soit l'outil choisi.
    bool trace = false;
    bool traceFil = false;
    Pt   traceDepart;
    Pt   traceFin;

    double tempsAnim = 0.0;   // horloge de l'animation du courant
    bool   simulationActive = true;

    std::vector<Releve> releves;
    std::string cheminCsv = "releves.csv";

    // Énergie (3e ch.7) : calcul E = P × t sur l'appareil choisi, et compteur
    // de l'installation qui tourne en temps accéléré.
    double prixKWh          = 0.20;   // €/kWh — à ajuster au tarif en vigueur
    double dureeMinutes     = 60.0;   // durée du calcul E = P × t
    bool   compteurEnMarche = false;
    double compteurJoules   = 0.0;
    double compteurSecondes = 0.0;    // temps simulé, pas le temps réel
    double acceleration     = 60.0;   // 1 s à l'écran = 1 min au compteur

    std::deque<std::string> journal;
    std::vector<Circuit>    historique;   // pile d'annulation
    std::string             fichierCourant;

    void note(const std::string& message);
    void memoriser();          // à appeler AVANT toute modification du montage
    void annuler();
    void recalculer();
    void relever();            // fige une ligne de mesures sur le dipôle choisi
    void avancerCompteur(double dtReel);
    void tournerSelection();   // quart de tour du dipôle choisi (touche R)
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
void panneauEnergie(App& app);

} // namespace croc
