#include "Format.h"

#include <cmath>
#include <cstdio>
#include <cstring>

namespace croc {

const char* kOhm = "ohm"; // remplacé par "Ω" si la police le permet (App.cpp)

namespace {

// Arrondit à `sig` chiffres significatifs et rend la chaîne correspondante,
// point décimal encore à l'anglaise (converti juste après).
std::string bruteSig(double v, int sig)
{
    if (v == 0.0) return "0";
    double av = std::fabs(v);
    int expo = (int)std::floor(std::log10(av));
    int dec  = sig - 1 - expo;          // chiffres après la virgule
    if (dec < 0)  dec = 0;
    if (dec > 12) dec = 12;
    char buf[64];
    std::snprintf(buf, sizeof buf, "%.*f", dec, v);
    // 60.0 reste "60,0" (le cours écrit les zéros significatifs), mais on
    // évite "0,220000" quand l'arrondi a déjà tout dit.
    return std::string(buf);
}

std::string francise(std::string s)
{
    // Point décimal → virgule.
    for (char& c : s)
        if (c == '.') c = ',';

    // Séparateur de milliers : "1540" → "1 540" (partie entière seulement).
    std::size_t virgule = s.find(',');
    std::size_t fin = (virgule == std::string::npos) ? s.size() : virgule;
    std::size_t debut = (!s.empty() && (s[0] == '-')) ? 1 : 0;
    if (fin - debut > 4) { // on ne coupe pas les nombres à 4 chiffres (1500 ha)
        for (std::size_t pos = fin; pos > debut + 3;) {
            pos -= 3;
            s.insert(pos, " ");
            fin++;
        }
    }
    return s;
}

// Choisit le multiple qui donne une lecture de collégien : on veut 190 mA,
// pas 0,19 A — c'est l'unité que l'élève lit sur le multimètre.
std::string avecPrefixe(double v, const char* unite, int sig)
{
    double av = std::fabs(v);
    const char* pref = "";
    double mult = 1.0;

    if (av == 0.0)            { pref = "";  mult = 1.0; }
    else if (av < 1e-6)       { pref = "n"; mult = 1e9; }
    else if (av < 1e-3)       { pref = "\xC2\xB5"; mult = 1e6; }  // µ
    else if (av < 1.0)        { pref = "m"; mult = 1e3; }
    else if (av < 1e3)        { pref = "";  mult = 1.0; }
    else if (av < 1e6)        { pref = "k"; mult = 1e-3; }
    else                      { pref = "M"; mult = 1e-6; }

    std::string s = francise(bruteSig(v * mult, sig));
    s += " ";
    s += pref;
    s += unite;
    return s;
}

} // namespace

std::string fmtNombre(double v, int sig) { return francise(bruteSig(v, sig)); }

// En dessous de ces seuils, la valeur n'est plus une mesure mais le résidu du
// modèle : un fil et un interrupteur fermé valent 0,1 mΩ, ce qui donnerait des
// « 9,54 µV » aux bornes d'un interrupteur — vrai numériquement, absurde pour
// un élève de 4e qui vient de lire « l'interrupteur fermé ne consomme rien ».
namespace {
constexpr double kZeroTension   = 1e-3;   // 1 mV
constexpr double kZeroIntensite = 1e-6;   // 1 µA
constexpr double kZeroPuissance = 1e-6;   // 1 µW
constexpr double kZeroResistance = 1e-2;  // 10 mΩ
const char* kPresqueZero = "\xE2\x89\x88 0 ";   // « ≈ 0 »
} // namespace

std::string fmtTension(double u)
{
    if (std::fabs(u) < kZeroTension) return std::string(kPresqueZero) + "V";
    return avecPrefixe(u, "V", 3);
}

std::string fmtIntensite(double i)
{
    if (std::fabs(i) < kZeroIntensite) return std::string(kPresqueZero) + "A";
    return avecPrefixe(i, "A", 3);
}

std::string fmtPuissance(double p)
{
    if (std::fabs(p) < kZeroPuissance) return std::string(kPresqueZero) + "W";
    return avecPrefixe(p, "W", 3);
}

std::string fmtResistance(double r)
{
    // L'unité porte un glyphe grec : on la colle après coup pour laisser
    // avecPrefixe() gérer le multiple (kΩ, MΩ) sans connaître kOhm.
    if (std::fabs(r) < kZeroResistance) return std::string(kPresqueZero) + kOhm;
    // avecPrefixe rend "60,0 " (pas de multiple, l'espace est déjà là) ou
    // "1,00 k" (multiple collé) : dans les deux cas kOhm se colle au bout.
    return avecPrefixe(r, "", 3) + kOhm;
}

std::string fmtEnergie(double joules)
{
    if (std::fabs(joules) >= 3.6e5) // au-delà de 0,1 kWh, le kWh parle plus
        return francise(bruteSig(joules / 3.6e6, 3)) + " kWh";
    return avecPrefixe(joules, "J", 3);
}

} // namespace croc
