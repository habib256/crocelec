// Schematic.cpp — le canevas : quadrillage, symboles normalisés, animation du
// courant et édition du montage.
//
// Tout est dessiné à l'ImDrawList, sans texture : c'est ce qui permet de tenir
// la contrainte WebGL 1 de la Galaxy Tab 4 (§ 8.1). Les symboles suivent le
// tracé du cahier de l'élève — rectangle pour la résistance, cercle barré pour
// la lampe, trait long / trait court pour la pile.
#include "App.h"
#include "Format.h"

#include <algorithm>
#include <cmath>

namespace croc {
namespace {

constexpr float kGrille = 28.0f;   // pas du quadrillage, en pixels à zoom 1

// Palette : fond papier, encre sombre, un seul accent chaud pour le courant.
// Pensée pour le vidéoprojecteur (§ 7 du cahier des charges).
const ImU32 kColFond      = IM_COL32(250, 249, 245, 255);
const ImU32 kColGrille    = IM_COL32(120, 130, 145, 40);
const ImU32 kColGrille5   = IM_COL32(120, 130, 145, 75);
const ImU32 kColEncre     = IM_COL32(24, 30, 38, 255);
const ImU32 kColCourant   = IM_COL32(198, 62, 35, 255);
const ImU32 kColSelection = IM_COL32(20, 110, 200, 255);
const ImU32 kColNoeud     = IM_COL32(24, 30, 38, 255);
const ImU32 kColEteint    = IM_COL32(150, 155, 162, 255);
const ImU32 kColLampeOn   = IM_COL32(255, 205, 70, 255);
const ImU32 kColGrillee   = IM_COL32(170, 40, 40, 255);
const ImU32 kColTexte     = IM_COL32(60, 70, 82, 255);
const ImU32 kColAide      = IM_COL32(20, 110, 200, 160);

ImVec2 operator+(ImVec2 a, ImVec2 b) { return ImVec2(a.x + b.x, a.y + b.y); }
ImVec2 operator-(ImVec2 a, ImVec2 b) { return ImVec2(a.x - b.x, a.y - b.y); }
ImVec2 operator*(ImVec2 a, float k)  { return ImVec2(a.x * k, a.y * k); }

float longueur(ImVec2 v) { return std::sqrt(v.x * v.x + v.y * v.y); }

ImVec2 normalise(ImVec2 v)
{
    const float l = longueur(v);
    return (l > 1e-6f) ? ImVec2(v.x / l, v.y / l) : ImVec2(1, 0);
}

ImVec2 perpendiculaire(ImVec2 v) { return ImVec2(-v.y, v.x); }

ImVec2 versEcran(const App& app, Pt p)
{
    return ImVec2(app.pan.x + p.x * kGrille * app.zoom,
                  app.pan.y + p.y * kGrille * app.zoom);
}

Pt versGrille(const App& app, ImVec2 s)
{
    const float gx = (s.x - app.pan.x) / (kGrille * app.zoom);
    const float gy = (s.y - app.pan.y) / (kGrille * app.zoom);
    Pt p;
    p.x = (int)std::lround(gx);
    p.y = (int)std::lround(gy);
    return p;
}

float distancePointSegment(ImVec2 p, ImVec2 a, ImVec2 b)
{
    const ImVec2 ab = b - a;
    const float l2 = ab.x * ab.x + ab.y * ab.y;
    if (l2 < 1e-6f) return longueur(p - a);
    float t = ((p.x - a.x) * ab.x + (p.y - a.y) * ab.y) / l2;
    t = std::clamp(t, 0.0f, 1.0f);
    return longueur(p - (a + ab * t));
}

// ── Symboles ─────────────────────────────────────────────────────────────

void traitEpais(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 col, float ep)
{
    dl->AddLine(a, b, col, ep);
}

void dessineSymbole(ImDrawList* dl, const Elem& e, ImVec2 A, ImVec2 B, float zoom, ImU32 col)
{
    const ImVec2 d = normalise(B - A);
    const ImVec2 n = perpendiculaire(d);
    const float  L = longueur(B - A);
    const float  ep = std::max(1.6f, 2.0f * zoom);
    const ImVec2 centre = ImVec2((A.x + B.x) * 0.5f, (A.y + B.y) * 0.5f);

    // Longueur du corps du symbole, bornée pour rester lisible quel que soit
    // l'écartement choisi par l'élève.
    const float corps = std::min(L * 0.55f, 34.0f * zoom);
    const ImVec2 c1 = centre - d * (corps * 0.5f);
    const ImVec2 c2 = centre + d * (corps * 0.5f);

    auto amenees = [&]() {
        traitEpais(dl, A, c1, col, ep);
        traitEpais(dl, c2, B, col, ep);
    };

    switch (e.kind) {
    case Kind::Fil:
        traitEpais(dl, A, B, col, ep);
        break;

    case Kind::Resistance: {
        amenees();
        const float h = 7.0f * zoom;
        ImVec2 p1 = c1 + n * h, p2 = c2 + n * h, p3 = c2 - n * h, p4 = c1 - n * h;
        dl->AddQuadFilled(p1, p2, p3, p4, kColFond);
        dl->AddQuad(p1, p2, p3, p4, col, ep);
        break;
    }

    case Kind::Multimetre: {
        // Boîtier rectangulaire avec l'unité mesurée : on reconnaît l'appareil
        // du TP, pas un symbole normalisé de plus.
        amenees();
        const float demiL = corps * 0.62f, demiH = 11.0f * zoom;
        const ImVec2 p1 = centre + d * demiL + n * demiH;
        const ImVec2 p2 = centre + d * demiL - n * demiH;
        const ImVec2 p3 = centre - d * demiL - n * demiH;
        const ImVec2 p4 = centre - d * demiL + n * demiH;
        dl->AddQuadFilled(p1, p2, p3, p4, e.grillee ? IM_COL32(250, 235, 235, 255) : kColFond);
        dl->AddQuad(p1, p2, p3, p4, e.grillee ? kColGrillee : col, ep);
        const char* lettre = (e.mode == ModeMultimetre::Tension)     ? "V"
                             : (e.mode == ModeMultimetre::Intensite) ? "A"
                                                                     : kOhm;
        const ImVec2 ts = ImGui::CalcTextSize(lettre);
        dl->AddText(ImVec2(centre.x - ts.x * 0.5f, centre.y - ts.y * 0.5f),
                    e.grillee ? kColGrillee : col, lettre);
        break;
    }

    case Kind::Moteur:
    case Kind::Voltmetre:
    case Kind::Amperemetre: {
        amenees();
        const float r = corps * 0.5f;
        dl->AddCircleFilled(centre, r, kColFond, 24);
        dl->AddCircle(centre, r, col, 24, ep);
        const char* lettre = (e.kind == Kind::Moteur) ? "M" : (e.kind == Kind::Voltmetre ? "V" : "A");
        const ImVec2 ts = ImGui::CalcTextSize(lettre);
        dl->AddText(ImVec2(centre.x - ts.x * 0.5f, centre.y - ts.y * 0.5f), col, lettre);
        break;
    }

    case Kind::Lampe: {
        amenees();
        const float r = corps * 0.5f;
        if (e.allume) {
            const float halo = r * 2.2f;
            dl->AddCircleFilled(centre, halo, IM_COL32(255, 210, 90, 60), 24);
            dl->AddCircleFilled(centre, r, kColLampeOn, 24);
        } else {
            dl->AddCircleFilled(centre, r, kColFond, 24);
        }
        dl->AddCircle(centre, r, e.grillee ? kColGrillee : col, 24, ep);
        const float k = r * 0.7071f;
        const ImVec2 u = d * k + n * k;
        const ImVec2 v = d * k - n * k;
        dl->AddLine(centre - u, centre + u, e.grillee ? kColGrillee : col, ep);
        dl->AddLine(centre - v, centre + v, e.grillee ? kColGrillee : col, ep);
        break;
    }

    case Kind::Pile: {
        // Trait long = borne + (côté b), trait court = borne − (côté a).
        const float ecart = 4.5f * zoom;
        const ImVec2 court = centre - d * ecart;
        const ImVec2 lng   = centre + d * ecart;
        traitEpais(dl, A, court, col, ep);
        traitEpais(dl, lng, B, col, ep);
        traitEpais(dl, court - n * (6.0f * zoom), court + n * (6.0f * zoom), col, ep * 1.8f);
        traitEpais(dl, lng - n * (11.0f * zoom), lng + n * (11.0f * zoom), col, ep);
        break;
    }

    case Kind::Interrupteur: {
        const float ecart = 9.0f * zoom;
        const ImVec2 g = centre - d * ecart;
        const ImVec2 dr = centre + d * ecart;
        traitEpais(dl, A, g, col, ep);
        traitEpais(dl, dr, B, col, ep);
        dl->AddCircleFilled(g, 2.5f * zoom, col, 12);
        dl->AddCircleFilled(dr, 2.5f * zoom, col, 12);
        if (e.ferme) {
            traitEpais(dl, g, dr, col, ep);
        } else {
            const ImVec2 leve = g + d * (2.0f * ecart * 0.85f) - n * (11.0f * zoom);
            traitEpais(dl, g, leve, col, ep);
        }
        break;
    }

    case Kind::Del: {
        amenees();
        const float r = 8.0f * zoom;
        const ImVec2 pointe = c2;
        const ImVec2 base1  = c1 + n * r;
        const ImVec2 base2  = c1 - n * r;
        if (e.allume) dl->AddTriangleFilled(base1, base2, pointe, kColLampeOn);
        else          dl->AddTriangleFilled(base1, base2, pointe, kColFond);
        dl->AddTriangle(base1, base2, pointe, e.grillee ? kColGrillee : col, ep);
        traitEpais(dl, pointe + n * r, pointe - n * r, e.grillee ? kColGrillee : col, ep);
        // Les deux flèches qui distinguent la DEL de la diode.
        for (int k = 0; k < 2; ++k) {
            const ImVec2 base = centre + n * (r + 4.0f * zoom) + d * (k * 6.0f * zoom - 3.0f * zoom);
            const ImVec2 bout = base + n * (7.0f * zoom) + d * (4.0f * zoom);
            traitEpais(dl, base, bout, col, ep * 0.8f);
        }
        break;
    }

    default:
        traitEpais(dl, A, B, col, ep);
        break;
    }
}

// Points animés qui matérialisent le courant. La vitesse dépend de I, mais
// elle est bornée : au-delà, l'élève ne voit plus qu'un trait continu.
void dessineCourant(ImDrawList* dl, const App& app, const Elem& e, ImVec2 A, ImVec2 B)
{
    if (!app.reglages.animerCourant) return;
    if (std::fabs(e.i) < 1e-5) return;
    if (e.kind == Kind::Interrupteur && !e.ferme) return;

    float sens = (e.i > 0) ? 1.0f : -1.0f;
    if (app.reglages.sensElectrons) sens = -sens;

    const float L = longueur(B - A);
    if (L < 4.0f) return;
    const ImVec2 d = normalise(B - A);

    const float vitesse = std::clamp((float)std::fabs(e.i) * 60.0f, 10.0f, 90.0f); // px/s
    const float pas = 26.0f * app.zoom;
    const int   nb  = std::max(1, (int)(L / pas));
    const float phase = std::fmod((float)app.tempsAnim * vitesse, pas);

    for (int k = 0; k <= nb; ++k) {
        float t = k * pas + (sens > 0 ? phase : pas - phase);
        if (t < 0 || t > L) continue;
        const ImVec2 p = A + d * t;
        dl->AddCircleFilled(p, 3.0f * app.zoom, kColCourant, 10);
    }
}

void dessineValeurs(ImDrawList* dl, const App& app, const Elem& e, ImVec2 A, ImVec2 B)
{
    const ImVec2 centre = ImVec2((A.x + B.x) * 0.5f, (A.y + B.y) * 0.5f);
    const ImVec2 n = perpendiculaire(normalise(B - A));
    const float  decalage = 30.0f * app.zoom;

    if (e.kind == Kind::Fil) return;

    // Deux lignes séparées — nom au-dessus, mesure en dessous : une seule
    // ligne « L1 : U = 4,50 V I = 95,4 mA » finit toujours par recouvrir le
    // symbole voisin dès que le montage se resserre.
    std::string ligne1, ligne2;
    if (app.reglages.afficherNoms && !e.nom.empty()) ligne1 = e.nom;
    if (app.reglages.afficherValeurs) {
        switch (e.kind) {
        case Kind::Voltmetre:   ligne2 = fmtTension(std::fabs(e.u)); break;
        case Kind::Amperemetre: ligne2 = fmtIntensite(std::fabs(e.i)); break;
        case Kind::Multimetre:
            if (e.grillee)         ligne2 = "fusible fondu";
            else if (e.depassement) ligne2 = "1.  (dépassement)";
            else if (e.mode == ModeMultimetre::Tension)   ligne2 = fmtTension(e.lecture);
            else if (e.mode == ModeMultimetre::Intensite) ligne2 = fmtIntensite(e.lecture);
            else                                          ligne2 = fmtResistance(e.lecture);
            break;
        default:
            ligne2 = fmtTension(std::fabs(e.u)) + " / " + fmtIntensite(std::fabs(e.i));
            break;
        }
    }
    if (ligne1.empty() && ligne2.empty()) return;

    const float hauteurLigne = ImGui::GetTextLineHeight();
    const ImVec2 t1 = ligne1.empty() ? ImVec2(0, 0) : ImGui::CalcTextSize(ligne1.c_str());
    const ImVec2 t2 = ligne2.empty() ? ImVec2(0, 0) : ImGui::CalcTextSize(ligne2.c_str());
    const float largeur = std::max(t1.x, t2.x);
    const float hauteur = (ligne1.empty() ? 0.0f : hauteurLigne) + (ligne2.empty() ? 0.0f : hauteurLigne);

    ImVec2 pos = centre + n * decalage;
    pos.x -= largeur * 0.5f;
    pos.y -= hauteur * 0.5f;
    dl->AddRectFilled(ImVec2(pos.x - 4, pos.y - 2), ImVec2(pos.x + largeur + 4, pos.y + hauteur + 2),
                      IM_COL32(250, 249, 245, 225), 3.0f);
    float y = pos.y;
    if (!ligne1.empty()) {
        dl->AddText(ImVec2(pos.x + (largeur - t1.x) * 0.5f, y), kColEncre, ligne1.c_str());
        y += hauteurLigne;
    }
    if (!ligne2.empty())
        dl->AddText(ImVec2(pos.x + (largeur - t2.x) * 0.5f, y), kColTexte, ligne2.c_str());
}

// Recadre la vue sur le montage : c'est ce qui manque le plus quand on ouvre
// un circuit de la bibliothèque, dont l'étendue n'a aucune raison de coïncider
// avec la fenêtre de l'utilisateur.
void ajusterLaVue(App& app, ImVec2 origine, ImVec2 taille)
{
    if (app.circuit.elems.empty()) {
        app.pan = ImVec2(origine.x + 60.0f, origine.y + 60.0f);
        app.zoom = 1.0f;
        return;
    }
    int xmin = 1 << 30, xmax = -(1 << 30), ymin = 1 << 30, ymax = -(1 << 30);
    for (const Elem& e : app.circuit.elems) {
        xmin = std::min({xmin, e.a.x, e.b.x});
        xmax = std::max({xmax, e.a.x, e.b.x});
        ymin = std::min({ymin, e.a.y, e.b.y});
        ymax = std::max({ymax, e.a.y, e.b.y});
    }
    const float marge = 90.0f;                       // place pour les étiquettes
    const float largeur = std::max(1.0f, (float)(xmax - xmin)) * kGrille;
    const float hauteur = std::max(1.0f, (float)(ymax - ymin)) * kGrille;
    const float zx = (taille.x - 2 * marge) / largeur;
    const float zy = (taille.y - 2 * marge) / hauteur;
    app.zoom = std::clamp(std::min(zx, zy), 0.35f, 2.0f);

    const ImVec2 centreCanevas(origine.x + taille.x * 0.5f, origine.y + taille.y * 0.5f);
    const float cx = (xmin + xmax) * 0.5f * kGrille * app.zoom;
    const float cy = (ymin + ymax) * 0.5f * kGrille * app.zoom;
    app.pan = ImVec2(centreCanevas.x - cx, centreCanevas.y - cy);
}

int dipoleSousLaSouris(const App& app, ImVec2 souris)
{
    int trouve = -1;
    float meilleure = 12.0f * std::max(0.6f, app.zoom);
    for (int k = (int)app.circuit.elems.size() - 1; k >= 0; --k) {
        const Elem& e = app.circuit.elems[k];
        const float d = distancePointSegment(souris, versEcran(app, e.a), versEcran(app, e.b));
        if (d < meilleure) { meilleure = d; trouve = k; }
    }
    return trouve;
}

Elem fabrique(Kind k, Pt a, Pt b)
{
    Elem e;
    e.kind = k;
    e.a = a;
    e.b = b;
    switch (k) {
    case Kind::Pile:        e.valeur = 4.5;  e.valeur2 = 0.0;  break; // pile plate de collège
    case Kind::Resistance:  e.valeur = 100.0;                  break;
    case Kind::Moteur:      e.valeur = 20.0;                   break;
    case Kind::Lampe:       e.valeur = 6.0;  e.valeur2 = 0.6;  break; // 6 V / 0,6 W → 60 Ω, 100 mA
    case Kind::Del:         e.valeur = 2.0;  e.valeur2 = 0.03; break; // seuil 2 V, 30 mA max
    default: break;
    }
    return e;
}

const char* libelleOutil(Outil o)
{
    switch (o) {
    case Outil::Selection: return "Sélection";
    default:               return nomKind(kindDeLOutil(o));
    }
}

} // namespace

void panneauSchema(App& app)
{
    ImGui::Begin("Schéma");

    // ── Barre d'outils ───────────────────────────────────────────────────
    if (ImGui::Button("Annuler")) app.annuler();
    ImGui::SameLine();
    if (ImGui::Button("Tout effacer")) {
        app.memoriser();
        app.circuit.vider();
        app.selection = -1;
        app.note("Nouveau montage.");
        app.recalculer();
    }
    ImGui::SameLine();
    if (ImGui::Button("Ajuster la vue")) app.ajusterVue = true;
    ImGui::SameLine();
    ImGui::TextUnformatted("|");
    ImGui::SameLine();
    ImGui::Checkbox("Valeurs", &app.reglages.afficherValeurs);
    ImGui::SameLine();
    ImGui::Checkbox("Animer", &app.reglages.animerCourant);
    // La bascule « Électrons » reste un geste de professeur (4e ch.6) :
    // en mode simple, seul le sens conventionnel du cours est montré.
    if (!app.reglages.modeSimple) {
        ImGui::SameLine();
        ImGui::Checkbox("Électrons", &app.reglages.sensElectrons);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Décoché : sens conventionnel du courant, du + vers le − à l'extérieur du générateur.\n"
                              "Coché : sens réel de déplacement des électrons, qui est l'inverse.");
    }
    ImGui::SameLine();
    ImGui::TextDisabled("outil : %s", libelleOutil(app.outil));

