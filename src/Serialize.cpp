#include "Serialize.h"

#include "Circuit.h"

#include <cstdlib>
#include <fstream>
#include <sstream>

namespace croc {
namespace {

// Le nom du dipôle est écrit en dernier et peut contenir des espaces ; tout le
// reste est numérique, ce qui rend le format lisible d'un coup d'œil :
//
//   dipole 3 10 0 10 3 6 0.6 1 0 L1
//          ^  ^-- a --^ ^- b -^ ^ valeurs ^ ^état^ nom
//          type
const char* kEntete = "crocelec 2";

} // namespace

bool enregistrer(const Circuit& c, const std::string& chemin, std::string& erreur)
{
    std::ofstream f(chemin);
    if (!f) { erreur = "impossible d'écrire " + chemin; return false; }

    f << kEntete << "\n";
    f << "# montage CrocElec — " << c.elems.size() << " dipôle(s)\n";
    for (const Elem& e : c.elems) {
        f << "dipole " << (int)e.kind << ' '
          << e.a.x << ' ' << e.a.y << ' ' << e.b.x << ' ' << e.b.y << ' '
          << e.valeur << ' ' << e.valeur2 << ' '
          << (e.ferme ? 1 : 0) << ' ' << (e.grillee ? 1 : 0) << ' '
          << (int)e.mode << ' ' << (int)e.borne << ' ' << e.calibre << ' '
          << (e.nom.empty() ? "-" : e.nom) << '\n';
    }
    if (!f) { erreur = "écriture interrompue"; return false; }
    return true;
}

bool ouvrir(Circuit& c, const std::string& chemin, std::string& erreur)
{
    std::ifstream f(chemin);
    if (!f) { erreur = "fichier introuvable : " + chemin; return false; }

    std::string entete;
    std::getline(f, entete);
    if (entete.rfind("crocelec", 0) != 0) {
        erreur = "ce fichier n'est pas un montage CrocElec";
        return false;
    }
    // Les fichiers de la version 1 n'ont pas les réglages du multimètre : on
    // les relit sans rien perdre, c'est la compatibilité ascendante promise.
    const int version = (entete.size() > 9) ? std::atoi(entete.c_str() + 9) : 1;

    Circuit lu;
    std::string ligne;
    while (std::getline(f, ligne)) {
        if (ligne.empty() || ligne[0] == '#') continue;
        std::istringstream is(ligne);
        std::string mot;
        is >> mot;
        if (mot != "dipole") continue;

        int type = 0, ferme = 1, grillee = 0;
        Elem e;
        is >> type >> e.a.x >> e.a.y >> e.b.x >> e.b.y >> e.valeur >> e.valeur2 >> ferme >> grillee;
        if (!is) { erreur = "ligne illisible : " + ligne; return false; }
        if (type < 0 || type >= (int)Kind::COUNT) { erreur = "dipôle inconnu dans le fichier"; return false; }

        if (version >= 2) {
            int mode = 0, borne = 0;
            double calibre = 20.0;
            is >> mode >> borne >> calibre;
            if (is) {
                e.mode = (ModeMultimetre)mode;
                e.borne = (BorneMultimetre)borne;
                e.calibre = calibre;
            }
        }
        e.kind = (Kind)type;
        e.ferme = (ferme != 0);
        e.grillee = (grillee != 0);
        std::getline(is >> std::ws, e.nom);
        if (e.nom == "-") e.nom.clear();
        lu.elems.push_back(e);
    }

    lu.construireNetlist();
    c = lu;
    return true;
}

} // namespace croc
