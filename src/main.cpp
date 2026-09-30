// main.cpp — fenêtre, boucle principale et DockSpace.
//
// Dear ImGui branche docking : les panneaux de CrocElec forment une
// disposition persistante (Schéma, Palette, Propriétés, Mesures, Lois,
// Journal, Bibliothèque) au lieu d'une pile de fenêtres flottantes.
//
// Le multi-viewport de la branche docking est volontairement laissé OFF :
// il n'apporte rien en classe (un seul écran, souvent vidéoprojeté) et il
// n'existe pas dans la future version WebAssembly, cible Galaxy Tab 4.
#include "App.h"
#include "Format.h"
#include "Serialize.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#endif
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstring>
#include <string>

namespace {

void erreurGlfw(int code, const char* description)
{
    std::fprintf(stderr, "GLFW %d : %s\n", code, description);
}

// Cherche une police système capable de dessiner les accents ET le symbole Ω.
// À défaut, on garde la police intégrée d'ImGui et l'ohm s'écrit « ohm » :
// mieux vaut un mot lisible qu'un carré vide au tableau.
bool chargerPolice(ImGuiIO& io)
{
    const char* candidats[] = {
        "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/Library/Fonts/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "C:\\Windows\\Fonts\\segoeui.ttf",
        "C:\\Windows\\Fonts\\arial.ttf",
    };
    for (const char* chemin : candidats) {
        if (FILE* f = std::fopen(chemin, "rb")) {
            std::fclose(f);
            if (io.Fonts->AddFontFromFileTTF(chemin, 17.0f)) {
                croc::kOhm = "\xCE\xA9"; // Ω en UTF-8
                return true;
            }
        }
    }
    return false;
}

// Deux dispositions : le schéma au centre dans les deux cas.
// Mode simple (élève) : Palette et Propriétés à gauche, Mesures et Lois en
// onglets à droite, Bibliothèque en bas — cinq panneaux, rien de plus.
// Mode complet (professeur) : s'y ajoutent Multimètre, Relevés, Graphique,
// Énergie et Journal.
void construireDisposition(ImGuiID dockspace, bool simple)
{
    ImGui::DockBuilderRemoveNode(dockspace);
    ImGui::DockBuilderAddNode(dockspace, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace, ImGui::GetMainViewport()->WorkSize);

    ImGuiID centre = dockspace;
    const ImGuiID gauche = ImGui::DockBuilderSplitNode(centre, ImGuiDir_Left, simple ? 0.19f : 0.17f, nullptr, &centre);
    const ImGuiID droite = ImGui::DockBuilderSplitNode(centre, ImGuiDir_Right, simple ? 0.26f : 0.28f, nullptr, &centre);
    const ImGuiID bas    = ImGui::DockBuilderSplitNode(centre, ImGuiDir_Down, simple ? 0.22f : 0.25f, nullptr, &centre);
    ImGuiID gaucheBas    = 0;
    const ImGuiID gaucheHaut = ImGui::DockBuilderSplitNode(gauche, ImGuiDir_Up, 0.55f, nullptr, &gaucheBas);

    ImGui::DockBuilderDockWindow("Palette", gaucheHaut);
    ImGui::DockBuilderDockWindow("Propriétés", gaucheBas);
    ImGui::DockBuilderDockWindow("Schéma", centre);
    ImGui::DockBuilderDockWindow("Bibliothèque", bas);

    if (simple) {
        // Mesures et Lois partagent le même emplacement : un seul onglet
        // visible à la fois, Lois devant (c'est lui que le cours utilise).
        ImGui::DockBuilderDockWindow("Mesures", droite);
        ImGui::DockBuilderDockWindow("Lois", droite);
    } else {
        ImGuiID droiteBas = 0;
        const ImGuiID droiteHaut = ImGui::DockBuilderSplitNode(droite, ImGuiDir_Up, 0.5f, nullptr, &droiteBas);
        ImGui::DockBuilderDockWindow("Multimètre", gaucheBas);
        ImGui::DockBuilderDockWindow("Mesures", droiteHaut);
        ImGui::DockBuilderDockWindow("Relevés", droiteHaut);
        ImGui::DockBuilderDockWindow("Lois", droiteBas);
        ImGui::DockBuilderDockWindow("Graphique", droiteBas);
        ImGui::DockBuilderDockWindow("Énergie", droiteBas);
        ImGui::DockBuilderDockWindow("Journal", bas);
    }
    ImGui::DockBuilderFinish(dockspace);
}

// Le mode simple agrandit tout le texte : lisible pour un élève au fond de
// la salle, et cohérent avec les cibles tactiles du cahier des charges.
void appliquerEchelleDuMode(bool simple)
{
    ImGui::GetStyle().FontScaleMain = simple ? 1.2f : 1.0f;
}

void styleCrocElec(ImGuiStyle& style)
{
    ImGui::StyleColorsLight(&style);           // thème clair : lisible au vidéoprojecteur
    style.WindowRounding    = 4.0f;
    style.FrameRounding     = 3.0f;
    style.GrabRounding      = 3.0f;
    style.WindowPadding     = ImVec2(10, 10);
    style.FramePadding      = ImVec2(8, 5);
    style.ItemSpacing       = ImVec2(8, 7);
    style.ScrollbarSize     = 14.0f;
    style.WindowTitleAlign  = ImVec2(0.02f, 0.5f);
    style.Colors[ImGuiCol_WindowBg]      = ImVec4(0.97f, 0.97f, 0.96f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.83f, 0.86f, 0.90f, 1.00f);
    style.Colors[ImGuiCol_Header]        = ImVec4(0.78f, 0.84f, 0.92f, 1.00f);
    style.Colors[ImGuiCol_Button]        = ImVec4(0.88f, 0.90f, 0.93f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.78f, 0.85f, 0.94f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive]  = ImVec4(0.60f, 0.74f, 0.90f, 1.00f);
}

} // namespace