    // ── Canevas ──────────────────────────────────────────────────────────
    const ImVec2 origine = ImGui::GetCursorScreenPos();
    ImVec2 taille = ImGui::GetContentRegionAvail();
    taille.x = std::max(taille.x, 80.0f);
    taille.y = std::max(taille.y, 80.0f);

    ImGui::InvisibleButton("canevas", taille,
                           ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonMiddle);
    const bool survole = ImGui::IsItemHovered();
    const bool actif   = ImGui::IsItemActive();

    if (!app.vueInitialisee || app.ajusterVue) {
        ajusterLaVue(app, origine, taille);
        app.vueInitialisee = true;
        app.ajusterVue = false;
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->PushClipRect(origine, origine + taille, true);
    dl->AddRectFilled(origine, origine + taille, kColFond);

    ImGuiIO& io = ImGui::GetIO();
    const ImVec2 souris = io.MousePos;

    // Zoom à la molette, centré sur le pointeur.
    if (survole && io.MouseWheel != 0.0f) {
        const Pt sousSouris = versGrille(app, souris);
        app.zoom = std::clamp(app.zoom * (io.MouseWheel > 0 ? 1.12f : 1.0f / 1.12f), 0.35f, 3.5f);
        const ImVec2 apres = versEcran(app, sousSouris);
        app.pan = app.pan + (souris - apres);
    }

    // Déplacement de la vue : bouton du milieu, ou glissé dans le vide en
    // mode Sélection (geste naturel, et transposable au doigt plus tard).
    static bool deplaceVue = false;
    static int  dipoleDeplace = -1;
    static Pt   accroche;
    static ImVec2 pointDePression;
    static bool deplacementCommence = false;   // vrai une fois le seuil franchi

    const int survoleDipole = survole ? dipoleSousLaSouris(app, souris) : -1;

    if (survole && ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) deplaceVue = true;
    if (!ImGui::IsMouseDown(ImGuiMouseButton_Middle) && deplaceVue && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
        deplaceVue = false;
    if (deplaceVue && ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
        app.pan = app.pan + io.MouseDelta;
    }

    if (survole && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        if (app.outil == Outil::Selection) {
            app.selection = survoleDipole;
            if (survoleDipole >= 0) {
                Elem& e = app.circuit.elems[survoleDipole];
                if (e.kind == Kind::Interrupteur) {          // clic = manœuvre
                    app.memoriser();
                    e.ferme = !e.ferme;
                    app.note(e.nom + (e.ferme ? " fermé : le circuit est fermé." : " ouvert : le circuit est ouvert."));
                    app.recalculer();
                }
                if (e.kind == Kind::Lampe && e.grillee) {
                    app.memoriser();
                    e.grillee = false;
                    app.note(e.nom + " remplacée.");
                    app.recalculer();
                }
                dipoleDeplace = survoleDipole;
                accroche = versGrille(app, souris);
                pointDePression = souris;
                deplacementCommence = false;
            } else {
                deplaceVue = true;
            }
        } else {
            app.trace = true;
            app.traceDepart = versGrille(app, souris);
            app.traceFin = app.traceDepart;
        }
    }

    if (deplaceVue && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        app.pan = app.pan + io.MouseDelta;

    // Déplacement d'un dipôle. Le seuil de quelques pixels est indispensable :
    // sans lui, le simple clic qui sélectionne un dipôle (ou qui manœuvre un
    // interrupteur) le déplace d'une case et déforme le montage.
    if (dipoleDeplace >= 0 && ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
        dipoleDeplace < (int)app.circuit.elems.size()) {
        if (!deplacementCommence && longueur(souris - pointDePression) > 6.0f) {
            deplacementCommence = true;
            app.memoriser();
        }
        const Pt maintenant = versGrille(app, souris);
        if (deplacementCommence && maintenant != accroche) {
            Elem& e = app.circuit.elems[dipoleDeplace];
            e.a.x += maintenant.x - accroche.x;
            e.a.y += maintenant.y - accroche.y;
            e.b.x += maintenant.x - accroche.x;
            e.b.y += maintenant.y - accroche.y;
            accroche = maintenant;
            app.recalculer();
        }
    }
    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        dipoleDeplace = -1;
        deplacementCommence = false;
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Middle)) deplaceVue = false;
    }

