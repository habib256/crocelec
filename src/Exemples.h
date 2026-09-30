// Exemples.h — bibliothèque de montages du cours.
//
// Volontairement séparée d'App.h : ces montages sont aussi le banc d'essai du
// solveur (tests/test_solveur.cpp), qui doit pouvoir tourner sans interface.
#pragma once

#include <vector>

namespace croc {

struct Circuit;

struct Exemple {
    const char* titre;
    const char* chapitre;
    const char* attendu;
    void (*construire)(Circuit&);
};

const std::vector<Exemple>& exemples();

} // namespace croc
