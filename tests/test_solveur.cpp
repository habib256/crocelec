// test_solveur.cpp — recette chiffrée du solveur.
//
// Chaque cas reprend une valeur écrite noir sur blanc dans mes cours ou dans
// le § 11 du cahier des charges. Si l'un d'eux tombe, une valeur affichée à
// l'élève est devenue fausse : c'est le seul test qui compte vraiment.
#include "Circuit.h"
#include "Exemples.h"
#include "Format.h"
#include "Serialize.h"
#include "Solver.h"

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>

using namespace croc;

namespace {

int echecs = 0;
int total  = 0;

void verifie(const std::string& intitule, double obtenu, double attendu, double tolerance,
             const char* unite = "")
{
    ++total;
    const bool ok = std::fabs(obtenu - attendu) <= tolerance;
    if (!ok) ++echecs;
    std::printf("%s %-58s %10.4f %-3s (attendu %.4f +/- %.4f)\n",
                ok ? "  ok  " : " ECHEC", intitule.c_str(), obtenu, unite, attendu, tolerance);
}

void verifieVrai(const std::string& intitule, bool obtenu, bool attendu)
{
    ++total;
    const bool ok = (obtenu == attendu);
    if (!ok) ++echecs;
    std::printf("%s %-58s %10s     (attendu %s)\n", ok ? "  ok  " : " ECHEC", intitule.c_str(),
                obtenu ? "vrai" : "faux", attendu ? "vrai" : "faux");
}

Elem faire(Kind k, Pt a, Pt b, double v = 0, double v2 = 0)
{
    Elem e;
    e.kind = k; e.a = a; e.b = b; e.valeur = v; e.valeur2 = v2;
    return e;
}

// Boucle minimale : une pile et un dipôle, reliés par deux fils.
Circuit boucle(double tension, Kind kind, double valeur, double valeur2 = 0)
{
    Circuit c;
    c.ajouter(faire(Kind::Pile, {0, 2}, {0, 0}, tension, 0.0));
    c.ajouter(faire(Kind::Fil, {0, 0}, {4, 0}));
    c.ajouter(faire(kind, {4, 0}, {4, 2}, valeur, valeur2));
    c.ajouter(faire(Kind::Fil, {4, 2}, {0, 2}));
    return c;
}

const Elem* premier(const Circuit& c, Kind k, int rang = 0)
{
    int vu = 0;
    for (const Elem& e : c.elems)
        if (e.kind == k && vu++ == rang) return &e;
    return nullptr;
}

void construire(const char* titre, Circuit& c)
{
    for (const Exemple& ex : exemples())
        if (std::string(ex.titre).find(titre) != std::string::npos) { ex.construire(c); return; }
    std::printf(" ECHEC montage introuvable : %s\n", titre);
    ++echecs;
}

} // namespace

