#include "App.h"

#include "Format.h"

#include <algorithm>
#include <cmath>

namespace croc {

Kind kindDeLOutil(Outil o)
{
    switch (o) {
    case Outil::Fil:          return Kind::Fil;
    case Outil::Pile:         return Kind::Pile;
    case Outil::Interrupteur: return Kind::Interrupteur;
    case Outil::Lampe:        return Kind::Lampe;
    case Outil::Resistance:   return Kind::Resistance;
    case Outil::Moteur:       return Kind::Moteur;
    case Outil::Del:          return Kind::Del;
    case Outil::Voltmetre:    return Kind::Voltmetre;
    case Outil::Amperemetre:  return Kind::Amperemetre;
    case Outil::Multimetre:   return Kind::Multimetre;
    case Outil::Prise:        return Kind::Prise;
    case Outil::Fusible:      return Kind::Fusible;
    case Outil::Materiau:     return Kind::Materiau;
    default:                  return Kind::COUNT;
    }
}

void App::note(const std::string& message)
{
    if (!journal.empty() && journal.back() == message) return; // pas de rabâchage
    journal.push_back(message);
    while (journal.size() > 200) journal.pop_front();
}

void App::memoriser()
{
    historique.push_back(circuit);
    if (historique.size() > 50) historique.erase(historique.begin());
}

void App::annuler()
{
    if (historique.empty()) return;
    circuit = historique.back();
    historique.pop_back();
    selection = -1;
    recalculer();
}

void App::relever()
{
    if (selection < 0 || selection >= (int)circuit.elems.size()) {
        note("Choisis d'abord le dipôle à relever dans le schéma.");
        return;
    }
    const Elem& e = circuit.elems[selection];
    Releve r;
    r.dipole = e.nom;
    r.u = std::fabs(e.u);
    r.i = std::fabs(e.i);
    r.r = (e.rEff > 0 && e.rEff < 1e6) ? e.rEff : 0.0;
    r.p = std::fabs(e.p);
    releves.push_back(r);
    note("Relevé n°" + std::to_string(releves.size()) + " sur " + e.nom + " : U = " +
         fmtTension(r.u) + ", I = " + fmtIntensite(r.i));
}

void App::tournerSelection()
{
    if (selection < 0 || selection >= (int)circuit.elems.size()) return;
    memoriser();
    tourner(circuit.elems[selection]);
    recalculer();
}

void App::avancerCompteur(double dtReel)
{
    if (!compteurEnMarche) return;
    const double dt = dtReel * acceleration;
    compteurSecondes += dt;
    compteurJoules   += energieJoules(std::max(0.0, puissanceFournie(circuit)), dt);
}

void App::recalculer()
{
    if (!simulationActive) return;
    diag = resoudre(circuit);
    for (const std::string& m : diag.messages) note(m);
}

} // namespace croc
