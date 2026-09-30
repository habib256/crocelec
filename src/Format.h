// Format.h — écriture des grandeurs à la française.
//
// Le § 5 du cahier des charges impose la virgule décimale, l'espace comme
// séparateur de milliers, l'unité toujours affichée et trois chiffres
// significatifs (afficheur 3½ digits). Tout passe par ici : un affichage
// "0.25 A" au lieu de "250 mA" est une erreur de fond, pas de présentation.
#pragma once
#include <string>

namespace croc {

// "Ω" si la police chargée le sait dessiner, "ohm" sinon (police par défaut
// d'ImGui : ProggyClean n'a pas de glyphe grec). Fixé une fois au démarrage.
extern const char* kOhm;

// Nombre seul, virgule décimale, `sig` chiffres significatifs, espace fine
// insécable comme séparateur de milliers.
std::string fmtNombre(double v, int sig = 3);

std::string fmtTension(double u);     // 1,80 V   ·  12,8 V  ·  230 V
std::string fmtIntensite(double i);   // 190 mA   ·  0,220 A → 220 mA · 5,00 A
std::string fmtResistance(double r);  // 60,0 Ω   ·  1,00 kΩ ·  10,0 MΩ
std::string fmtPuissance(double p);   // 0,600 W  ·  2,30 kW
std::string fmtEnergie(double joules);

} // namespace croc