int main()
{
    std::printf("CrocElec — recette du solveur\n\n");

    // ── Loi d'Ohm, les valeurs exactes du cours ──────────────────────────
    {
        Circuit c = boucle(6.0, Kind::Resistance, 60.0);   // 4e ch.9
        resoudre(c);
        verifie("R = 60 ohm sous 6 V : I", std::fabs(premier(c, Kind::Resistance)->i), 0.1, 1e-4, "A");
    }
    {
        Circuit c = boucle(5.0, Kind::Resistance, 20.0);   // smartphone, 4e ch.10
        resoudre(c);
        verifie("R = 20 ohm sous 5 V : I", std::fabs(premier(c, Kind::Resistance)->i), 0.25, 1e-4, "A");
    }
    {
        Circuit c = boucle(230.0, Kind::Resistance, 115.0); // bouilloire, 4e ch.10
        resoudre(c);
        const Elem* r = premier(c, Kind::Resistance);
        verifie("R = 115 ohm sous 230 V : I", std::fabs(r->i), 2.0, 1e-3, "A");
        verifie("R = 115 ohm sous 230 V : U", std::fabs(r->u), 230.0, 1e-2, "V");
    }
    {
        Circuit c = boucle(230.0, Kind::Resistance, 23.0);  // fer a repasser, 4e ch.10
        resoudre(c);
        verifie("Fer a repasser 23 ohm / 230 V : P", std::fabs(premier(c, Kind::Resistance)->p),
                2300.0, 1.0, "W");
    }
    {
        Circuit c = boucle(220.0, Kind::Resistance, 1000.0); // ventilateur, 3e ch.6
        resoudre(c);
        verifie("R = 1 kohm sous 220 V : I", std::fabs(premier(c, Kind::Resistance)->i), 0.22, 1e-4, "A");
    }

    // ── TP de référence : 9 V et cinq lampes en série ────────────────────
    {
        Circuit c;
        construire("cinq lampes", c);
        resoudre(c);
        for (int k = 0; k < 5; ++k)
            verifie("9 V / 5 lampes en serie : U aux bornes de L" + std::to_string(k + 1),
                    std::fabs(premier(c, Kind::Lampe, k)->u), 1.80, 0.01, "V");
        verifieVrai("9 V / 5 lampes : le circuit est bien vu en serie", c.estEnSerie(), true);
    }

    // ── Deux lampes en série : la tension se partage ─────────────────────
    {
        Circuit c;
        construire("Deux lampes en série", c);
        resoudre(c);
        const double u1 = std::fabs(premier(c, Kind::Lampe, 0)->u);
        const double u2 = std::fabs(premier(c, Kind::Lampe, 1)->u);
        const double i1 = std::fabs(premier(c, Kind::Lampe, 0)->i);
        const double i2 = std::fabs(premier(c, Kind::Lampe, 1)->i);
        verifie("Deux lampes en serie : U1", u1, 4.5, 0.01, "V");
        verifie("Deux lampes en serie : U1 + U2 = 9 V", u1 + u2, 9.0, 0.02, "V");
        verifie("Deux lampes en serie : meme intensite", i1 - i2, 0.0, 1e-9, "A");
    }

    // ── Deux lampes en dérivation : même tension, loi des nœuds ──────────
    {
        Circuit c;
        construire("dérivation", c);
        resoudre(c);
        const Elem* l1 = premier(c, Kind::Lampe, 0);
        const Elem* l2 = premier(c, Kind::Lampe, 1);
        const Elem* g  = premier(c, Kind::Pile);
        verifie("Derivation : U aux bornes de L1", std::fabs(l1->u), 9.0, 0.01, "V");
        verifie("Derivation : U aux bornes de L2", std::fabs(l2->u), 9.0, 0.01, "V");
        verifie("Derivation : I = I1 + I2 (loi des noeuds)",
                std::fabs(g->i) - (std::fabs(l1->i) + std::fabs(l2->i)), 0.0, 1e-6, "A");
        verifieVrai("Derivation : le circuit n'est pas vu en serie", c.estEnSerie(), false);
        verifieVrai("Derivation : deux noeuds detectes", c.nbNoeudsDeDerivation() == 2, true);
    }

    // ── DEL et sa résistance de protection ───────────────────────────────
    {
        Circuit c;
        construire("DEL", c);
        resoudre(c);
        const Elem* del = premier(c, Kind::Del);
        verifie("DEL 2 V + 350 ohm sur 9 V : I", std::fabs(del->i) * 1000.0, 20.0, 0.3, "mA");
        verifieVrai("DEL : elle est allumee", del->allume, true);
    }
    {
        // Même montage, résistance de protection retirée : la DEL grille.
        Circuit c;
        construire("DEL", c);
        for (Elem& e : c.elems)
            if (e.kind == Kind::Resistance) e.valeur = 0.5;   // presque un fil
        resoudre(c);
        verifieVrai("DEL sans protection : elle grille", premier(c, Kind::Del)->grillee, true);
    }

    // ── Circuit ouvert et court-circuit ──────────────────────────────────
    {
        Circuit c;
        construire("Circuit simple", c);
        Diagnostic d = resoudre(c);
        verifieVrai("Interrupteur ferme : le courant circule", d.boucleFermee, true);
        for (Elem& e : c.elems)
            if (e.kind == Kind::Interrupteur) e.ferme = false;
        d = resoudre(c);
        verifieVrai("Interrupteur ouvert : plus aucun courant", d.boucleFermee, false);
        verifie("Interrupteur ouvert : I dans la lampe",
                std::fabs(premier(c, Kind::Lampe)->i), 0.0, 1e-6, "A");
    }
    {
        Circuit c;
        construire("Court-circuit", c);
        const Diagnostic d = resoudre(c);
        verifieVrai("Court-circuit detecte", d.courtCircuit, true);
        verifieVrai("Court-circuit : la pile debite plus de 10 A",
                    std::fabs(premier(c, Kind::Pile)->i) > 10.0, true);
    }

    // ── Appareils de mesure ──────────────────────────────────────────────
    {
        Circuit c;
        construire("Loi d'Ohm", c);
        resoudre(c);
        verifie("Amperemetre en serie : lecture", std::fabs(premier(c, Kind::Amperemetre)->i) * 1000.0,
                100.0, 0.5, "mA");
        verifie("Voltmetre en derivation : lecture", std::fabs(premier(c, Kind::Voltmetre)->u),
                6.0, 0.02, "V");
    }

    // ── Guirlande : une lampe grillée éteint toute la série ──────────────
    {
        Circuit c;
        construire("Guirlande", c);
        resoudre(c);
        verifieVrai("Guirlande : toutes les lampes brillent", premier(c, Kind::Lampe, 3)->allume, true);
        for (Elem& e : c.elems)
            if (e.kind == Kind::Lampe) { e.grillee = true; break; }
        const Diagnostic d = resoudre(c);
        verifieVrai("Guirlande : une lampe grillee eteint tout", d.boucleFermee, false);
    }

    // ── Lampe : conducteur non ohmique ───────────────────────────────────
    {
        const double froid = resistanceLampe(6.0, 0.6, 0.0);
        const double chaud = resistanceLampe(6.0, 0.6, 0.6);
        verifie("Lampe 6 V / 0,6 W : resistance a chaud", chaud, 60.0, 0.01, "ohm");
        verifieVrai("Lampe : la resistance a froid est bien plus faible", froid < chaud * 0.5, true);
    }

    // ── Écriture française des grandeurs ─────────────────────────────────
    {
        ++total;
        const std::string i = fmtIntensite(0.19);
        const std::string u = fmtTension(1.8);
        const bool ok = (i == "190 mA") && (u == "1,80 V");
        if (!ok) { ++echecs; std::printf(" ECHEC format : \"%s\" / \"%s\"\n", i.c_str(), u.c_str()); }
        else       std::printf("  ok   %-58s %10s\n", "Ecriture francaise : 0,19 A et 1,8 V", "190 mA");
    }

    // ── Multimètre : calibres, bornes, fusible, ohmmètre ─────────────────
    {
        Circuit c;
        construire("Mesure au multimètre", c);
        resoudre(c);
        const Elem* mm = premier(c, Kind::Multimetre);
        verifie("Multimetre en serie (10 A) : lecture", mm->lecture * 1000.0, 190.0, 3.0, "mA");
        verifieVrai("Multimetre : pas de depassement sur le calibre 10 A", mm->depassement, false);
        verifieVrai("Multimetre : fusible intact", mm->grillee, false);
    }
    {
        // Calibre 2 V choisi pour mesurer 12,8 V : l'ecran doit afficher « 1. »
        Circuit c;
        c.ajouter(faire(Kind::Pile, {0, 2}, {0, 0}, 12.8, 0.0));
        c.ajouter(faire(Kind::Fil, {0, 0}, {4, 0}));
        c.ajouter(faire(Kind::Resistance, {4, 0}, {4, 2}, 100.0));
        c.ajouter(faire(Kind::Fil, {4, 2}, {0, 2}));
        Elem mm = faire(Kind::Multimetre, {4, 0}, {4, 2});
        mm.mode = ModeMultimetre::Tension;
        mm.borne = BorneMultimetre::VOhm;
        mm.calibre = 2.0;
        c.ajouter(mm);
        resoudre(c);
        const Elem* lu = premier(c, Kind::Multimetre);
        verifie("Voltmetre : tension mesuree", lu->lecture, 12.8, 0.02, "V");
        verifieVrai("Calibre 2 V pour 12,8 V : affichage « 1. »", lu->depassement, true);
    }
    {
        // Amperemetre branche EN DERIVATION aux bornes de la pile : fusible.
        Circuit c;
        c.ajouter(faire(Kind::Pile, {0, 2}, {0, 0}, 4.5, 0.0));
        c.ajouter(faire(Kind::Fil, {0, 0}, {4, 0}));
        c.ajouter(faire(Kind::Lampe, {4, 0}, {4, 2}, 6.0, 0.6));
        c.ajouter(faire(Kind::Fil, {4, 2}, {0, 2}));
        Elem mm = faire(Kind::Multimetre, {4, 0}, {4, 2});
        mm.mode = ModeMultimetre::Intensite;
        mm.borne = BorneMultimetre::DixAmperes;
        mm.calibre = 10.0;
        c.ajouter(mm);
        resoudre(c);
        verifieVrai("Amperemetre en derivation : le fusible fond", premier(c, Kind::Multimetre)->grillee, true);
        resoudre(c);
        verifie("Fusible fondu : plus aucun courant dans l'appareil",
                std::fabs(premier(c, Kind::Multimetre)->i), 0.0, 1e-6, "A");
    }
    {
        // Ohmmetre hors circuit sur une resistance de 82 ohm.
        Circuit c;
        c.ajouter(faire(Kind::Resistance, {0, 0}, {4, 0}, 82.0));
        Elem mm = faire(Kind::Multimetre, {0, 0}, {4, 0});
        mm.mode = ModeMultimetre::Resistance;
        mm.calibre = 2000.0;
        c.ajouter(mm);
        resoudre(c);
        verifie("Ohmmetre hors circuit : lecture", premier(c, Kind::Multimetre)->lecture, 82.0, 0.1, "ohm");
    }
    {
        // Deux resistances en parallele vues par l'ohmmetre : 100 // 100 = 50.
        Circuit c;
        c.ajouter(faire(Kind::Resistance, {0, 0}, {4, 0}, 100.0));
        c.ajouter(faire(Kind::Resistance, {0, 0}, {4, 0}, 100.0));
        Elem mm = faire(Kind::Multimetre, {0, 0}, {4, 0});
        mm.mode = ModeMultimetre::Resistance;
        mm.calibre = 2000.0;
        c.ajouter(mm);
        resoudre(c);
        verifie("Ohmmetre : deux resistances de 100 ohm en derivation",
                premier(c, Kind::Multimetre)->lecture, 50.0, 0.1, "ohm");
    }

    // ── Secteur, puissance et énergie (3e ch.6, 3e ch.7, 4e ch.10) ───────
    {
        Circuit c;
        construire("Ventilateur", c);
        resoudre(c);
        verifie("Ventilateur 1 kohm sous 220 V : I", std::fabs(premier(c, Kind::Moteur)->i), 0.22, 1e-4, "A");
    }
    {
        Circuit c;
        construire("Applications", c);
        resoudre(c);
        verifie("Lave-linge 46 ohm sous 230 V : I", std::fabs(premier(c, Kind::Resistance, 0)->i), 5.0, 1e-3, "A");
        verifie("Smartphone 20 ohm sous 5 V : I", std::fabs(premier(c, Kind::Resistance, 1)->i) * 1000.0,
                250.0, 0.1, "mA");
        verifie("Bouilloire 115 ohm sous 230 V : I", std::fabs(premier(c, Kind::Resistance, 2)->i), 2.0, 1e-3, "A");
    }
    {
        Circuit c;
        construire("Puissance, énergie", c);
        resoudre(c);
        const double p = std::fabs(premier(c, Kind::Resistance, 0)->p);
        verifie("Seche-cheveux sur 230 V : P (wattmetre)", p, 2000.0, 0.5, "W");
        verifie("Seche-cheveux seul : P fournie par la prise", puissanceFournie(c), 2000.0, 0.5, "W");
        verifie("Seche-cheveux 15 min : E en kWh", energieKWh(p, 0.25), 0.5, 1e-3, "kWh");
        verifie("Seche-cheveux 15 min : E en joules", energieJoules(p, 900.0) / 1e6, 1.8, 1e-3, "MJ");
        verifie("Seche-cheveux 15 min : cout a 0,20 euro/kWh", energieKWh(p, 0.25) * 0.20, 0.10, 1e-3, "EUR");
        for (Elem& e : c.elems)
            if (e.kind == Kind::Interrupteur) e.ferme = true;
        resoudre(c);
        verifie("Trois appareils en marche : P fournie = somme", puissanceFournie(c), 3010.0, 1.0, "W");
    }
    {
        ++total;
        const std::string e = fmtEnergie(1.8e6);
        const std::string eur = fmtEuros(0.10);
        const std::string centime = fmtEuros(0.002);
        const bool ok = (e == "0,500 kWh") && (eur == "0,10 \xE2\x82\xAC") && (centime == "0,002 \xE2\x82\xAC");
        if (!ok) { ++echecs; std::printf(" ECHEC format : \"%s\" / \"%s\" / \"%s\"\n", e.c_str(), eur.c_str(), centime.c_str()); }
        else       std::printf("  ok   %-58s %10s\n", "Ecriture : 1,8 MJ = 0,500 kWh, 0,10 euro", "0,500 kWh");
    }
    {
        Circuit c;
        construire("Installation domestique", c);
        resoudre(c);
        verifie("Installation : intensite totale au disjoncteur", std::fabs(premier(c, Kind::Prise)->i),
                7.26, 0.02, "A");
        verifie("Installation : le plafonnier recoit 230 V", std::fabs(premier(c, Kind::Lampe)->u), 230.0, 0.01, "V");
        verifieVrai("Installation : le disjoncteur tient a 7,26 A", premier(c, Kind::Prise)->grillee, false);
        for (Elem& e : c.elems)
            if (e.kind == Kind::Interrupteur) e.ferme = true;   // le radiateur en plus
        const Diagnostic d = resoudre(c);
        verifieVrai("Radiateur en plus (18 A) : le disjoncteur declenche", premier(c, Kind::Prise)->grillee, true);
        verifieVrai("Disjoncteur declenche : plus aucun courant", d.boucleFermee, false);
    }
    {
        Circuit c;                                   // prise en court-circuit franc
        c.ajouter(faire(Kind::Prise, {0, 2}, {0, 0}, 230.0, 16.0));
        c.ajouter(faire(Kind::Fil, {0, 0}, {4, 0}));
        c.ajouter(faire(Kind::Fil, {4, 0}, {4, 2}));
        c.ajouter(faire(Kind::Fil, {4, 2}, {0, 2}));
        const Diagnostic d = resoudre(c);
        verifieVrai("Prise en court-circuit : le disjoncteur declenche", premier(c, Kind::Prise)->grillee, true);
        verifieVrai("Prise en court-circuit : calcul resolu", d.singulier, false);
    }

    // ── Conducteurs et isolants, fusible (6e ch.13) ──────────────────────
    {
        Circuit c;
        construire("Test conducteur", c);
        Elem* materiau = nullptr;
        for (Elem& e : c.elems)
            if (e.kind == Kind::Materiau) materiau = &e;
        double iCuivre = 0;
        for (int k = 0; k < nbMateriaux(); ++k) {
            materiau->valeur = k;
            resoudre(c);
            const Elem* lampe = premier(c, Kind::Lampe);
            const std::string nom = nomMateriau(k);
            if (k == 0) iCuivre = std::fabs(lampe->i);
            verifieVrai("Materiau " + nom + " : la lampe brille", lampe->allume, estConducteur(k));
            if (!estConducteur(k))
                verifie("Materiau " + nom + " : aucun courant", std::fabs(lampe->i) * 1000.0, 0.0, 1e-3, "mA");
            if (nom == "graphite")
                verifieVrai("Graphite : moins de courant qu'avec le cuivre", std::fabs(lampe->i) < iCuivre, true);
        }
    }
    {
        Circuit c;
        construire("Fusible contre", c);
        resoudre(c);
        verifieVrai("Fusible 1 A : la lampe brille, le fusible tient",
                    premier(c, Kind::Lampe)->allume && !premier(c, Kind::Fusible)->grillee, true);
        for (Elem& e : c.elems)
            if (e.kind == Kind::Interrupteur) e.ferme = true;
        const Diagnostic d = resoudre(c);
        verifieVrai("Court-circuit par K2 : le fusible fond", premier(c, Kind::Fusible)->grillee, true);
        verifie("Fusible fondu : la pile ne debite plus", std::fabs(premier(c, Kind::Pile)->i), 0.0, 1e-6, "A");
        verifieVrai("Fusible fondu : le circuit est ouvert", d.boucleFermee, false);
    }

    // ── Circuit mixte et caractéristique de la lampe ─────────────────────
    {
        Circuit c;
        construire("Circuit mixte", c);
        resoudre(c);
        verifie("Mixte : U aux bornes de R1", std::fabs(premier(c, Kind::Resistance, 0)->u), 4.5, 0.01, "V");
        verifie("Mixte : U aux bornes de R2", std::fabs(premier(c, Kind::Resistance, 1)->u), 4.5, 0.01, "V");
        verifie("Mixte : I dans R1", std::fabs(premier(c, Kind::Resistance, 0)->i) * 1000.0, 45.0, 0.1, "mA");
        verifie("Mixte : I dans R2", std::fabs(premier(c, Kind::Resistance, 1)->i) * 1000.0, 22.5, 0.1, "mA");
    }
    {
        Circuit c;
        construire("d'une lampe", c);
        Elem* pile = nullptr;
        for (Elem& e : c.elems)
            if (e.kind == Kind::Pile) pile = &e;
        pile->valeur = 1.0;
        resoudre(c);
        const Elem* lampe = premier(c, Kind::Lampe);
        const double rFaible = std::fabs(lampe->u / lampe->i);
        pile->valeur = 6.0;
        resoudre(c);
        const double rNominal = std::fabs(lampe->u / lampe->i);
        verifie("Lampe sous 6 V : R = 60 ohm", rNominal, 60.0, 0.5, "ohm");
        verifieVrai("Lampe : R sous 1 V bien plus faible (non ohmique)", rFaible < 0.6 * rNominal, true);
    }

    // ── Montage en cours de construction : jamais « impossible à résoudre » ─
    {
        Circuit c;
        construire("Circuit simple", c);
        c.ajouter(faire(Kind::Fil, {20, 0}, {24, 0}));     // fil posé, pas encore raccordé
        c.ajouter(faire(Kind::Fil, {24, 0}, {24, 3}));
        const Diagnostic d = resoudre(c);
        verifieVrai("Fil en l'air : le montage reste calculable", d.singulier, false);
        verifieVrai("Fil en l'air : la lampe brille toujours", premier(c, Kind::Lampe)->allume, true);
    }

    // ── Édition : quart de tour et fil orthogonal (§ 6.1) ────────────────
    {
        int defauts = 0;
        for (int ax = -2; ax <= 2; ++ax) for (int ay = -2; ay <= 2; ++ay)
        for (int bx = -3; bx <= 3; ++bx) for (int by = -3; by <= 3; ++by) {
            Elem e = faire(Kind::Resistance, {ax, ay}, {bx, by});
            Elem f = e;
            const int l2 = (bx - ax) * (bx - ax) + (by - ay) * (by - ay);
            for (int k = 0; k < 4; ++k) {
                tourner(f);
                const int dx = f.b.x - f.a.x, dy = f.b.y - f.a.y;
                if (dx * dx + dy * dy != l2) ++defauts;
            }
            if (f.a != e.a || f.b != e.b) ++defauts;
        }
        verifie("Quart de tour : longueur gardee, 4 quarts = identite", defauts, 0, 0);

        Elem h = faire(Kind::Lampe, {0, 0}, {4, 0});
        tourner(h);
        verifieVrai("Quart de tour : une lampe horizontale devient verticale",
                    h.a.x == h.b.x && std::abs(h.b.y - h.a.y) == 4, true);

        const Pt coude = coudeDuFil({0, 0}, {3, 2});
        verifieVrai("Fil de (0,0) a (3,2) : coude en (3,0)", coude == Pt{3, 0}, true);
        verifieVrai("Fil droit : pas de coude", coudeDuFil({0, 0}, {0, 5}) == Pt{0, 5}, true);
    }

    // ── Aller-retour sur disque : un montage rouvert doit être identique ──
    {
        Circuit avant;
        construire("cinq lampes", avant);
        Elem materiau = faire(Kind::Materiau, {30, 0}, {34, 0}, 4);      // bois sec
        Elem prise    = faire(Kind::Prise, {30, 4}, {34, 4}, 220.0, 10.0);
        Elem fusible  = faire(Kind::Fusible, {30, 8}, {34, 8}, 0.5);
        avant.ajouter(materiau);
        avant.ajouter(prise);
        avant.ajouter(fusible);
        std::string erreur;
        ++total;
        Circuit apres;
        if (!enregistrer(avant, "/tmp/crocelec_test.croc", erreur) ||
            !ouvrir(apres, "/tmp/crocelec_test.croc", erreur)) {
            ++echecs;
            std::printf(" ECHEC enregistrement/ouverture : %s\n", erreur.c_str());
        } else {
            bool identique = (avant.elems.size() == apres.elems.size());
            for (std::size_t k = 0; identique && k < avant.elems.size(); ++k) {
                const Elem& x = avant.elems[k];
                const Elem& y = apres.elems[k];
                identique = (x.kind == y.kind && x.a == y.a && x.b == y.b && x.nom == y.nom &&
                             std::fabs(x.valeur - y.valeur) < 1e-9 &&
                             std::fabs(x.valeur2 - y.valeur2) < 1e-9 &&
                             x.ferme == y.ferme && x.grillee == y.grillee);
            }
            if (!identique) { ++echecs; std::printf(" ECHEC montage rouvert different de l'original\n"); }
            else std::printf("  ok   %-58s %10s\n", "Montage .croc enregistre puis rouvert a l'identique", "identique");
        }
        resoudre(apres);
        verifie("Montage rouvert : U aux bornes de L1", std::fabs(premier(apres, Kind::Lampe)->u),
                1.80, 0.01, "V");
    }

    std::printf("\n%d verifications, %d echec(s)\n", total, echecs);
    return echecs == 0 ? 0 : 1;
}
