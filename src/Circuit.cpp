#include "Circuit.h"

#include <algorithm>
#include <functional>
#include <map>
#include <numeric>

namespace croc {

const char* nomKind(Kind k)
{
    switch (k) {
    case Kind::Fil:          return "Fil";
    case Kind::Pile:         return "Pile";
    case Kind::Interrupteur: return "Interrupteur";
    case Kind::Lampe:        return "Lampe";
    case Kind::Resistance:   return "Résistance";
    case Kind::Moteur:       return "Moteur";
    case Kind::Del:          return "DEL";
    case Kind::Voltmetre:    return "Voltmètre";
    case Kind::Amperemetre:  return "Ampèremètre";
    case Kind::Multimetre:   return "Multimètre";
    default:                 return "?";
    }
}

const char* prefixeKind(Kind k)
{
    switch (k) {
    case Kind::Fil:          return "f";
    case Kind::Pile:         return "G";
    case Kind::Interrupteur: return "K";
    case Kind::Lampe:        return "L";
    case Kind::Resistance:   return "R";
    case Kind::Moteur:       return "M";
    case Kind::Del:          return "D";
    case Kind::Voltmetre:    return "V";
    case Kind::Amperemetre:  return "A";
    case Kind::Multimetre:   return "MM";
    default:                 return "X";
    }
}

const char* aideKind(Kind k)
{
    switch (k) {
    case Kind::Fil:          return "Relie les dipôles. Conducteur : il laisse passer le courant.";
    case Kind::Pile:         return "Générateur : il fournit l'énergie électrique au circuit.";
    case Kind::Interrupteur: return "Ouvre ou ferme le circuit. Clique dessus pour le manœuvrer.";
    case Kind::Lampe:        return "Récepteur : convertit l'électricité en lumière et en chaleur.";
    case Kind::Resistance:   return "Conducteur ohmique : U = R x I. Il s'oppose au passage du courant.";
    case Kind::Moteur:       return "Récepteur : convertit l'électricité en mouvement.";
    case Kind::Del:          return "Diode électroluminescente : le courant ne passe que dans un sens.";
    case Kind::Voltmetre:    return "Mesure la tension. Il se branche EN DÉRIVATION, aux bornes du dipôle.";
    case Kind::Amperemetre:  return "Mesure l'intensité. Il se branche EN SÉRIE, dans la boucle.";
    case Kind::Multimetre:   return "Voltmètre, ampèremètre et ohmmètre en un seul appareil. Choisis les bornes et le calibre.";
    default:                 return "";
    }
}

int Circuit::ajouter(const Elem& e)
{
    Elem copie = e;
    if (copie.nom.empty()) copie.nom = nomDisponible(copie.kind);
    elems.push_back(copie);
    construireNetlist();
    return (int)elems.size() - 1;
}

void Circuit::supprimer(int index)
{
    if (index < 0 || index >= (int)elems.size()) return;
    elems.erase(elems.begin() + index);
    construireNetlist();
}

void Circuit::vider()
{
    elems.clear();
    construireNetlist();
}

std::string Circuit::nomDisponible(Kind k) const
{
    const std::string pref = prefixeKind(k);
    for (int n = 1; n < 1000; ++n) {
        std::string candidat = pref + std::to_string(n);
        bool pris = false;
        for (const Elem& e : elems)
            if (e.nom == candidat) { pris = true; break; }
        if (!pris) return candidat;
    }
    return pref;
}

void Circuit::construireNetlist()
{
    net = Netlist{};
    if (elems.empty()) return;

    // Union-find sur les points de grille : deux extrémités au même point,
    // c'est le même nœud ; un fil fusionne ses deux extrémités.
    struct Cmp {
        bool operator()(Pt l, Pt r) const {
            return (l.x != r.x) ? (l.x < r.x) : (l.y < r.y);
        }
    };
    std::map<Pt, int, Cmp> indexDuPoint;
    std::vector<Pt> points;

    auto idPoint = [&](Pt p) {
        auto it = indexDuPoint.find(p);
        if (it != indexDuPoint.end()) return it->second;
        int id = (int)points.size();
        points.push_back(p);
        indexDuPoint.emplace(p, id);
        return id;
    };

    for (Elem& e : elems) { idPoint(e.a); idPoint(e.b); }

    std::vector<int> parent(points.size());
    std::iota(parent.begin(), parent.end(), 0);
    std::function<int(int)> trouver = [&](int x) {
        while (parent[x] != x) { parent[x] = parent[parent[x]]; x = parent[x]; }
        return x;
    };
    auto unir = [&](int x, int y) {
        x = trouver(x); y = trouver(y);
        if (x != y) parent[y] = x;
    };

    // Seuls les fils fusionnent des points distincts : un fil est un morceau
    // de cuivre, pas un dipôle. Tous les autres gardent deux nœuds séparés,
    // même l'interrupteur fermé (le solveur lui donnera une résistance
    // négligeable) — c'est ce qui permet d'afficher le courant qui le traverse.
    for (const Elem& e : elems)
        if (e.kind == Kind::Fil) unir(idPoint(e.a), idPoint(e.b));

    // Jonction en T : une extrémité posée AU MILIEU d'un fil est reliée à ce
    // fil. Sans cela, un élève qui branche une dérivation sur un fil déjà
    // tendu obtient un circuit ouvert sans comprendre pourquoi — alors que sur
    // la paillasse, la pince crocodile mord bien le fil au milieu.
    for (const Elem& fil : elems) {
        if (fil.kind != Kind::Fil) continue;
        const int dx = fil.b.x - fil.a.x, dy = fil.b.y - fil.a.y;
        if (dx == 0 && dy == 0) continue;
        for (std::size_t i = 0; i < points.size(); ++i) {
            const Pt p = points[i];
            if (p == fil.a || p == fil.b) continue;
            const int ex = p.x - fil.a.x, ey = p.y - fil.a.y;
            if (ex * dy - ey * dx != 0) continue;                 // pas aligné
            const int t = ex * dx + ey * dy;                      // projection
            if (t <= 0 || t >= dx * dx + dy * dy) continue;       // hors segment
            unir((int)i, idPoint(fil.a));
        }
    }

    // Numérotation compacte des nœuds.
    std::map<int, int> numero;
    for (std::size_t i = 0; i < points.size(); ++i) {
        int r = trouver((int)i);
        if (numero.find(r) == numero.end()) {
            int n = (int)numero.size();
            numero[r] = n;
            net.position.push_back(points[r]);
        }
    }
    net.nbNoeuds = (int)numero.size();
    net.degre.assign(net.nbNoeuds, 0);
    net.degreTotal.assign(net.nbNoeuds, 0);

    for (Elem& e : elems) {
        e.na = numero[trouver(idPoint(e.a))];
        e.nb = numero[trouver(idPoint(e.b))];
        net.degreTotal[e.na]++;
        net.degreTotal[e.nb]++;
        if (e.kind != Kind::Fil) {
            net.degre[e.na]++;
            net.degre[e.nb]++;
        }
    }

    // Le point représentatif d'un nœud sert au dessin : on préfère un point
    // où plusieurs fils se rejoignent réellement, sinon la pastille de nœud
    // apparaît à un endroit où l'élève ne voit aucune jonction.
    std::map<std::pair<int, int>, int> compteParPoint;
    for (const Elem& e : elems) {
        compteParPoint[{e.a.x, e.a.y}]++;
        compteParPoint[{e.b.x, e.b.y}]++;
    }
    std::vector<int> meilleur(net.nbNoeuds, 0);
    for (const auto& kv : indexDuPoint) {
        int n = numero[trouver(kv.second)];
        int c = compteParPoint[{kv.first.x, kv.first.y}];
        if (c > meilleur[n]) {
            meilleur[n] = c;
            net.position[n] = kv.first;
        }
    }
}

bool Circuit::estNoeudDeDerivation(int noeud) const
{
    if (noeud < 0 || noeud >= net.nbNoeuds) return false;
    return net.degre[noeud] >= 3;
}

int Circuit::nbNoeudsDeDerivation() const
{
    int n = 0;
    for (int i = 0; i < net.nbNoeuds; ++i)
        if (estNoeudDeDerivation(i)) ++n;
    return n;
}

bool Circuit::estEnSerie() const { return nbNoeudsDeDerivation() == 0; }

std::vector<std::vector<int>> Circuit::groupesParalleles() const
{
    std::map<std::pair<int, int>, std::vector<int>> parPaire;
    for (std::size_t i = 0; i < elems.size(); ++i) {
        const Elem& e = elems[i];
        if (e.kind == Kind::Fil) continue;
        if (e.na < 0 || e.nb < 0 || e.na == e.nb) continue;
        parPaire[{std::min(e.na, e.nb), std::max(e.na, e.nb)}].push_back((int)i);
    }
    std::vector<std::vector<int>> groupes;
    for (auto& kv : parPaire)
        if (kv.second.size() >= 2) groupes.push_back(kv.second);
    return groupes;
}

} // namespace croc
