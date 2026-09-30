// Instruments.cpp — face avant du multimètre, tableau de relevés, tracé U(I),
// wattmètre et compteur d'énergie.
//
// Le multimètre est le chapitre 4e ch.7 à lui tout seul : bornes, sélecteur,
// calibres, affichage « 1. » de dépassement, et les erreurs de branchement qui
// font fondre le fusible. On ne simplifie rien ici — c'est précisément ce que
// l'élève doit apprendre à manœuvrer.
#include "App.h"
#include "Format.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>

namespace croc {
namespace {

const ImVec4 kVert  = ImVec4(0.09f, 0.45f, 0.32f, 1.0f);
const ImVec4 kRouge = ImVec4(0.72f, 0.19f, 0.12f, 1.0f);

struct Calibre {
    double pleineEchelle;   // en unités SI
    const char* etiquette;
};

const Calibre kCalibresTension[]   = {{2, "2 V"}, {20, "20 V"}, {200, "200 V"}, {1000, "1000 V"}};
const Calibre kCalibresIntensite[] = {{0.002, "2 mA"}, {0.02, "20 mA"}, {0.2, "200 mA"}, {10, "10 A"}};
const Calibre kCalibresResistance[] = {{200, "200"}, {2000, "2 k"}, {20000, "20 k"}, {2000000, "2 M"}};

// Un afficheur 3½ digits montre 1999 points : la résolution vaut donc le
// calibre divisé par 2000, et c'est elle qui fixe le nombre de décimales.
int decimalesPourCalibre(double calibre)
{
    if (calibre <= 0) return 2;
    const int d = (int)std::floor(std::log10(2000.0 / calibre));
    return std::clamp(d, 0, 3);
}

std::string uniteDuMode(const Elem& e)
{
    switch (e.mode) {
    case ModeMultimetre::Tension:   return e.calibre < 1.0 ? "mV" : "V";
    case ModeMultimetre::Intensite: return e.calibre < 1.0 ? "mA" : "A";
    default:                        return e.calibre >= 1000 ? (std::string("k") + kOhm) : kOhm;
    }
}

// Ce que montre l'écran : la valeur ramenée à l'unité du calibre, ou « 1. »
// quand elle déborde — exactement l'indication du TP.
std::string afficheur(const Elem& e)
{
    if (e.grillee) return "---";
    if (e.depassement) return "1.";

    double valeur = e.lecture;
    double calibre = e.calibre;
    if (e.mode == ModeMultimetre::Intensite && e.calibre < 1.0) { valeur *= 1000.0; calibre *= 1000.0; }
    if (e.mode == ModeMultimetre::Tension && e.calibre < 1.0)   { valeur *= 1000.0; calibre *= 1000.0; }
    if (e.mode == ModeMultimetre::Resistance && e.calibre >= 1000) { valeur /= 1000.0; calibre /= 1000.0; }

    char buf[64];
    std::snprintf(buf, sizeof buf, "%.*f", decimalesPourCalibre(calibre), valeur);
    std::string s(buf);
    for (char& c : s)
        if (c == '.') c = ',';
    return s;
}

int premierMultimetre(const App& app)
{
    if (app.selection >= 0 && app.selection < (int)app.circuit.elems.size() &&
        app.circuit.elems[app.selection].kind == Kind::Multimetre)
        return app.selection;
    for (int k = 0; k < (int)app.circuit.elems.size(); ++k)
        if (app.circuit.elems[k].kind == Kind::Multimetre) return k;
    return -1;
}

// Régression linéaire U = a·I + b par les moindres carrés. `a` est la pente,
// donc la résistance ; `b` dit si la droite passe par l'origine, ce qui est le
// critère du conducteur ohmique (4e ch.9, 3e ch.6).
struct Regression {
    bool valide = false;
    double pente = 0, ordonnee = 0, r2 = 0;
};

Regression regresser(const std::vector<ImVec2>& points)
{
    Regression reg;
    const int n = (int)points.size();
    if (n < 2) return reg;

    double sx = 0, sy = 0;
    for (const ImVec2& p : points) { sx += p.x; sy += p.y; }
    const double mx = sx / n, my = sy / n;

    double sxy = 0, sxx = 0, syy = 0;
    for (const ImVec2& p : points) {
        sxy += (p.x - mx) * (p.y - my);
        sxx += (p.x - mx) * (p.x - mx);
        syy += (p.y - my) * (p.y - my);
    }
    if (sxx < 1e-18) return reg;

    reg.pente = sxy / sxx;
    reg.ordonnee = my - reg.pente * mx;
    reg.r2 = (syy > 1e-18) ? (sxy * sxy) / (sxx * syy) : 1.0;
    reg.valide = true;
    return reg;
}

} // namespace

// ── Multimètre ───────────────────────────────────────────────────────────

void panneauMultimetre(App& app)
{
    ImGui::Begin("Multimètre");

    const int index = premierMultimetre(app);
    if (index < 0) {
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextDisabled("Aucun multimètre dans le montage. Prends-le dans la Palette et branche-le : "
                            "en dérivation pour mesurer une tension, en série pour mesurer une intensité.");
        ImGui::PopTextWrapPos();
        ImGui::End();
        return;
    }

    Elem& mm = app.circuit.elems[index];
    bool change = false;

    // ── Afficheur ────────────────────────────────────────────────────────
    const std::string lecture = afficheur(mm);
    const std::string unite = uniteDuMode(mm);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.85f, 0.89f, 0.83f, 1.0f));
    ImGui::BeginChild("ecran", ImVec2(-1, ImGui::GetTextLineHeight() * 3.2f), ImGuiChildFlags_Borders);
    ImGui::PushFont(nullptr, ImGui::GetFontSize() * 2.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.09f, 0.14f, 0.11f, 1.0f));
    ImGui::Text("%s %s", lecture.c_str(), unite.c_str());
    ImGui::PopStyleColor();
    ImGui::PopFont();
    if (mm.depassement)
        ImGui::TextColored(kRouge, "Dépassement : passe au calibre supérieur.");
    else if (mm.grillee)
        ImGui::TextColored(kRouge, "Fusible fondu.");
    else
        ImGui::TextDisabled("%s — calibre %s", mm.nom.c_str(),
                            mm.mode == ModeMultimetre::Tension     ? "tension"
                            : mm.mode == ModeMultimetre::Intensite ? "intensité"
                                                                   : "résistance");
    ImGui::EndChild();
    ImGui::PopStyleColor();

    if (mm.grillee) {
        if (ImGui::Button("Remplacer le fusible")) { mm.grillee = false; change = true; }
        ImGui::SameLine();
        ImGui::TextDisabled("puis vérifie le branchement avant de rallumer.");
    }

    ImGui::Separator();

    // ── Bornes ───────────────────────────────────────────────────────────
    ImGui::Text("Cordons");
    ImGui::TextDisabled("Noir : toujours sur COM.");
    ImGui::Text("Rouge :");
    ImGui::SameLine();
    int borne = (int)mm.borne;
    if (ImGui::RadioButton("V / ohm / mA", &borne, 0)) { mm.borne = BorneMultimetre::VOhm; change = true; }
    ImGui::SameLine();
    if (ImGui::RadioButton("10 A", &borne, 1)) { mm.borne = BorneMultimetre::DixAmperes; change = true; }

    ImGui::Separator();

    // ── Sélecteur ────────────────────────────────────────────────────────
    ImGui::Text("Sélecteur");
    int mode = (int)mm.mode;
    if (ImGui::RadioButton("Tension (V)", &mode, 0)) { mm.mode = ModeMultimetre::Tension; mm.calibre = 20; change = true; }
    ImGui::SameLine();
    if (ImGui::RadioButton("Intensité (A)", &mode, 1)) { mm.mode = ModeMultimetre::Intensite; mm.calibre = 10; change = true; }
    ImGui::SameLine();
    if (ImGui::RadioButton("Résistance", &mode, 2)) { mm.mode = ModeMultimetre::Resistance; mm.calibre = 2000; change = true; }

    ImGui::Spacing();
    ImGui::Text("Calibre");
    const Calibre* calibres = kCalibresTension;
    int nbCalibres = 4;
    if (mm.mode == ModeMultimetre::Intensite)  calibres = kCalibresIntensite;
    if (mm.mode == ModeMultimetre::Resistance) calibres = kCalibresResistance;

    for (int k = 0; k < nbCalibres; ++k) {
        if (k) ImGui::SameLine();
        const bool actif = std::fabs(mm.calibre - calibres[k].pleineEchelle) < 1e-9;
        if (actif) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        std::string libelle = calibres[k].etiquette;
        if (mm.mode == ModeMultimetre::Resistance) libelle += kOhm;
        if (ImGui::Button(libelle.c_str())) { mm.calibre = calibres[k].pleineEchelle; change = true; }
        if (actif) ImGui::PopStyleColor();
    }

    ImGui::Separator();
    ImGui::PushTextWrapPos(0.0f);
    switch (mm.mode) {
    case ModeMultimetre::Tension:
        ImGui::TextDisabled("Voltmètre : se branche EN DÉRIVATION, aux bornes du dipôle.");
        break;
    case ModeMultimetre::Intensite:
        ImGui::TextDisabled("Ampèremètre : se branche EN SÉRIE, dans la boucle. Jamais en dérivation : "
                            "il court-circuiterait le dipôle et le fusible fondrait.");
        break;
    case ModeMultimetre::Resistance:
        ImGui::TextDisabled("Ohmmètre : se branche HORS CIRCUIT, sur le composant seul, courant coupé.");
        break;
    }
    ImGui::TextDisabled("Commence toujours par le plus grand calibre, puis affine.");
    ImGui::PopTextWrapPos();

    if (change) {
        app.selection = index;
        app.recalculer();
    }
    ImGui::End();
}

