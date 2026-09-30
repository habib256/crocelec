// Panels.cpp — Palette, Propriétés, Mesures, Lois, Journal, Bibliothèque.
//
// Le panneau Lois est le cœur pédagogique : il ne se contente pas d'afficher
// des nombres, il énonce la loi du programme et montre qu'elle est vérifiée
// sur le montage de l'élève (§ 6.5 du cahier des charges).
#include "App.h"
#include "Format.h"

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <vector>

namespace croc {
namespace {

const ImVec4 kVert  = ImVec4(0.09f, 0.45f, 0.32f, 1.0f);
const ImVec4 kRouge = ImVec4(0.72f, 0.19f, 0.12f, 1.0f);
const ImVec4 kGris  = ImVec4(0.45f, 0.48f, 0.52f, 1.0f);

struct EntreePalette {
    Outil outil;
    const char* etiquette;
    int niveau;          // 6 = 6e … 3 = 3e : première classe où le dipôle sert
};

// Familles, comme la barre de Crocodile Clips, mais réduites à ce que le
// programme du collège utilise (§ 6.1 : catégories et filtre par niveau).
struct Famille {
    const char* nom;
    std::vector<EntreePalette> entrees;
};

const std::vector<Famille>& familles()
{
    static const std::vector<Famille> f = {
        {"Générateurs",           {{Outil::Pile, "Pile", 6}, {Outil::Prise, "Prise 230 V", 3}}},
        {"Commande",              {{Outil::Interrupteur, "Interrupteur", 6}}},
        {"Récepteurs",            {{Outil::Lampe, "Lampe", 6}, {Outil::Moteur, "Moteur", 5},
                                   {Outil::Del, "DEL", 5}, {Outil::Resistance, "Résistance", 4}}},
        {"Sécurité et matériaux", {{Outil::Materiau, "Matériau test", 6}, {Outil::Fusible, "Fusible", 6}}},
        {"Appareils de mesure",   {{Outil::Voltmetre, "Voltmètre", 4}, {Outil::Amperemetre, "Ampèremètre", 4},
                                   {Outil::Multimetre, "Multimètre", 4}}},
    };
    return f;
}

bool proche(double a, double b, double tolerance)
{
    const double echelle = std::max({std::fabs(a), std::fabs(b), 1e-9});
    return std::fabs(a - b) <= tolerance * echelle;
}

// Les énoncés du panneau Lois sont des phrases entières : sans repli, la
// moitié disparaît dès que le panneau est un peu étroit.
void verdict(bool ok, const char* texteOk, const char* texteNon)
{
    ImGui::PushStyleColor(ImGuiCol_Text, ok ? kVert : kRouge);
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted(ok ? texteOk : texteNon);
    ImGui::PopTextWrapPos();
    ImGui::PopStyleColor();
}

// Ligne d'information qui se replie plutôt que de se faire couper.
void ligne(const char* format, ...) IM_FMTARGS(1);
void ligne(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextV(format, args);
    ImGui::PopTextWrapPos();
    va_end(args);
}

} // namespace

// ── Palette ──────────────────────────────────────────────────────────────

namespace {

// Bouton d'outil, surligné quand il est actif. Rend vrai si on l'a choisi.
bool boutonOutil(App& app, Outil outil, const char* etiquette, ImVec2 taille)
{
    const bool actif = (app.outil == outil);
    if (actif) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
    const bool clic = ImGui::Button(etiquette, taille);
    if (actif) ImGui::PopStyleColor();
    if (clic) app.outil = outil;
    return clic;
}

void bulle(const char* titre, const char* texte)
{
    if (!ImGui::IsItemHovered()) return;
    ImGui::BeginTooltip();
    ImGui::TextUnformatted(titre);
    ImGui::Separator();
    ImGui::PushTextWrapPos(320.0f);
    ImGui::TextUnformatted(texte);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
}

} // namespace

void panneauPalette(App& app)
{
    ImGui::Begin("Palette");

    // En mode simple : boutons hauts (faciles à viser, y compris au doigt),
    // et pas de Multimètre — son panneau n'est visible qu'en mode complet,
    // le voltmètre et l'ampèremètre simplifiés suffisent (§ 6.3).
    const float hauteurBouton = app.reglages.modeSimple ? ImGui::GetFontSize() * 2.1f : 0.0f;

    // ── Les trois outils de la fiche : sélectionner, relier, effacer ─────
    const float tiers = (ImGui::GetContentRegionAvail().x - 2 * ImGui::GetStyle().ItemSpacing.x) / 3.0f;
    boutonOutil(app, Outil::Selection, "Sélection", ImVec2(tiers, hauteurBouton));
    bulle("Sélection", "Clique un dipôle pour le régler, glisse-le pour le déplacer. "
                       "Tire depuis l'extrémité d'un dipôle pour poser un fil.");
    ImGui::SameLine();
    boutonOutil(app, Outil::Fil, "Fil", ImVec2(tiers, hauteurBouton));
    bulle("Fil", aideKind(Kind::Fil));
    ImGui::SameLine();
    boutonOutil(app, Outil::Gomme, "Gomme", ImVec2(tiers, hauteurBouton));
    bulle("Gomme", "Efface le dipôle ou le fil que tu touches. Garde le bouton enfoncé pour en effacer plusieurs.");

    // ── Filtre par niveau ────────────────────────────────────────────────
    ImGui::Spacing();
    ImGui::TextDisabled("Niveau de la classe :");
    const struct { const char* libelle; int niveau; } niveaux[] = {
        {"6e", 6}, {"5e", 5}, {"4e", 4}, {"3e", 3}, {"Tout", 0}};
    for (const auto& n : niveaux) {
        if (&n != niveaux) ImGui::SameLine();
        const bool actif = (app.reglages.niveau == n.niveau);
        if (actif) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        if (ImGui::SmallButton(n.libelle)) app.reglages.niveau = n.niveau;
        if (actif) ImGui::PopStyleColor();
    }
    ImGui::Separator();

    // ── Familles de dipôles ──────────────────────────────────────────────
    for (const Famille& famille : familles()) {
        std::vector<const EntreePalette*> visibles;
        for (const EntreePalette& e : famille.entrees) {
            if (app.reglages.modeSimple && e.outil == Outil::Multimetre) continue;
            // Un élève de 4e voit aussi ce qui a servi en 6e et en 5e.
            if (app.reglages.niveau != 0 && e.niveau < app.reglages.niveau) continue;
            visibles.push_back(&e);
        }
        if (visibles.empty()) continue;

        if (!ImGui::CollapsingHeader(famille.nom, ImGuiTreeNodeFlags_DefaultOpen)) continue;
        for (const EntreePalette* e : visibles) {
            boutonOutil(app, e->outil, e->etiquette, ImVec2(-1, hauteurBouton));
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("%s  (dès la %de)", nomKind(kindDeLOutil(e->outil)), e->niveau);
                ImGui::Separator();
                ImGui::PushTextWrapPos(320.0f);
                ImGui::TextUnformatted(aideKind(kindDeLOutil(e->outil)));
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
        }
    }

    ImGui::Separator();
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("Choisis un dipôle, puis trace-le sur le schéma en glissant.");
    ImGui::TextDisabled("Tirer depuis l'extrémité d'un dipôle : un fil.");
    ImGui::TextDisabled("Clic sur un interrupteur : l'ouvrir ou le fermer.");
    ImGui::TextDisabled("R : tourner le dipôle choisi. Suppr ou Gomme : l'effacer.");
    ImGui::TextDisabled("Molette : zoom. Glisser dans le vide : déplacer la vue.");
    ImGui::PopTextWrapPos();
    ImGui::End();
}

// ── Propriétés ───────────────────────────────────────────────────────────

void panneauProprietes(App& app)
{
    ImGui::Begin("Propriétés");

    if (app.selection < 0 || app.selection >= (int)app.circuit.elems.size()) {
        ImGui::TextDisabled("Aucun dipôle sélectionné.");
        ImGui::Spacing();
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextDisabled("Choisis l'outil Sélection puis clique sur un dipôle du schéma pour régler sa valeur.");
        ImGui::PopTextWrapPos();
        ImGui::End();
        return;
    }

    Elem& e = app.circuit.elems[app.selection];
    ImGui::Text("%s  —  %s", e.nom.c_str(), nomKind(e.kind));
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", aideKind(e.kind));
    ImGui::PopTextWrapPos();
    ImGui::Separator();

    bool change = false;
    switch (e.kind) {
    case Kind::Pile: {
        float v = (float)e.valeur;
        if (ImGui::SliderFloat("Tension (V)", &v, 0.0f, 24.0f, "%.2f V")) { e.valeur = v; change = true; }
        ImGui::TextDisabled("Piles usuelles :");
        const double usuelles[] = {1.5, 4.5, 6.0, 9.0, 12.0};
        for (double u : usuelles) {
            ImGui::SameLine();
            char lib[16];
            std::snprintf(lib, sizeof lib, "%.1f V", u);
            if (ImGui::SmallButton(lib)) { e.valeur = u; change = true; }
        }
        float r = (float)e.valeur2;
        if (ImGui::SliderFloat("Résistance interne (ohm)", &r, 0.0f, 5.0f, "%.2f")) { e.valeur2 = r; change = true; }
        ImGui::TextDisabled("0 = pile idéale : les valeurs collent au calcul du cours.");
        break;
    }
    case Kind::Resistance: {
        float v = (float)e.valeur;
        if (ImGui::DragFloat("Résistance (ohm)", &v, 1.0f, 0.1f, 100000.0f, "%.1f")) { e.valeur = std::max(0.1f, v); change = true; }
        const double usuelles[] = {10, 33, 82, 100, 270, 350, 1000};
        for (double r : usuelles) {
            ImGui::SameLine(0.0f, 4.0f);
            char lib[24];
            std::snprintf(lib, sizeof lib, "%g", r);
            if (ImGui::SmallButton(lib)) { e.valeur = r; change = true; }
        }
        break;
    }
    case Kind::Moteur: {
        float v = (float)e.valeur;
        if (ImGui::DragFloat("Résistance (ohm)", &v, 1.0f, 0.5f, 10000.0f, "%.1f")) { e.valeur = std::max(0.5f, v); change = true; }
        break;
    }
    case Kind::Lampe: {
        float un = (float)e.valeur, pn = (float)e.valeur2;
        // Échelle logarithmique : de la lampe de poche (6 V) à l'ampoule
        // de la maison (230 V / 60 W) sur le même curseur.
        if (ImGui::SliderFloat("Tension nominale (V)", &un, 1.0f, 230.0f, "%.1f V", ImGuiSliderFlags_Logarithmic)) { e.valeur = un; change = true; }
        if (ImGui::SliderFloat("Puissance nominale (W)", &pn, 0.05f, 100.0f, "%.2f W", ImGuiSliderFlags_Logarithmic)) { e.valeur2 = pn; change = true; }
        ImGui::TextDisabled("Résistance nominale : %s", fmtResistance(e.valeur * e.valeur / std::max(1e-6, e.valeur2)).c_str());
        if (e.grillee) {
            ImGui::TextColored(kRouge, "Lampe grillée.");
            ImGui::SameLine();
            if (ImGui::SmallButton("Remplacer")) { e.grillee = false; change = true; }
        }
        break;
    }
    case Kind::Del: {
        float seuil = (float)e.valeur, imax = (float)(e.valeur2 * 1000.0);
        if (ImGui::SliderFloat("Tension de seuil (V)", &seuil, 1.5f, 3.5f, "%.2f V")) { e.valeur = seuil; change = true; }
        if (ImGui::SliderFloat("Intensité maximale (mA)", &imax, 5.0f, 100.0f, "%.0f mA")) { e.valeur2 = imax / 1000.0; change = true; }
        if (e.grillee) {
            ImGui::TextColored(kRouge, "DEL grillée : il lui faut une résistance de protection.");
            if (ImGui::SmallButton("Remplacer")) { e.grillee = false; change = true; }
        }
        break;
    }
    case Kind::Prise: {
        ImGui::TextColored(kRouge, "230 V : danger de mort. Simulation seulement.");
        ImGui::Text("Tension : %s", fmtTension(e.valeur).c_str());
        ImGui::SameLine();
        // 220 V : la valeur des anciens énoncés (ventilateur du 3e ch.6).
        if (ImGui::SmallButton("230 V")) { e.valeur = 230.0; change = true; }
        ImGui::SameLine();
        if (ImGui::SmallButton("220 V")) { e.valeur = 220.0; change = true; }
        ImGui::Text("Disjoncteur : %s", fmtIntensite(e.valeur2).c_str());
        const double calibres[] = {2, 10, 16, 20, 32};
        for (double a : calibres) {
            ImGui::SameLine(0.0f, 4.0f);
            char lib[16];
            std::snprintf(lib, sizeof lib, "%g A", a);
            if (ImGui::SmallButton(lib)) { e.valeur2 = a; change = true; }
        }
        if (e.grillee) {
            ImGui::TextColored(kRouge, "Disjoncteur déclenché : plus rien n'est alimenté.");
            if (ImGui::SmallButton("Réarmer le disjoncteur")) { e.grillee = false; change = true; }
            ImGui::TextDisabled("Supprime d'abord la cause (court-circuit, appareil de trop).");
        }
        break;
    }
    case Kind::Fusible: {
        float imax = (float)e.valeur;
        if (ImGui::SliderFloat("Intensité maximale (A)", &imax, 0.05f, 20.0f, "%.2f A", ImGuiSliderFlags_Logarithmic)) {
            e.valeur = imax;
            change = true;
        }
        const double usuels[] = {0.1, 0.5, 1, 5, 10};
        for (double a : usuels) {
            ImGui::SameLine(0.0f, 4.0f);
            if (ImGui::SmallButton(fmtIntensite(a).c_str())) { e.valeur = a; change = true; }
        }
        if (e.grillee) {
            ImGui::TextColored(kRouge, "Fusible fondu : le circuit est ouvert.");
            if (ImGui::SmallButton("Remplacer le fusible")) { e.grillee = false; change = true; }
        }
        break;
    }
    case Kind::Materiau: {
        int index = (int)e.valeur;
        for (int k = 0; k < nbMateriaux(); ++k) {
            if (k) ImGui::SameLine();
            if (ImGui::RadioButton(nomMateriau(k), &index, k)) { e.valeur = k; change = true; }
        }
        // Le verdict n'est affiché qu'une fois le circuit alimenté : c'est
        // la lampe (ou l'ampèremètre) qui doit répondre, pas le logiciel.
        if (app.diag.boucleFermee || std::fabs(e.u) > 1e-3) {
            if (estConducteur(index))
                ImGui::TextColored(kVert, "Matériau %s : le courant passe, c'est un conducteur électrique.", nomMateriau(index));
            else
                ImGui::TextColored(kRouge, "Matériau %s : le courant ne passe pas, c'est un isolant électrique.", nomMateriau(index));
        } else {
            ImGui::TextDisabled("Place-le dans un circuit alimenté pour le tester.");
        }
        break;
    }
    case Kind::Interrupteur: {
        if (ImGui::Checkbox("Fermé", &e.ferme)) change = true;
        ImGui::TextDisabled(e.ferme ? "Circuit fermé : le courant peut circuler."
                                    : "Circuit ouvert : aucun courant ne circule.");
        break;
    }
    default:
        ImGui::TextDisabled("Ce dipôle n'a pas de réglage.");
        break;
    }

    ImGui::Separator();
    ImGui::Text("Mesures");
    ImGui::BulletText("U = %s", fmtTension(std::fabs(e.u)).c_str());
    ImGui::BulletText("I = %s", fmtIntensite(std::fabs(e.i)).c_str());
    if (!estGenerateur(e.kind) && e.rEff > 0 && e.rEff < 1e6)
        ImGui::BulletText("R = %s", fmtResistance(e.rEff).c_str());
    ImGui::BulletText("P = %s", fmtPuissance(std::fabs(e.p)).c_str());

    ImGui::Separator();
    // Au doigt, ni touche R ni clic droit : les deux gestes ont leur bouton.
    if (ImGui::Button("Tourner")) {
        app.tournerSelection();
        ImGui::End();
        return;
    }
    ImGui::SameLine();
    if (ImGui::Button("Supprimer ce dipôle")) {
        app.memoriser();
        app.circuit.supprimer(app.selection);
        app.selection = -1;
        change = true;
    }

    if (change) app.recalculer();
    ImGui::End();
}

// ── Mesures ──────────────────────────────────────────────────────────────

void panneauMesures(App& app)
{
    ImGui::Begin("Mesures");

    if (app.diag.courtCircuit)
        ImGui::TextColored(kRouge, "Court-circuit ! Le générateur débite un courant énorme.");
    else if (!app.diag.boucleFermee)
        ImGui::TextColored(kGris, "Aucun courant : la boucle n'est pas fermée.");
    else
        ImGui::TextColored(kVert, "Le courant circule.");

    ImGui::Separator();

    const ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                  ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("mesures", 5, flags)) {
        ImGui::TableSetupColumn("Dipôle");
        ImGui::TableSetupColumn("U");
        ImGui::TableSetupColumn("I");
        ImGui::TableSetupColumn("R");
        ImGui::TableSetupColumn("P");
        ImGui::TableHeadersRow();

        for (int k = 0; k < (int)app.circuit.elems.size(); ++k) {
            const Elem& e = app.circuit.elems[k];
            if (e.kind == Kind::Fil) continue;
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            const bool selectionne = (k == app.selection);
            if (ImGui::Selectable((e.nom + " (" + nomKind(e.kind) + ")").c_str(), selectionne,
                                  ImGuiSelectableFlags_SpanAllColumns))
                app.selection = k;
            ImGui::TableNextColumn(); ImGui::TextUnformatted(fmtTension(std::fabs(e.u)).c_str());
            ImGui::TableNextColumn(); ImGui::TextUnformatted(fmtIntensite(std::fabs(e.i)).c_str());
            ImGui::TableNextColumn();
            if (estGenerateur(e.kind)) ImGui::TextDisabled("—");
            else if (e.rEff > 0 && e.rEff < 1e6) ImGui::TextUnformatted(fmtResistance(e.rEff).c_str());
            else ImGui::TextDisabled("—");
            ImGui::TableNextColumn(); ImGui::TextUnformatted(fmtPuissance(std::fabs(e.p)).c_str());
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

// ── Lois ─────────────────────────────────────────────────────────────────

void panneauLois(App& app)
{
    ImGui::Begin("Lois");

    Circuit& c = app.circuit;
    if (c.elems.empty()) {
        ImGui::TextDisabled("Construis un circuit : les lois du programme s'y vérifieront toutes seules.");
        ImGui::End();
        return;
    }

    const int noeuds = c.nbNoeudsDeDerivation();
    if (noeuds == 0) {
        ImGui::TextColored(kVert, "Circuit en série");
        ImGui::TextDisabled("Une seule boucle : le courant n'a qu'un seul chemin.");
    } else {
        ImGui::TextColored(kVert, "Circuit avec dérivation");
        ImGui::TextDisabled("%d nœud%s : le courant se divise.", noeuds, noeuds > 1 ? "s" : "");
    }
    ImGui::Separator();

    // ── Unicité de l'intensité en série ──────────────────────────────────
    if (noeuds == 0 && app.diag.boucleFermee) {
        double imin = 1e30, imax = -1e30;
        int nb = 0;
        for (const Elem& e : c.elems) {
            if (e.kind == Kind::Fil || e.kind == Kind::Voltmetre) continue;
            const double v = std::fabs(e.i);
            imin = std::min(imin, v);
            imax = std::max(imax, v);
            ++nb;
        }
        if (nb > 0) {
            ImGui::Text("Unicité de l'intensité");
            ImGui::Indent();
            ligne("I = %s en tout point de la boucle", fmtIntensite(imax).c_str());
            verdict(proche(imin, imax, 1e-3), "L'intensité est bien la même partout.",
                    "Les intensités diffèrent : vérifie le montage.");
            ImGui::Unindent();
            ImGui::Spacing();
        }

        // ── Additivité des tensions ──────────────────────────────────────
        double uGen = 0, uRecepteurs = 0;
        bool aGenerateur = false;
        for (const Elem& e : c.elems) {
            if (estGenerateur(e.kind)) { uGen += std::fabs(e.u); aGenerateur = true; }
            else if (e.kind == Kind::Lampe || e.kind == Kind::Resistance ||
                     e.kind == Kind::Moteur || e.kind == Kind::Del || e.kind == Kind::Materiau)
                uRecepteurs += std::fabs(e.u);
        }
        if (aGenerateur) {
            ImGui::Text("Additivité des tensions");
            ImGui::Indent();
            ligne("U générateur = %s", fmtTension(uGen).c_str());
            ligne("somme des U des récepteurs = %s", fmtTension(uRecepteurs).c_str());
            verdict(proche(uGen, uRecepteurs, 2e-2),
                    "La tension du générateur se partage entre les récepteurs.",
                    "Écart : il reste de la tension ailleurs (interrupteur ouvert, appareil de mesure).");
            ImGui::Unindent();
            ImGui::Spacing();
        }
    }

    // ── Loi des nœuds ────────────────────────────────────────────────────
    if (noeuds > 0) {
        ImGui::Text("Loi des nœuds");
        ImGui::Indent();
        int affiches = 0;
        for (int nd = 0; nd < c.net.nbNoeuds && affiches < 6; ++nd) {
            if (!c.estNoeudDeDerivation(nd)) continue;
            double entrants = 0, sortants = 0;
            for (const Elem& e : c.elems) {
                if (e.kind == Kind::Fil) continue;
                if (e.na == nd) { (e.i > 0 ? sortants : entrants) += std::fabs(e.i); }
                else if (e.nb == nd) { (e.i > 0 ? entrants : sortants) += std::fabs(e.i); }
            }
            if (entrants < 1e-9 && sortants < 1e-9) continue;
            ligne("Nœud %d : entrants %s = sortants %s", nd + 1,
                  fmtIntensite(entrants).c_str(), fmtIntensite(sortants).c_str());
            verdict(proche(entrants, sortants, 5e-2), "Le courant se conserve au nœud.",
                    "Déséquilibre au nœud.");
            ++affiches;
        }
        ImGui::Unindent();
        ImGui::Spacing();

        // ── Unicité de la tension en dérivation ─────────────────────────
        const std::vector<std::vector<int>> groupes = c.groupesParalleles();
        if (!groupes.empty()) {
            ImGui::Text("Unicité de la tension en dérivation");
            ImGui::Indent();
            for (const std::vector<int>& g : groupes) {
                double umin = 1e30, umax = -1e30;
                std::string noms;
                for (int k : g) {
                    const double v = std::fabs(c.elems[k].u);
                    umin = std::min(umin, v);
                    umax = std::max(umax, v);
                    if (!noms.empty()) noms += ", ";
                    noms += c.elems[k].nom;
                }
                ligne("%s : U = %s", noms.c_str(), fmtTension(umax).c_str());
                verdict(proche(umin, umax, 1e-2), "Même tension sur chaque branche.",
                        "Les tensions diffèrent.");
            }
            ImGui::Unindent();
        }
    }

    // ── Loi d'Ohm sur les conducteurs ohmiques ───────────────────────────
    bool titre = false;
    for (const Elem& e : c.elems) {
        if (e.kind != Kind::Resistance || std::fabs(e.i) < 1e-9) continue;
        if (!titre) { ImGui::Spacing(); ImGui::Text("Loi d'Ohm"); ImGui::Indent(); titre = true; }
        ligne("%s : U = R x I = %s x %s = %s", e.nom.c_str(),
              fmtResistance(e.valeur).c_str(), fmtIntensite(std::fabs(e.i)).c_str(),
              fmtTension(std::fabs(e.u)).c_str());
    }
    if (titre) ImGui::Unindent();

    // ── Puissance électrique (3e ch.7) ───────────────────────────────────
    if (app.diag.boucleFermee) {
        double pRecue = 0;
        int nb = 0;
        ImGui::Spacing();
        ImGui::Text("Puissance : P = U x I");
        ImGui::Indent();
        for (const Elem& e : c.elems) {
            if (e.kind != Kind::Lampe && e.kind != Kind::Resistance && e.kind != Kind::Moteur &&
                e.kind != Kind::Del && e.kind != Kind::Materiau)
                continue;
            if (std::fabs(e.p) < 1e-6) continue;
            pRecue += std::fabs(e.p);
            if (nb++ < 8)
                ligne("%s : %s x %s = %s", e.nom.c_str(), fmtTension(std::fabs(e.u)).c_str(),
                      fmtIntensite(std::fabs(e.i)).c_str(), fmtPuissance(std::fabs(e.p)).c_str());
        }
        const double pFournie = puissanceFournie(c);
        if (nb > 0 && pFournie > 1e-6) {
            ligne("Puissance fournie par le générateur : %s", fmtPuissance(pFournie).c_str());
            verdict(proche(pFournie, pRecue, 2e-2),
                    "Elle est égale à la somme des puissances reçues par les récepteurs.",
                    "Une partie de la puissance part ailleurs (appareil de mesure, résistance interne).");
        }
        ImGui::Unindent();
    }

    ImGui::End();
}

// ── Journal ──────────────────────────────────────────────────────────────

void panneauJournal(App& app)
{
    ImGui::Begin("Journal");
    if (ImGui::SmallButton("Effacer")) app.journal.clear();
    ImGui::SameLine();
    ImGui::TextDisabled("%d dipôles · %d nœuds · système %dx%d · %.2f ms · %d itération(s)",
                        (int)app.circuit.elems.size(), app.circuit.net.nbNoeuds,
                        app.diag.taille, app.diag.taille, app.diag.dureeMs, app.diag.iterations);
    ImGui::Separator();
    ImGui::BeginChild("lignes");
    for (const std::string& m : app.journal) {
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextUnformatted(m.c_str());
        ImGui::PopTextWrapPos();
    }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 4.0f) ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
    ImGui::End();
}

// ── Bibliothèque ─────────────────────────────────────────────────────────

void panneauBibliotheque(App& app)
{
    ImGui::Begin("Bibliothèque");
    ImGui::TextDisabled("Montages du cours, prêts à ouvrir.");
    ImGui::Separator();

    for (const Exemple& ex : exemples()) {
        ImGui::PushID(ex.titre);
        if (ImGui::Button("Ouvrir")) {
            app.memoriser();
            app.circuit.vider();
            ex.construire(app.circuit);
            app.circuit.construireNetlist();
            app.selection = -1;
            app.outil = Outil::Selection;
            app.ajusterVue = true;
            app.note(std::string("Montage ouvert : ") + ex.titre + " (" + ex.chapitre + ").");
            if (ex.attendu && ex.attendu[0]) app.note(std::string("Attendu : ") + ex.attendu);
            app.recalculer();
        }
        ImGui::SameLine();
        ImGui::Text("%s", ex.titre);
        ImGui::TextDisabled("    %s", ex.chapitre);
        if (ex.attendu && ex.attendu[0]) {
            ImGui::PushTextWrapPos(0.0f);
            ImGui::TextDisabled("    %s", ex.attendu);
            ImGui::PopTextWrapPos();
        }
        ImGui::Separator();
        ImGui::PopID();
    }
    ImGui::End();
}

} // namespace croc
