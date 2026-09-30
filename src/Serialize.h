// Serialize.h — format de fichier .croc
//
// Texte, une ligne par dipôle, lisible et versionnable : un montage doit
// pouvoir être diffé, corrigé à la main et ouvert par toutes les versions
// ultérieures (§ 6.10 du cahier des charges). Pas de binaire, pas de
// dépendance JSON.
#pragma once

#include <string>

namespace croc {

struct Circuit;

bool enregistrer(const Circuit& c, const std::string& chemin, std::string& erreur);
bool ouvrir(Circuit& c, const std::string& chemin, std::string& erreur);

} // namespace croc