// ── Relevés ──────────────────────────────────────────────────────────────

void panneauReleves(App& app)
{
    ImGui::Begin("Relevés");

    const bool dipoleChoisi = (app.selection >= 0 && app.selection < (int)app.circuit.elems.size());
    if (dipoleChoisi)
        ImGui::Text("Dipôle suivi : %s", app.circuit.elems[app.selection].nom.c_str());
    else
        ImGui::TextDisabled("Choisis un dipôle dans le schéma pour le relever.");

    if (ImGui::Button("Relever")) app.relever();
    ImGui::SameLine();
    if (ImGui::Button("Effacer le tableau")) app.releves.clear();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200);
    char chemin[256];
    std::snprintf(chemin, sizeof chemin, "%s", app.cheminCsv.c_str());
    if (ImGui::InputText("##csv", chemin, sizeof chemin)) app.cheminCsv = chemin;
    ImGui::SameLine();
    if (ImGui::Button("Exporter CSV")) {
        std::ofstream f(app.cheminCsv);
        if (!f) {
            app.note("Export impossible vers " + app.cheminCsv);
        } else {
            // Point-virgule et virgule décimale : le tableur français ouvre le
            // fichier sans aucune boîte de dialogue d'import (§ 6.6).
            f << "Dipole;I (A);U (V);R (ohm);P (W)\n";
            for (const Releve& r : app.releves) {
                auto fr = [](double v) {
                    char b[48];
                    std::snprintf(b, sizeof b, "%.6g", v);
                    std::string s(b);
                    for (char& c : s) if (c == '.') c = ',';
                    return s;
                };
                f << r.dipole << ';' << fr(r.i) << ';' << fr(r.u) << ';' << fr(r.r) << ';' << fr(r.p) << '\n';
            }
            app.note("Tableau exporté vers " + app.cheminCsv + " (" +
                     std::to_string(app.releves.size()) + " lignes).");
        }
    }

    ImGui::Separator();

    const ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                  ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("releves", 6, flags)) {
        ImGui::TableSetupColumn("N°");
        ImGui::TableSetupColumn("Dipôle");
        ImGui::TableSetupColumn("I");
        ImGui::TableSetupColumn("U");
        ImGui::TableSetupColumn("R");
        ImGui::TableSetupColumn("P");
        ImGui::TableHeadersRow();
        for (int k = 0; k < (int)app.releves.size(); ++k) {
            const Releve& r = app.releves[k];
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("%d", k + 1);
            ImGui::TableNextColumn(); ImGui::TextUnformatted(r.dipole.c_str());
            ImGui::TableNextColumn(); ImGui::TextUnformatted(fmtIntensite(r.i).c_str());
            ImGui::TableNextColumn(); ImGui::TextUnformatted(fmtTension(r.u).c_str());
            ImGui::TableNextColumn();
            if (r.r > 0) ImGui::TextUnformatted(fmtResistance(r.r).c_str());
            else ImGui::TextDisabled("—");
            ImGui::TableNextColumn(); ImGui::TextUnformatted(fmtPuissance(r.p).c_str());
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

// ── Graphique U = f(I) ───────────────────────────────────────────────────

void panneauGraphique(App& app)
{
    ImGui::Begin("Graphique");

    // On trace le dipôle sélectionné s'il a des relevés, sinon le premier
    // dipôle relevé : l'élève ne devrait jamais avoir à choisir deux fois.
    std::string cible;
    if (app.selection >= 0 && app.selection < (int)app.circuit.elems.size())
        cible = app.circuit.elems[app.selection].nom;
    bool trouve = false;
    for (const Releve& r : app.releves)
        if (r.dipole == cible) { trouve = true; break; }
    if (!trouve && !app.releves.empty()) cible = app.releves.front().dipole;

    std::vector<ImVec2> points;   // x = I (A), y = U (V)
    for (const Releve& r : app.releves)
        if (r.dipole == cible) points.push_back(ImVec2((float)r.i, (float)r.u));

    ImGui::Text("Caractéristique U = f(I)");
    ImGui::SameLine();
    ImGui::TextDisabled("%s — %d point(s)", cible.empty() ? "aucun dipôle" : cible.c_str(), (int)points.size());

    const Regression reg = regresser(points);
    if (reg.valide) {
        ImGui::PushTextWrapPos(0.0f);
        ImGui::Text("Coefficient directeur de la droite : %s", fmtResistance(reg.pente).c_str());
        const bool parLOrigine = std::fabs(reg.ordonnee) < 0.05 * std::max(0.1, (double)points.back().y);
        const bool alignes = reg.r2 > 0.999;
        if (alignes && parLOrigine)
            ImGui::TextColored(kVert, "Droite passant par l'origine : c'est un conducteur ohmique, "
                                      "et sa résistance vaut le coefficient directeur.");
        else if (!alignes)
            ImGui::TextColored(kRouge, "Les points ne sont pas alignés : ce dipôle n'est pas un "
                                       "conducteur ohmique (sa résistance change).");
        else
            ImGui::TextColored(kRouge, "La droite ne passe pas par l'origine.");
        ImGui::PopTextWrapPos();
    } else {
        ImGui::TextDisabled("Fais varier le générateur et clique « Relever » plusieurs fois : "
                            "le nuage de points et sa droite apparaîtront ici.");
    }

    // ── Tracé ────────────────────────────────────────────────────────────
    const ImVec2 origine = ImGui::GetCursorScreenPos();
    ImVec2 taille = ImGui::GetContentRegionAvail();
    taille.x = std::max(taille.x, 120.0f);
    taille.y = std::max(taille.y, 120.0f);
    ImGui::InvisibleButton("graphe", taille);
    ImDrawList* dl = ImGui::GetWindowDrawList();

    const ImU32 colFond = IM_COL32(250, 249, 245, 255);
    const ImU32 colAxe  = IM_COL32(60, 70, 82, 255);
    const ImU32 colGrille = IM_COL32(120, 130, 145, 45);
    const ImU32 colPoint = IM_COL32(198, 62, 35, 255);
    const ImU32 colDroite = IM_COL32(20, 110, 200, 200);
    dl->AddRectFilled(origine, ImVec2(origine.x + taille.x, origine.y + taille.y), colFond);

    const float margeG = 62.0f, margeB = 34.0f, margeH = 14.0f, margeD = 14.0f;
    const ImVec2 o(origine.x + margeG, origine.y + taille.y - margeB);
    const float largeur = taille.x - margeG - margeD;
    const float hauteur = taille.y - margeH - margeB;

    double imax = 0, umax = 0;
    for (const ImVec2& p : points) { imax = std::max(imax, (double)p.x); umax = std::max(umax, (double)p.y); }
    if (imax <= 0) imax = 1;
    if (umax <= 0) umax = 1;
    imax *= 1.15;
    umax *= 1.15;

    auto versEcran = [&](double i, double u) {
        return ImVec2(o.x + (float)(i / imax) * largeur, o.y - (float)(u / umax) * hauteur);
    };

    for (int k = 1; k <= 4; ++k) {
        const float x = o.x + largeur * k / 4.0f;
        const float y = o.y - hauteur * k / 4.0f;
        dl->AddLine(ImVec2(x, o.y), ImVec2(x, o.y - hauteur), colGrille);
        dl->AddLine(ImVec2(o.x, y), ImVec2(o.x + largeur, y), colGrille);
    }
    dl->AddLine(o, ImVec2(o.x + largeur, o.y), colAxe, 2.0f);
    dl->AddLine(o, ImVec2(o.x, o.y - hauteur), colAxe, 2.0f);
    dl->AddText(ImVec2(o.x + largeur - 46, o.y + 8), colAxe, "I (A)");
    dl->AddText(ImVec2(origine.x + 6, origine.y + margeH), colAxe, "U (V)");
    dl->AddText(ImVec2(o.x + largeur - 60, o.y + 20), colAxe, fmtIntensite(imax).c_str());
    dl->AddText(ImVec2(origine.x + 6, o.y - hauteur + 16), colAxe, fmtTension(umax).c_str());

    if (reg.valide) {
        const ImVec2 a = versEcran(0, reg.ordonnee);
        const ImVec2 b = versEcran(imax, reg.pente * imax + reg.ordonnee);
        dl->AddLine(a, b, colDroite, 2.0f);
    }
    for (const ImVec2& p : points)
        dl->AddCircleFilled(versEcran(p.x, p.y), 4.0f, colPoint, 12);

    ImGui::End();
}

// ── Énergie : wattmètre, E = P × t, compteur (3e ch.7) ───────────────────

namespace {

// Ce qu'on branche sur une prise ou une pile pour qu'il consomme : c'est sur
// eux que le wattmètre a un sens.
bool estRecepteur(Kind k)
{
    return k == Kind::Lampe || k == Kind::Resistance || k == Kind::Moteur ||
           k == Kind::Del || k == Kind::Materiau;
}

std::string fmtDuree(double secondes)
{
    const long total = (long)std::llround(secondes);
    const long h = total / 3600, min = (total % 3600) / 60, s = total % 60;
    char buf[48];
    if (h > 0)        std::snprintf(buf, sizeof buf, "%ld h %02ld min", h, min);
    else if (min > 0) std::snprintf(buf, sizeof buf, "%ld min %02ld s", min, s);
    else              std::snprintf(buf, sizeof buf, "%ld s", s);
    return buf;
}

// Index du compteur électrique : décimales fixes, comme les rouleaux du
// compteur, pour que l'élève voie les chiffres de droite défiler.
std::string fmtIndex(double kwh)
{
    char buf[48];
    std::snprintf(buf, sizeof buf, "%.4f", kwh);
    std::string s(buf);
    for (char& c : s)
        if (c == '.') c = ',';
    return s + " kWh";
}

// Arrondi à trois chiffres significatifs, écrit en entier quand il est
// grand : « 7 200 000 J » comme au tableau, pas le « 7 199 946 J » que
// donnerait le résidu des fils et de l'interrupteur.
std::string fmtArrondi(double v)
{
    if (v == 0.0) return "0";
    const double pas = std::pow(10.0, std::floor(std::log10(std::fabs(v))) - 2);
    const double r = std::round(v / pas) * pas;
    return (std::fabs(r) >= 100.0) ? fmtNombre(r, 1 + (int)std::floor(std::log10(std::fabs(r)))) : fmtNombre(r, 3);
}

void ecranVert(const char* id, const std::string& texte)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.85f, 0.89f, 0.83f, 1.0f));
    ImGui::BeginChild(id, ImVec2(-1, ImGui::GetTextLineHeight() * 3.0f), ImGuiChildFlags_Borders,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.8f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.09f, 0.14f, 0.11f, 1.0f));
    ImGui::TextUnformatted(texte.c_str());
    ImGui::PopStyleColor();
    ImGui::PopFont();
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

} // namespace