int main(int, char**)
{
    glfwSetErrorCallback(erreurGlfw);
    if (!glfwInit()) return 1;

#if defined(__APPLE__)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    const char* glsl_version = "#version 150";
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    const char* glsl_version = "#version 130";
#endif

    const float echelle = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
    GLFWwindow* fenetre = glfwCreateWindow((int)(1440 * echelle), (int)(900 * echelle),
                                           "CrocElec — simulateur de circuits électriques", nullptr, nullptr);
    if (!fenetre) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(fenetre);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigDpiScaleFonts = true;
    io.IniFilename = "crocelec-v4.ini";  // la disposition survit à la fermeture
                                     // (suffixe de version : une disposition enregistrée
                                     //  avant l'ajout d'un panneau le laisserait flotter)

    styleCrocElec(ImGui::GetStyle());
    ImGui::GetStyle().ScaleAllSizes(echelle);
    ImGui::GetStyle().FontScaleDpi = echelle;

    chargerPolice(io);

    ImGui_ImplGlfw_InitForOpenGL(fenetre, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    croc::App app;
    croc::exemples()[0].construire(app.circuit);
    app.note("Bienvenue. Choisis un montage dans la Bibliothèque, ou construis le tien avec la Palette.");
    app.recalculer();
    appliquerEchelleDuMode(app.reglages.modeSimple);

    bool dispositionFaite = false;
    bool afficherApropos = false;
    bool demandeOuvrir = false;
    bool demandeEnregistrer = false;
    // Pas de sélecteur de fichiers natif : une seule zone de saisie, qui se
    // comporte pareil sur les trois plateformes et dans la future version web.
    static char chemin[512] = "montage.croc";

    while (!glfwWindowShouldClose(fenetre)) {
        glfwPollEvents();
        if (glfwGetWindowAttrib(fenetre, GLFW_ICONIFIED)) { glfwWaitEventsTimeout(0.1); continue; }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        app.tempsAnim += ImGui::GetIO().DeltaTime;
        app.avancerCompteur(ImGui::GetIO().DeltaTime);

        const ImGuiID dockspace = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
        if (!dispositionFaite) {
            if (!ImGui::DockBuilderGetNode(dockspace) ||
                ImGui::DockBuilderGetNode(dockspace)->IsEmpty())
                construireDisposition(dockspace, app.reglages.modeSimple);
            dispositionFaite = true;
        }

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Fichier")) {
                if (ImGui::MenuItem("Nouveau montage", "Ctrl+N")) {
                    app.memoriser();
                    app.circuit.vider();
                    app.selection = -1;
                    app.note("Nouveau montage.");
                    app.recalculer();
                }
                if (ImGui::MenuItem("Ouvrir un montage...", "Ctrl+O")) demandeOuvrir = true;
                if (ImGui::MenuItem("Enregistrer", "Ctrl+S", false, !app.fichierCourant.empty())) {
                    std::string erreur;
                    if (croc::enregistrer(app.circuit, app.fichierCourant, erreur))
                        app.note("Montage enregistré dans " + app.fichierCourant);
                    else
                        app.note("Échec de l'enregistrement : " + erreur);
                }
                if (ImGui::MenuItem("Enregistrer sous...")) demandeEnregistrer = true;
                ImGui::Separator();
                if (ImGui::MenuItem("Quitter")) glfwSetWindowShouldClose(fenetre, 1);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Édition")) {
                if (ImGui::MenuItem("Annuler", "Ctrl+Z")) app.annuler();
                if (!app.reglages.modeSimple) {
                    ImGui::Separator();
                    if (ImGui::MenuItem("Relever les mesures du dipôle choisi", "Ctrl+R")) app.relever();
                    if (ImGui::MenuItem("Effacer le tableau de relevés")) app.releves.clear();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Affichage")) {
                // En mode simple, ces réglages restent ceux de la barre du
                // Schéma : le menu ne garde que les deux actions utiles.
                if (!app.reglages.modeSimple) {
                    ImGui::MenuItem("Valeurs sur les dipôles", nullptr, &app.reglages.afficherValeurs);
                    ImGui::MenuItem("Noms des dipôles", nullptr, &app.reglages.afficherNoms);
                    ImGui::MenuItem("Pastilles sur les nœuds", nullptr, &app.reglages.afficherNoeuds);
                    ImGui::MenuItem("Animer le courant", nullptr, &app.reglages.animerCourant);
                    ImGui::MenuItem("Sens de déplacement des électrons", nullptr, &app.reglages.sensElectrons);
                    ImGui::Separator();
                }
                if (ImGui::MenuItem("Ajuster la vue sur le montage")) app.ajusterVue = true;
                if (ImGui::MenuItem("Réinitialiser la disposition"))
                    construireDisposition(dockspace, app.reglages.modeSimple);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Mode")) {
                if (ImGui::MenuItem("Simple (élève)", nullptr, app.reglages.modeSimple) &&
                    !app.reglages.modeSimple) {
                    app.reglages.modeSimple = true;
                    if (app.outil == croc::Outil::Multimetre) app.outil = croc::Outil::Selection;
                    appliquerEchelleDuMode(true);
                    construireDisposition(dockspace, true);
                }
                if (ImGui::MenuItem("Complet (professeur)", nullptr, !app.reglages.modeSimple) &&
                    app.reglages.modeSimple) {
                    app.reglages.modeSimple = false;
                    appliquerEchelleDuMode(false);
                    construireDisposition(dockspace, false);
                }
                ImGui::Separator();
                ImGui::TextDisabled("Le mode complet ajoute le multimètre,\nles relevés, le graphique, l'énergie\net le journal.");
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Aide")) {
                if (ImGui::MenuItem("À propos")) afficherApropos = true;
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        croc::panneauSchema(app);
        croc::panneauPalette(app);
        croc::panneauProprietes(app);
        croc::panneauMesures(app);
        croc::panneauLois(app);
        croc::panneauBibliotheque(app);
        if (!app.reglages.modeSimple) {
            croc::panneauMultimetre(app);
            croc::panneauReleves(app);
            croc::panneauGraphique(app);
            croc::panneauEnergie(app);
            croc::panneauJournal(app);
        }

        {
            const ImGuiIO& clavier = ImGui::GetIO();
            if (clavier.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O)) demandeOuvrir = true;
            if (clavier.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_R) && !app.reglages.modeSimple) app.relever();
            if (clavier.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
                if (app.fichierCourant.empty()) demandeEnregistrer = true;
                else {
                    std::string erreur;
                    app.note(croc::enregistrer(app.circuit, app.fichierCourant, erreur)
                                 ? "Montage enregistré dans " + app.fichierCourant
                                 : "Échec de l'enregistrement : " + erreur);
                }
            }
        }

        if (demandeOuvrir)      { ImGui::OpenPopup("Ouvrir un montage"); demandeOuvrir = false; }
        if (demandeEnregistrer) { ImGui::OpenPopup("Enregistrer le montage"); demandeEnregistrer = false; }

        if (ImGui::BeginPopupModal("Ouvrir un montage", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextUnformatted("Chemin du fichier .croc :");
            ImGui::SetNextItemWidth(420);
            ImGui::InputText("##chemin", chemin, sizeof chemin);
            if (ImGui::Button("Ouvrir")) {
                std::string erreur;
                app.memoriser();
                if (croc::ouvrir(app.circuit, chemin, erreur)) {
                    app.fichierCourant = chemin;
                    app.selection = -1;
                    app.note(std::string("Montage ouvert : ") + chemin);
                    app.recalculer();
                } else {
                    app.note("Ouverture impossible : " + erreur);
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Annuler")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopupModal("Enregistrer le montage", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextUnformatted("Enregistrer sous :");
            ImGui::SetNextItemWidth(420);
            ImGui::InputText("##cheminEnr", chemin, sizeof chemin);
            if (ImGui::Button("Enregistrer")) {
                std::string erreur;
                if (croc::enregistrer(app.circuit, chemin, erreur)) {
                    app.fichierCourant = chemin;
                    app.note(std::string("Montage enregistré dans ") + chemin);
                } else {
                    app.note("Échec de l'enregistrement : " + erreur);
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Annuler")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        if (afficherApropos) {
            ImGui::OpenPopup("À propos de CrocElec");
            afficherApropos = false;
        }
        if (ImGui::BeginPopupModal("À propos de CrocElec", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextUnformatted("CrocElec — simulateur de circuits électriques pour le collège");
            ImGui::TextDisabled("Physique-Chimie · cycle 4 · symboles normalisés, unités françaises");
            ImGui::Separator();
            ImGui::TextUnformatted("Dear ImGui (branche docking) · GLFW · OpenGL 3");
            if (ImGui::Button("Fermer")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        ImGui::Render();
        int largeur = 0, hauteur = 0;
        glfwGetFramebufferSize(fenetre, &largeur, &hauteur);
        glViewport(0, 0, largeur, hauteur);
        glClearColor(0.93f, 0.93f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(fenetre);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(fenetre);
    glfwTerminate();
    return 0;
}