    if (app.trace) {
        app.traceFin = versGrille(app, souris);
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            Pt fin = app.traceFin;
            if (fin == app.traceDepart) fin.x += 2;      // simple clic : dipôle horizontal
            app.memoriser();
            const int index = app.circuit.ajouter(fabrique(kindDeLOutil(app.outil), app.traceDepart, fin));
            app.selection = index;
            app.trace = false;
            app.recalculer();
        }
    }

    // ── Menu contextuel : pannes, polarité, suppression ──────────────────
    static int dipoleMenu = -1;
    if (survole && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && survoleDipole >= 0) {
        dipoleMenu = survoleDipole;
        app.selection = survoleDipole;
        ImGui::OpenPopup("menuDipole");
    }
    if (ImGui::BeginPopup("menuDipole")) {
        if (dipoleMenu >= 0 && dipoleMenu < (int)app.circuit.elems.size()) {
            Elem& e = app.circuit.elems[dipoleMenu];
            ImGui::TextDisabled("%s — %s", e.nom.c_str(), nomKind(e.kind));
            ImGui::Separator();

            if (e.kind == Kind::Lampe || e.kind == Kind::Del) {
                if (!e.grillee && ImGui::MenuItem("Faire griller (panne)")) {
                    app.memoriser();
                    e.grillee = true;
                    app.note(e.nom + " est grillée : observe ce que deviennent les autres dipôles.");
                    app.recalculer();
                }
                if (e.grillee && ImGui::MenuItem("Remplacer")) {
                    app.memoriser();
                    e.grillee = false;
                    app.recalculer();
                }
            }
            if (e.kind == Kind::Multimetre && e.grillee && ImGui::MenuItem("Remplacer le fusible")) {
                app.memoriser();
                e.grillee = false;
                app.recalculer();
            }
            if (e.kind == Kind::Interrupteur && ImGui::MenuItem(e.ferme ? "Ouvrir" : "Fermer")) {
                app.memoriser();
                e.ferme = !e.ferme;
                app.recalculer();
            }
            if (e.kind == Kind::Pile && ImGui::MenuItem("Inverser les bornes + et −")) {
                app.memoriser();
                std::swap(e.a, e.b);
                app.recalculer();
            }
            if (ImGui::MenuItem("Retourner le dipôle")) {
                app.memoriser();
                std::swap(e.a, e.b);
                app.recalculer();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Supprimer")) {
                app.memoriser();
                app.circuit.supprimer(dipoleMenu);
                app.selection = -1;
                dipoleMenu = -1;
                app.recalculer();
            }
        }
        ImGui::EndPopup();
    }

    // ── Quadrillage ──────────────────────────────────────────────────────
    {
        const float pas = kGrille * app.zoom;
        const float x0 = std::fmod(app.pan.x - origine.x, pas);
        const float y0 = std::fmod(app.pan.y - origine.y, pas);
        const int   i0 = (int)std::floor((origine.x + x0 - app.pan.x) / pas);
        const int   j0 = (int)std::floor((origine.y + y0 - app.pan.y) / pas);
        int i = i0;
        for (float x = origine.x + x0; x < origine.x + taille.x; x += pas, ++i)
            dl->AddLine(ImVec2(x, origine.y), ImVec2(x, origine.y + taille.y),
                        (i % 5 == 0) ? kColGrille5 : kColGrille);
        int j = j0;
        for (float y = origine.y + y0; y < origine.y + taille.y; y += pas, ++j)
            dl->AddLine(ImVec2(origine.x, y), ImVec2(origine.x + taille.x, y),
                        (j % 5 == 0) ? kColGrille5 : kColGrille);
    }

    // ── Dipôles ──────────────────────────────────────────────────────────
    for (int k = 0; k < (int)app.circuit.elems.size(); ++k) {
        const Elem& e = app.circuit.elems[k];
        const ImVec2 A = versEcran(app, e.a);
        const ImVec2 B = versEcran(app, e.b);

        ImU32 couleur = kColEncre;
        if (!app.diag.boucleFermee && e.kind != Kind::Fil) couleur = kColEteint;
        if (e.grillee) couleur = kColGrillee;

        if (k == app.selection) {
            const ImVec2 n = perpendiculaire(normalise(B - A)) * (14.0f * app.zoom);
            dl->AddQuadFilled(A + n, B + n, B - n, A - n, IM_COL32(20, 110, 200, 28));
        }
        if (k == survoleDipole && k != app.selection)
            dl->AddCircle(ImVec2((A.x + B.x) / 2, (A.y + B.y) / 2), 16.0f * app.zoom, kColAide, 20, 1.5f);

        dessineSymbole(dl, e, A, B, app.zoom, k == app.selection ? kColSelection : couleur);
        dessineCourant(dl, app, e, A, B);
        dessineValeurs(dl, app, e, A, B);
    }

    // ── Nœuds ────────────────────────────────────────────────────────────
    if (app.reglages.afficherNoeuds) {
        for (int nd = 0; nd < app.circuit.net.nbNoeuds; ++nd) {
            if (!app.circuit.estNoeudDeDerivation(nd)) continue;
            const ImVec2 p = versEcran(app, app.circuit.net.position[nd]);
            dl->AddCircleFilled(p, 5.0f * app.zoom, kColNoeud, 14);
        }
    }

    // Tracé en cours.
    if (app.trace) {
        const ImVec2 A = versEcran(app, app.traceDepart);
        const ImVec2 B = versEcran(app, app.traceFin);
        dl->AddLine(A, B, kColSelection, 2.0f);
        dl->AddCircleFilled(A, 4.0f, kColSelection, 12);
        dl->AddCircleFilled(B, 4.0f, kColSelection, 12);
    }

    // Repère du point de grille visé, pour poser proprement.
    if (survole && app.outil != Outil::Selection) {
        const ImVec2 p = versEcran(app, versGrille(app, souris));
        dl->AddCircle(p, 6.0f * app.zoom, kColAide, 12, 2.0f);
    }

    // ── Bandeau de message ───────────────────────────────────────────────
    // Le Journal n'existe qu'en mode complet : en mode simple, le dernier
    // message (interrupteur manœuvré, court-circuit, lampe grillée, valeur
    // attendue du montage) s'affiche au bas du schéma.
    if (app.reglages.modeSimple && !app.journal.empty()) {
        const float h = ImGui::GetFontSize() + 12.0f;
        const ImVec2 a(origine.x, origine.y + taille.y - h);
        const ImVec2 b(origine.x + taille.x, origine.y + taille.y);
        dl->AddRectFilled(a, b, IM_COL32(248, 248, 244, 235));
        dl->AddLine(a, ImVec2(b.x, a.y), IM_COL32(180, 185, 190, 255));
        dl->AddText(ImVec2(a.x + 10.0f, a.y + 6.0f), IM_COL32(45, 50, 60, 255),
                    app.journal.back().c_str());
    }

    dl->PopClipRect();

    // ── Clavier ──────────────────────────────────────────────────────────
    if (ImGui::IsWindowFocused() || actif) {
        if (ImGui::IsKeyPressed(ImGuiKey_Delete) || ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
            if (app.selection >= 0 && app.selection < (int)app.circuit.elems.size()) {
                app.memoriser();
                app.circuit.supprimer(app.selection);
                app.selection = -1;
                app.recalculer();
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            app.outil = Outil::Selection;
            app.trace = false;
        }
        if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z)) app.annuler();
    }

    ImGui::End();
}

} // namespace croc