void panneauEnergie(App& app)
{
    ImGui::Begin("Énergie");
    ImGui::PushTextWrapPos(0.0f);

    // ── Wattmètre sur l'appareil choisi ──────────────────────────────────
    ImGui::Text("Wattmètre");
    const bool choisi = app.selection >= 0 && app.selection < (int)app.circuit.elems.size() &&
                        estRecepteur(app.circuit.elems[app.selection].kind);
    if (!choisi) {
        ImGui::TextDisabled("Choisis un appareil dans le schéma (lampe, résistance, moteur…) : "
                            "le wattmètre affiche la puissance qu'il reçoit.");
    } else {
        const Elem& e = app.circuit.elems[app.selection];
        const double p = std::fabs(e.p);
        ecranVert("wattmetre", fmtPuissance(p));
        ImGui::Text("%s : P = U x I = %s x %s = %s", e.nom.c_str(), fmtTension(std::fabs(e.u)).c_str(),
                    fmtIntensite(std::fabs(e.i)).c_str(), fmtPuissance(p).c_str());

        ImGui::Spacing();
        ImGui::Text("Énergie reçue : E = P x t");
        ImGui::SetNextItemWidth(120);
        ImGui::InputDouble("durée (min)", &app.dureeMinutes, 5.0, 60.0, "%.0f");
        app.dureeMinutes = std::clamp(app.dureeMinutes, 0.0, 525600.0);
        const struct { const char* libelle; double minutes; } durees[] = {
            {"10 min", 10}, {"15 min", 15}, {"1 h", 60}, {"2 h", 120}, {"24 h", 1440}};
        for (const auto& d : durees) {
            if (&d != durees) ImGui::SameLine(0.0f, 4.0f);   // sur leur propre ligne : le panneau est étroit
            if (ImGui::SmallButton(d.libelle)) app.dureeMinutes = d.minutes;
        }

        const double secondes = app.dureeMinutes * 60.0;
        const double heures   = app.dureeMinutes / 60.0;
        const double joules   = energieJoules(p, secondes);
        const double kwh      = energieKWh(p, heures);
        ImGui::BulletText("en joules : E = %s W x %s s = %s J", fmtArrondi(p).c_str(),
                          fmtArrondi(secondes).c_str(), fmtArrondi(joules).c_str());
        ImGui::BulletText("en kWh : E = %s kW x %s h = %s kWh", fmtNombre(p / 1000.0, 3).c_str(),
                          fmtNombre(heures, 3).c_str(), fmtNombre(kwh, 3).c_str());
        ImGui::BulletText("coût : %s kWh x %s = %s", fmtNombre(kwh, 3).c_str(),
                          (fmtEuros(app.prixKWh) + "/kWh").c_str(), fmtEuros(kwh * app.prixKWh).c_str());
    }

    ImGui::Separator();

    // ── Compteur de l'installation ───────────────────────────────────────
    ImGui::Text("Compteur électrique");
    ImGui::TextDisabled("Il compte l'énergie fournie à tous les appareils en marche, "
                        "comme le compteur de la maison.");
    const double pTotale = std::max(0.0, puissanceFournie(app.circuit));
    const double kwhCompteur = app.compteurJoules / 3.6e6;
    ecranVert("compteur", fmtIndex(kwhCompteur));
    ImGui::Text("Puissance appelée : %s", fmtPuissance(pTotale).c_str());
    ImGui::Text("Durée écoulée : %s", fmtDuree(app.compteurSecondes).c_str());
    ImGui::Text("Coût : %s", fmtEuros(kwhCompteur * app.prixKWh).c_str());

    if (ImGui::Button(app.compteurEnMarche ? "Arrêter" : "Démarrer")) app.compteurEnMarche = !app.compteurEnMarche;
    ImGui::SameLine();
    if (ImGui::Button("Remettre à zéro")) { app.compteurJoules = 0; app.compteurSecondes = 0; }

    ImGui::Text("Vitesse :");
    const struct { const char* libelle; double facteur; } vitesses[] = {
        {"temps réel", 1.0}, {"1 s = 1 min", 60.0}, {"1 s = 1 h", 3600.0}};
    for (const auto& v : vitesses) {
        ImGui::SameLine();
        if (ImGui::RadioButton(v.libelle, app.acceleration == v.facteur)) app.acceleration = v.facteur;
    }

    ImGui::Separator();
    ImGui::SetNextItemWidth(120);
    ImGui::InputDouble("prix du kWh (euros)", &app.prixKWh, 0.01, 0.05, "%.4f");
    app.prixKWh = std::max(0.0, app.prixKWh);
    ImGui::TextDisabled("Tarif à vérifier sur une facture récente.");

    ImGui::PopTextWrapPos();
    ImGui::End();
}

} // namespace croc
