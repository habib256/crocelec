#include "Solver.h"

#include "Circuit.h"
#include "Format.h"

#include <algorithm>
#include <chrono>
#include <cmath>

namespace croc {

using namespace modele;

double resistanceLampe(double tensionNom, double puissanceNom, double puissance)
{
    if (tensionNom <= 0 || puissanceNom <= 0) return 100.0;
    const double rNom = tensionNom * tensionNom / puissanceNom; // 6 V / 0,6 W → 60 Ω
    const double k    = 3.0;                                    // froid = rNom/4
    double x = puissance / puissanceNom;
    if (x < 0)   x = 0;
    if (x > 3.0) x = 3.0;   // au-delà, elle grille de toute façon
    return rNom / (1.0 + k) * (1.0 + k * x);
}

namespace {

// Résistance interne prêtée à une pile idéale quand le montage la court-circuite.
// Une pile plate réelle vaut quelques dixièmes d'ohm : c'est ce qui transforme
// un système singulier (courant infini) en un courant énorme mais calculable,
// donc affichable et commentable à l'élève.
constexpr double kRSecours = 0.1;

// Élimination de Gauss avec pivot partiel. A est détruite ; rend false si la
// matrice est singulière — cas qu'on veut signaler à l'élève, pas masquer.
bool resoudreSysteme(std::vector<double>& A, std::vector<double>& x, int n)
{
    if (n == 0) return true;
    auto at = [&](int r, int c) -> double& { return A[(std::size_t)r * n + c]; };

    for (int col = 0; col < n; ++col) {
        int pivot = col;
        double best = std::fabs(at(col, col));
        for (int r = col + 1; r < n; ++r) {
            const double v = std::fabs(at(r, col));
            if (v > best) { best = v; pivot = r; }
        }
        if (best < 1e-18) return false;

        if (pivot != col) {
            for (int c = col; c < n; ++c) std::swap(at(col, c), at(pivot, c));
            std::swap(x[col], x[pivot]);
        }
        const double inv = 1.0 / at(col, col);
        for (int r = col + 1; r < n; ++r) {
            const double f = at(r, col) * inv;
            if (f == 0.0) continue;
            for (int c = col; c < n; ++c) at(r, c) -= f * at(col, c);
            x[r] -= f * x[col];
        }
    }
    for (int r = n - 1; r >= 0; --r) {
        double s = x[r];
        for (int c = r + 1; c < n; ++c) s -= at(r, c) * x[c];
        x[r] = s / at(r, r);
    }
    return true;
}

bool delPassante(const Elem& e);
bool resoudreSysteme(std::vector<double>& A, std::vector<double>& x, int n);

// Un générateur fournit de l'énergie tant qu'il n'est pas coupé : une prise
// dont le disjoncteur a déclenché n'est plus qu'un circuit ouvert.
bool generateurActif(const Elem& e)
{
    return e.kind == Kind::Pile || (e.kind == Kind::Prise && !e.grillee);
}

// Résistance interne déclarée. La prise secteur n'en a pas : le réseau tient
// ses 230 V quel que soit l'appareil branché, comme dans les calculs du cours.
double resistanceInterneDeclaree(const Elem& e)
{
    return (e.kind == Kind::Pile) ? e.valeur2 : 0.0;
}

} // namespace

// Résistance retenue pour un dipôle à cette itération. Les dipôles non
// linéaires (lampe, DEL) lisent l'état du tour précédent.
double resistanceModele(const Elem& e)
{
    switch (e.kind) {
    case Kind::Fil:          return kRFil;
    case Kind::Interrupteur: return e.ferme ? kRFil : kROuvert;
    case Kind::Resistance:   return e.valeur > 0 ? e.valeur : 1e-3;
    case Kind::Moteur:       return e.valeur > 0 ? e.valeur : 1e-3;
    case Kind::Voltmetre:    return kRVoltmetre;
    case Kind::Amperemetre:  return kRAmperemetre;
    case Kind::Lampe:        return e.grillee ? kROuvert
                                              : (e.rEff > 0 ? e.rEff : resistanceLampe(e.valeur, e.valeur2, 0.0));
    case Kind::Del:          return e.grillee ? kROuvert : (e.rEff > 0 ? e.rEff : kROuvert);
    case Kind::Fusible:      return e.grillee ? kROuvert : kRFil;
    case Kind::Materiau:     return resistanceMateriau((int)e.valeur);
    case Kind::Prise:        return e.grillee ? kROuvert : kRFil;   // éteinte : un simple conducteur
    case Kind::Multimetre: {
        // L'impédance dépend de la BORNE où est planté le cordon rouge, pas du
        // sélecteur : c'est le shunt de la borne 10 A qui court-circuite le
        // dipôle quand l'élève laisse le cordon au mauvais endroit — la faute
        // que le cours 4e ch.7 cherche justement à faire vivre.
        if (e.grillee) return kROuvert;                                  // fusible fondu
        if (e.borne == BorneMultimetre::DixAmperes) return kRShunt;      // shunt : quoi qu'affiche le sélecteur
        if (e.mode == ModeMultimetre::Intensite)    return kRShuntMilli; // calibre mA
        if (e.mode == ModeMultimetre::Resistance)   return kROuvert;     // ohmmètre : rien ne passe
        return kRVoltmetre;
    }
    default:                 return kROuvert;
    }
}

namespace {

bool delPassante(const Elem& e)
{
    return !e.grillee && e.rEff > 0 && e.rEff < kROuvert / 2;
}

// Une passe complète : assemblage MNA + itérations de point fixe pour les
// dipôles non linéaires. `rSecours` > 0 donne une résistance interne aux piles
// idéales (voir kRSecours). Rend false si le système est singulier.
bool passe(Circuit& c, Diagnostic& diag, double rSecours)
{
    const int nbNoeuds = c.net.nbNoeuds;

    int ref = 0;
    std::vector<int> piles;
    for (std::size_t k = 0; k < c.elems.size(); ++k)
        if (generateurActif(c.elems[k])) piles.push_back((int)k);
    if (!piles.empty()) ref = c.elems[piles[0]].na;

    auto resistanceInterne = [&](const Elem& e) {
        const double r = resistanceInterneDeclaree(e);
        return (r > 1e-9) ? r : rSecours;
    };

    // Une pile de résistance interne nulle est une source de tension idéale :
    // elle réclame une inconnue de branche. Avec r > 0, l'équivalent de Norton
    // suffit et le système reste plus petit.
    std::vector<int> pilesIdeales;
    for (int k : piles)
        if (resistanceInterne(c.elems[k]) <= 1e-9) pilesIdeales.push_back(k);

    const int nNode = nbNoeuds - 1;
    const int nSrc  = (int)pilesIdeales.size();
    const int n     = nNode + nSrc;
    diag.taille = n;
    if (n <= 0) {
        // Un seul nœud : le générateur est relié à lui-même par un fil, rien
        // d'autre. Aucun système à résoudre, seulement sa résistance interne.
        for (int k : piles) {
            Elem& e = c.elems[k];
            e.u = 0.0;
            e.i = e.valeur / resistanceInterne(e);
            e.p = 0.0;
        }
        return true;
    }

    auto idx = [&](int noeud) {
        if (noeud < 0 || noeud == ref) return -1;
        return noeud < ref ? noeud : noeud - 1;
    };

    std::vector<double> A, rhs, x;
    bool convergence = false;

    for (int iter = 0; iter < 60 && !convergence; ++iter) {
        diag.iterations = iter + 1;
        A.assign((std::size_t)n * n, 0.0);
        rhs.assign(n, 0.0);

        auto stampG = [&](int na, int nb, double g) {
            // Un dipôle bouclé sur un seul nœud (un fil posé en l'air) ne
            // porte aucune équation. L'estampiller ajouterait puis retirerait
            // g sur la diagonale, et l'arrondi de ce va-et-vient effacerait
            // la conductance de fuite : matrice singulière pour un simple fil.
            if (na == nb) return;
            const int ia = idx(na), ib = idx(nb);
            if (ia >= 0) A[(std::size_t)ia * n + ia] += g;
            if (ib >= 0) A[(std::size_t)ib * n + ib] += g;
            if (ia >= 0 && ib >= 0) {
                A[(std::size_t)ia * n + ib] -= g;
                A[(std::size_t)ib * n + ia] -= g;
            }
        };
        // Source de courant : `courant` ampères injectés dans na, repris en nb.
        auto stampI = [&](int na, int nb, double courant) {
            const int ia = idx(na), ib = idx(nb);
            if (ia >= 0) rhs[ia] += courant;
            if (ib >= 0) rhs[ib] -= courant;
        };

        // Conductance de fuite sur chaque nœud : un montage inachevé comporte
        // toujours des nœuds flottants, et sans elle la matrice serait
        // singulière alors que la réponse physique (aucun courant) est claire.
        for (int i = 0; i < nNode; ++i) A[(std::size_t)i * n + i] += 1e-12;

        for (const Elem& e : c.elems) {
            if (e.na < 0 || e.nb < 0) continue;
            if (generateurActif(e)) {
                const double r = resistanceInterne(e);
                if (r > 1e-9) {                     // Norton : E/r en parallèle sur 1/r
                    const double g = 1.0 / r;
                    stampG(e.na, e.nb, g);
                    stampI(e.nb, e.na, e.valeur * g);   // courant sortant de la borne +
                }
                continue;                               // sinon : inconnue de branche, plus bas
            }
            switch (e.kind) {
            case Kind::Del: {
                if (delPassante(e)) {
                    // Seuil modélisé en source de Norton : i = (u − Useuil)/Ron,
                    // donc Useuil/Ron ampères injectés du côté de l'anode.
                    const double g = 1.0 / e.rEff;
                    stampG(e.na, e.nb, g);
                    stampI(e.na, e.nb, e.valeur * g);
                } else {
                    stampG(e.na, e.nb, 1.0 / kROuvert);
                }
                break;
            }
            default:
                stampG(e.na, e.nb, 1.0 / resistanceModele(e));
                break;
            }
        }

        for (int s = 0; s < nSrc; ++s) {
            const Elem& e = c.elems[pilesIdeales[s]];
            const int ligne = nNode + s;
            const int ip = idx(e.nb);   // borne +
            const int in = idx(e.na);   // borne −
            if (ip >= 0) { A[(std::size_t)ip * n + ligne] += 1.0; A[(std::size_t)ligne * n + ip] += 1.0; }
            if (in >= 0) { A[(std::size_t)in * n + ligne] -= 1.0; A[(std::size_t)ligne * n + in] -= 1.0; }
            rhs[ligne] = e.valeur;
        }

        x = rhs;
        if (!resoudreSysteme(A, x, n)) return false;

        auto potentiel = [&](int noeud) { const int i = idx(noeud); return i < 0 ? 0.0 : x[i]; };

        double ecartMax = 0.0;
        int srcCourant = 0;
        for (Elem& e : c.elems) {
            if (e.na < 0 || e.nb < 0) continue;
            e.u = potentiel(e.na) - potentiel(e.nb);

            if (generateurActif(e)) {
                const double r = resistanceInterne(e);
                if (r > 1e-9) e.i = (e.valeur + e.u) / r;   // (E − U aux bornes) / r
                else          e.i = -x[nNode + srcCourant++];
            } else if (e.kind == Kind::Del) {
                if (delPassante(e)) e.i = (e.u - e.valeur) / e.rEff;
                else                e.i = e.u / kROuvert;
            } else {
                e.i = e.u / resistanceModele(e);
            }
            e.p = e.u * e.i;

            if (e.kind == Kind::Lampe && !e.grillee) {
                // Au premier tour, la lampe est encore froide : le courant
                // calculé est celui de l'allumage, pas celui du régime établi.
                // Il faut donc forcer un tour de plus, sinon on déclarerait la
                // convergence sur l'appel de courant — et la lampe grillerait
                // à chaque mise sous tension.
                const bool premiere = (e.rEff <= 0);
                const double cible = resistanceLampe(e.valeur, e.valeur2, std::fabs(e.p));
                const double avant = premiere ? cible : e.rEff;
                const double neuf  = 0.5 * avant + 0.5 * cible;   // amortissement
                ecartMax = premiere ? 1.0
                                    : std::max(ecartMax, std::fabs(neuf - avant) / std::max(1.0, avant));
                e.rEff = neuf;
            } else if (e.kind == Kind::Del && !e.grillee) {
                // Modèle linéaire par morceaux : passante tant que le courant
                // reste positif, bloquée tant que la tension n'atteint pas le seuil.
                const bool avant = delPassante(e);
                const bool apres = avant ? (e.i > 0) : (e.u > e.valeur);
                e.rEff = apres ? kRDelPassante : kROuvert;
                if (apres != avant) ecartMax = 1.0;               // l'état a basculé
            } else {
                e.rEff = resistanceModele(e);
            }
        }
        convergence = (ecartMax < 1e-5);
    }
    return true;
}

} // namespace

double resistanceEntre(const Circuit& c, int na, int nb)
{
    if (na < 0 || nb < 0 || na == nb || c.net.nbNoeuds < 2) return 0.0;

    // Réseau passif : une pile éteinte est un simple morceau de fil (sa f.é.m.
    // tombe à zéro, il ne reste que sa résistance interne).
    const int ref = na;
    const int n = c.net.nbNoeuds - 1;
    auto idx = [&](int noeud) {
        if (noeud < 0 || noeud == ref) return -1;
        return noeud < ref ? noeud : noeud - 1;
    };

    std::vector<double> A((std::size_t)n * n, 0.0), x(n, 0.0);
    auto stampG = [&](int p, int q, double g) {
        if (p == q) return;                    // voir passe() : dipôle bouclé sur un nœud
        const int ia = idx(p), ib = idx(q);
        if (ia >= 0) A[(std::size_t)ia * n + ia] += g;
        if (ib >= 0) A[(std::size_t)ib * n + ib] += g;
        if (ia >= 0 && ib >= 0) {
            A[(std::size_t)ia * n + ib] -= g;
            A[(std::size_t)ib * n + ia] -= g;
        }
    };

    for (int i = 0; i < n; ++i) A[(std::size_t)i * n + i] += 1e-12;
    for (const Elem& e : c.elems) {
        if (e.na < 0 || e.nb < 0) continue;
        double r;
        if (e.kind == Kind::Pile)
            r = (e.valeur2 > 1e-9) ? e.valeur2 : kRFil;
        else if (e.kind == Kind::Multimetre)
            r = kROuvert;                      // l'appareil ne se mesure pas lui-même
        else
            r = resistanceModele(e);
        stampG(e.na, e.nb, 1.0 / r);
    }

    // 1 ampère injecté dans nb, repris en na : la tension lue vaut la résistance.
    const int ib = idx(nb);
    if (ib < 0) return 0.0;
    x[ib] = 1.0;
    if (!resoudreSysteme(A, x, n)) return kROuvert;
    return std::fabs(x[ib]);
}

namespace {

// Un calcul complet suivi de sa lecture pédagogique. Rend vrai si une
// protection vient de s'ouvrir (disjoncteur, fusible, lampe ou DEL qui
// grille) : le montage a changé, il faut le recalculer.
bool resoudreUneFois(Circuit& c, Diagnostic& diag)
{
    for (Elem& e : c.elems) { e.u = e.i = e.p = 0; e.allume = false; }
    diag.boucleFermee = diag.courtCircuit = diag.singulier = false;
    bool coupure = false;

    if (c.net.nbNoeuds >= 1) {
        // Un générateur dont les deux bornes tombent sur le même nœud est
        // court-circuité : plus aucune équation ne le tient. On lui prête sa
        // résistance interne réelle plutôt que de rendre « circuit impossible ».
        bool pileCourtCircuitee = false;
        for (const Elem& e : c.elems)
            if (generateurActif(e) && e.na == e.nb && resistanceInterneDeclaree(e) <= 1e-9)
                pileCourtCircuitee = true;

        double rSecours = pileCourtCircuitee ? kRSecours : 0.0;
        if (!passe(c, diag, rSecours) && rSecours == 0.0) {
            rSecours = kRSecours;                       // seconde chance : piles réelles
            if (!passe(c, diag, rSecours)) {
                diag.singulier = true;
                diag.messages.push_back("Ce montage ne peut pas être résolu : vérifie qu'aucun dipôle n'est posé deux fois au même endroit.");
            }
        }
    }

    // Lecture pédagogique du résultat.
    for (Elem& e : c.elems) {
        if (std::fabs(e.i) > 1e-6) diag.boucleFermee = true;

        if (e.kind == Kind::Lampe && !e.grillee) {
            e.allume = std::fabs(e.p) > 0.02 * std::max(1e-9, e.valeur2);
            if (std::fabs(e.p) > 1.6 * e.valeur2) {
                e.grillee = true;
                e.allume  = false;
                coupure   = true;
                diag.messages.push_back(e.nom + " a grillé : la puissance reçue dépasse largement sa valeur nominale.");
            }
        }
        if (e.kind == Kind::Del && !e.grillee) {
            e.allume = e.i > 1e-4;
            if (e.i > e.valeur2) {
                e.grillee = true;
                e.allume  = false;
                coupure   = true;
                diag.messages.push_back(e.nom + " a grillé : une DEL demande une résistance de protection en série.");
            }
        }
        if (e.kind == Kind::Fusible && !e.grillee && std::fabs(e.i) > e.valeur) {
            e.grillee = true;
            coupure   = true;
            diag.messages.push_back(e.nom + " a fondu : " + fmtIntensite(std::fabs(e.i)) + " pour un fusible de " +
                                    fmtIntensite(e.valeur) + ". Il a ouvert le circuit et protégé les autres dipôles.");
        }
        if (e.kind == Kind::Prise && !e.grillee && std::fabs(e.i) > e.valeur2) {
            e.grillee = true;
            coupure   = true;
            const bool courtCircuit = std::fabs(e.i) > 10.0 * e.valeur2;
            diag.messages.push_back(
                "Disjoncteur de " + e.nom + " déclenché : " +
                (courtCircuit ? std::string("court-circuit") : fmtIntensite(std::fabs(e.i))) +
                " pour un calibre de " + fmtIntensite(e.valeur2) +
                (courtCircuit ? ". Sans lui, les fils chaufferaient jusqu'à l'incendie."
                              : ". Trop d'appareils en marche : il coupe avant que les fils ne chauffent."));
        }
        if (e.kind == Kind::Multimetre) {
            if (e.grillee) {
                e.lecture = 0.0;
                e.depassement = false;
            } else {
                switch (e.mode) {
                case ModeMultimetre::Tension:    e.lecture = std::fabs(e.u); break;
                case ModeMultimetre::Intensite:  e.lecture = std::fabs(e.i); break;
                case ModeMultimetre::Resistance: e.lecture = resistanceEntre(c, e.na, e.nb); break;
                }
                // « 1. » : le calibre choisi est trop petit, il faut passer au
                // calibre supérieur. C'est la manipulation du TP 4e ch.7.
                e.depassement = (e.lecture > e.calibre);

                const double fusible = (e.borne == BorneMultimetre::DixAmperes) ? kIFusible : kIFusibleMilli;
                const bool enShunt = (e.borne == BorneMultimetre::DixAmperes ||
                                      e.mode == ModeMultimetre::Intensite);
                if (enShunt && std::fabs(e.i) > fusible) {
                    e.grillee = true;
                    coupure   = true;
                    e.lecture = 0.0;
                    diag.messages.push_back(
                        e.nom + " : fusible fondu. Un ampèremètre se branche EN SÉRIE ; "
                                "en dérivation il court-circuite le dipôle.");
                }
                if (e.mode == ModeMultimetre::Tension && e.borne == BorneMultimetre::DixAmperes) {
                    diag.messages.push_back(
                        e.nom + " : sélecteur sur V mais cordon rouge sur la borne 10 A — "
                                "le shunt court-circuite le dipôle mesuré.");
                }
                if (e.mode == ModeMultimetre::Resistance && std::fabs(e.u) > 0.1) {
                    diag.messages.push_back(
                        e.nom + " : un ohmmètre se branche HORS CIRCUIT, courant coupé. "
                                "La valeur affichée ne veut rien dire.");
                }
            }
        }

        if (e.kind == Kind::Pile && std::fabs(e.i) > kSeuilCourtCircuit) {
            diag.courtCircuit = true;
            diag.messages.push_back("Court-circuit : " + e.nom + " débite " + fmtIntensite(std::fabs(e.i)) +
                                    ". Ne jamais relier directement le + au − d'une pile.");
        }
    }
    return coupure;
}

} // namespace

double puissanceFournie(const Circuit& c)
{
    // Convention de Elem : pour un générateur qui débite, le courant entre
    // par la borne − (a) et ressort par la borne + (b), donc u × i < 0.
    double p = 0.0;
    for (const Elem& e : c.elems)
        if (estGenerateur(e.kind)) p -= e.p;
    return p;
}

Diagnostic resoudre(Circuit& c)
{
    const auto t0 = std::chrono::steady_clock::now();
    Diagnostic diag;

    c.construireNetlist();
    // Un disjoncteur qui déclenche ou une lampe qui grille change le montage :
    // on le recalcule aussitôt, sinon l'élève verrait encore le courant
    // d'avant la coupure. Chaque tour ouvre au moins un dipôle, quatre suffisent.
    for (int tour = 0; tour < 4; ++tour)
        if (!resoudreUneFois(c, diag)) break;

    // Les tours successifs répètent les avertissements permanents (ohmmètre
    // alimenté…) : on n'en garde qu'un exemplaire.
    std::vector<std::string> uniques;
    for (const std::string& m : diag.messages)
        if (std::find(uniques.begin(), uniques.end(), m) == uniques.end()) uniques.push_back(m);
    diag.messages.swap(uniques);

    const auto t1 = std::chrono::steady_clock::now();
    diag.dureeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    return diag;
}

} // namespace croc
