// ============================================================================
//  Libreria de melodias del cilindro.
//  Todas las obras son de dominio publico; los arreglos para peine son
//  propios de este proyecto.
//
//  Notacion: "NOTA[+NOTA...]:duracion" separadas por espacios; "r" = silencio.
//  La duracion va en unidades (unitsPerBeat unidades por negra).
// ============================================================================
#pragma once
#include <string>
#include <vector>

namespace mbox
{
struct MelodyDef
{
    const char* title;      // UTF-8
    const char* composer;
    double bpm;           // negras por minuto
    int unitsPerBeat;     // unidades por negra
    int transpose;        // semitonos
    std::vector<const char*> voices;
};

inline const std::vector<MelodyDef>& tokenMelodies()
{
    static const std::vector<MelodyDef> lib = {
    { "El lago de los cisnes", "P. I. Chaikovski", 84, 2, 12, {
        "F#5:4 B4:1 C#5:1 D5:1 E5:1  F#5:3 D5:1 F#5:3 D5:1  F#5:3 B4:1 D5:1 B4:1 G4:1 D5:1  B4:8 "
        "F#5:4 B4:1 C#5:1 D5:1 E5:1  F#5:3 D5:1 F#5:3 D5:1  F#5:3 B4:1 D5:1 B4:1 G4:1 D5:1  B4:8",
        "B3:1 F#4:1 D4:1 F#4:1 B3:1 F#4:1 D4:1 F#4:1  B3:1 F#4:1 D4:1 F#4:1 B3:1 F#4:1 D4:1 F#4:1 "
        "B3:1 F#4:1 D4:1 F#4:1 G3:1 D4:1 B3:1 D4:1  B3:1 F#4:1 D4:1 F#4:1 B3:4 "
        "B3:1 F#4:1 D4:1 F#4:1 B3:1 F#4:1 D4:1 F#4:1  B3:1 F#4:1 D4:1 F#4:1 B3:1 F#4:1 D4:1 F#4:1 "
        "B3:1 F#4:1 D4:1 F#4:1 G3:1 D4:1 B3:1 D4:1  B3:1 F#4:1 D4:1 F#4:1 B3+F#4:4" } },

    { "Para Elisa", "L. van Beethoven", 66, 4, 12, {
        "E5:1 D#5:1  E5:1 D#5:1 E5:1 B4:1 D5:1 C5:1  A4:2 r:1 C4:1 E4:1 A4:1  B4:2 r:1 E4:1 G#4:1 B4:1 "
        "C5:2 r:1 E4:1 E5:1 D#5:1  E5:1 D#5:1 E5:1 B4:1 D5:1 C5:1  A4:2 r:1 C4:1 E4:1 A4:1 "
        "B4:2 r:1 E4:1 C5:1 B4:1  A4:4",
        "r:2  r:6  A2:1 E3:1 A3:1 r:3  E2:1 E3:1 G#3:1 r:3  A2:1 E3:1 A3:1 r:3  r:6 "
        "A2:1 E3:1 A3:1 r:3  E2:1 E3:1 G#3:1 r:3  A2:1 E3:1 A3:1 r:1" } },

    { "Canci\xc3\xb3n de cuna", "J. Brahms", 80, 2, 12, {
        "E4:1 E4:1  G4:3 E4:1 E4:2  G4:4 E4:1 G4:1  C5:2 B4:3 A4:1  A4:2 G4:2 D4:1 E4:1 "
        "F4:2 D4:2 D4:1 E4:1  F4:4 D4:1 F4:1  B4:1 A4:1 G4:2 B4:2  C5:4 C4:1 C4:1 "
        "C5:4 A4:1 F4:1  G4:4 E4:1 C4:1  F4:2 G4:2 A4:2  G4:4 C4:1 C4:1 "
        "C5:4 A4:1 F4:1  G4:4 E4:1 C4:1  F4:2 E4:2 D4:2  C4:4",
        "r:2  C3:2 G3:2 E3:2  C3:2 G3:2 E3:2  C3:2 G3:2 E3:2  G2:2 D3:2 B2:2 "
        "G2:2 D3:2 B2:2  G2:2 D3:2 B2:2  G2:2 D3:2 F3:2  C3:2 G3:2 E3:2 "
        "F2:2 C3:2 A2:2  C3:2 G3:2 E3:2  G2:2 D3:2 B2:2  C3:2 G3:2 E3:2 "
        "F2:2 C3:2 A2:2  C3:2 G3:2 E3:2  G2:2 D3:2 B2:2  C3:2 E3:2" } },

    { "Ah, vous dirai-je, maman", "Trad. / W. A. Mozart", 104, 2, 12, {
        "C5:2 C5:2 G5:2 G5:2  A5:2 A5:2 G5:4  F5:2 F5:2 E5:2 E5:2  D5:2 D5:2 C5:4 "
        "G5:2 G5:2 F5:2 F5:2  E5:2 E5:2 D5:4  G5:2 G5:2 F5:2 F5:2  E5:2 E5:2 D5:4 "
        "C5:2 C5:2 G5:2 G5:2  A5:2 A5:2 G5:4  F5:2 F5:2 E5:2 E5:2  D5:2 D5:2 C5:4",
        "C4:1 G4:1 E4:1 G4:1 C4:1 G4:1 E4:1 G4:1  C4:1 A4:1 F4:1 A4:1 C4:1 G4:1 E4:1 G4:1 "
        "C4:1 A4:1 F4:1 A4:1 C4:1 G4:1 E4:1 G4:1  B3:1 G4:1 D4:1 G4:1 C4:1 G4:1 E4:2 "
        "C4:1 G4:1 E4:1 G4:1 C4:1 A4:1 F4:1 A4:1  C4:1 G4:1 E4:1 G4:1 B3:1 G4:1 D4:1 G4:1 "
        "C4:1 G4:1 E4:1 G4:1 C4:1 A4:1 F4:1 A4:1  C4:1 G4:1 E4:1 G4:1 B3:1 G4:1 D4:1 G4:1 "
        "C4:1 G4:1 E4:1 G4:1 C4:1 G4:1 E4:1 G4:1  C4:1 A4:1 F4:1 A4:1 C4:1 G4:1 E4:1 G4:1 "
        "C4:1 A4:1 F4:1 A4:1 C4:1 G4:1 E4:1 G4:1  B3:1 G4:1 D4:1 G4:1 C4+E4:4" } },

    { "Greensleeves", "Tradicional inglesa", 90, 4, 12, {
        "A4:2  C5:4 D5:2 E5:3 F5:1 E5:2  D5:4 B4:2 G4:3 A4:1 B4:2  C5:4 A4:2 A4:3 G#4:1 A4:2  B4:4 G#4:2 E4:4 A4:2 "
        "C5:4 D5:2 E5:3 F5:1 E5:2  D5:4 B4:2 G4:3 A4:1 B4:2  C5:3 B4:1 A4:2 G#4:3 F#4:1 G#4:2  A4:6 A4:6 "
        "G5:6 G5:3 F#5:1 E5:2  D5:4 B4:2 G4:3 A4:1 B4:2  C5:4 A4:2 A4:3 G#4:1 A4:2  B4:4 G#4:2 E4:6 "
        "G5:6 G5:3 F#5:1 E5:2  D5:4 B4:2 G4:3 A4:1 B4:2  C5:3 B4:1 A4:2 G#4:3 F#4:1 G#4:2  A4:6 A4:4",
        "r:2  A2:2 E3:2 C3:2 A2:2 E3:2 C3:2  G2:2 D3:2 B2:2 G2:2 D3:2 B2:2  A2:2 E3:2 C3:2 A2:2 E3:2 C3:2  E2:2 B2:2 G#2:2 E2:2 B2:2 G#2:2 "
        "A2:2 E3:2 C3:2 A2:2 E3:2 C3:2  G2:2 D3:2 B2:2 G2:2 D3:2 B2:2  A2:2 E3:2 C3:2 E2:2 B2:2 G#2:2  A2:2 E3:2 C3:2 A2:2 E3:2 C3:2 "
        "C3:2 G3:2 E3:2 C3:2 G3:2 E3:2  G2:2 D3:2 B2:2 G2:2 D3:2 B2:2  A2:2 E3:2 C3:2 A2:2 E3:2 C3:2  E2:2 B2:2 G#2:2 E2:2 B2:2 G#2:2 "
        "C3:2 G3:2 E3:2 C3:2 G3:2 E3:2  G2:2 D3:2 B2:2 G2:2 D3:2 B2:2  A2:2 E3:2 C3:2 E2:2 B2:2 G#2:2  A2:2 E3:2 C3:2 A2:4" } },

    { "Himno a la alegr\xc3\xad" "a", "L. van Beethoven", 108, 2, 12, {
        "E5:2 E5:2 F5:2 G5:2  G5:2 F5:2 E5:2 D5:2  C5:2 C5:2 D5:2 E5:2  E5:3 D5:1 D5:4 "
        "E5:2 E5:2 F5:2 G5:2  G5:2 F5:2 E5:2 D5:2  C5:2 C5:2 D5:2 E5:2  D5:3 C5:1 C5:4 "
        "D5:2 D5:2 E5:2 C5:2  D5:2 E5:1 F5:1 E5:2 C5:2  D5:2 E5:1 F5:1 E5:2 D5:2  C5:2 D5:2 G4:4 "
        "E5:2 E5:2 F5:2 G5:2  G5:2 F5:2 E5:2 D5:2  C5:2 C5:2 D5:2 E5:2  D5:3 C5:1 C5:4",
        "C4:1 G4:1 E4:1 G4:1 C4:1 G4:1 E4:1 G4:1  C4:1 G4:1 E4:1 G4:1 B3:1 G4:1 D4:1 G4:1  C4:1 G4:1 E4:1 G4:1 C4:1 G4:1 E4:1 G4:1  C4:1 G4:1 E4:1 G4:1 B3:1 G4:1 D4:2 "
        "C4:1 G4:1 E4:1 G4:1 C4:1 G4:1 E4:1 G4:1  C4:1 G4:1 E4:1 G4:1 B3:1 G4:1 D4:1 G4:1  C4:1 G4:1 E4:1 G4:1 C4:1 G4:1 E4:1 G4:1  B3:1 G4:1 D4:1 G4:1 C4:1 G4:1 E4:2 "
        "B3:1 G4:1 D4:1 G4:1 C4:1 G4:1 E4:1 G4:1  B3:1 G4:1 D4:1 G4:1 C4:1 G4:1 E4:1 G4:1  B3:1 G4:1 D4:1 G4:1 B3:1 G4:1 D4:1 G4:1  A3:1 E4:1 D4:1 F#4:1 G3:1 D4:1 B3:2 "
        "C4:1 G4:1 E4:1 G4:1 C4:1 G4:1 E4:1 G4:1  C4:1 G4:1 E4:1 G4:1 B3:1 G4:1 D4:1 G4:1  C4:1 G4:1 E4:1 G4:1 C4:1 G4:1 E4:1 G4:1  B3:1 G4:1 D4:1 G4:1 C4+E4:4" } },

    { "Minueto en sol", "C. Petzold (cuaderno de A. M. Bach)", 120, 2, 12, {
        "D5:2 G4:1 A4:1 B4:1 C5:1  D5:2 G4:2 G4:2  E5:2 C5:1 D5:1 E5:1 F#5:1  G5:2 G4:2 G4:2 "
        "C5:2 D5:1 C5:1 B4:1 A4:1  B4:2 C5:1 B4:1 A4:1 G4:1  F#4:2 G4:1 A4:1 B4:1 G4:1  B4:2 A4:4 "
        "D5:2 G4:1 A4:1 B4:1 C5:1  D5:2 G4:2 G4:2  E5:2 C5:1 D5:1 E5:1 F#5:1  G5:2 G4:2 G4:2 "
        "C5:2 D5:1 C5:1 B4:1 A4:1  B4:2 C5:1 B4:1 A4:1 G4:1  A4:2 B4:1 A4:1 G4:1 F#4:1  G4:6",
        "G3:4 A3:2  B3:6  C4:6  B3:6  A3:6  G3:6  D4:2 B3:2 G3:2  D4:2 D3:1 C4:1 B3:1 A3:1 "
        "B3:4 A3:2  G3:2 B3:2 G3:2  C4:6  B3:2 C4:1 B3:1 A3:1 G3:1  A3:4 F#3:2  G3:4 B3:2  C4:2 D4:2 D3:2  G3:4 G2:2" } },

    { "Canon en re", "J. Pachelbel", 66, 2, 0, {
        "F#5:4 E5:4 D5:4 C#5:4 B4:4 A4:4 B4:4 C#5:4 "
        "D5:4 C#5:4 B4:4 A4:4 G4:4 F#4:4 G4:4 E4:4 "
        "D5:1 F#5:1 A5:1 G5:1 F#5:1 D5:1 F#5:1 E5:1 D5:1 B4:1 D5:1 A5:1 G5:1 B5:1 A5:1 G5:1 "
        "F#5:1 D5:1 E5:1 C#6:1 D6:1 F#6:1 A6:1 A5:1 B5:1 G5:1 A5:1 F#5:1 D5:1 D6:1 D6:1 C#6:1",
        "D3:1 A3:1 D4:1 F#4:1 A2:1 E3:1 A3:1 C#4:1 B2:1 F#3:1 B3:1 D4:1 F#2:1 C#3:1 F#3:1 A3:1 "
        "G2:1 D3:1 G3:1 B3:1 D3:1 A3:1 D4:1 F#4:1 G2:1 D3:1 G3:1 B3:1 A2:1 E3:1 A3:1 C#4:1 "
        "D3:1 A3:1 D4:1 F#4:1 A2:1 E3:1 A3:1 C#4:1 B2:1 F#3:1 B3:1 D4:1 F#2:1 C#3:1 F#3:1 A3:1 "
        "G2:1 D3:1 G3:1 B3:1 D3:1 A3:1 D4:1 F#4:1 G2:1 D3:1 G3:1 B3:1 A2:1 E3:1 A3:1 C#4:1 "
        "D3:1 A3:1 D4:1 F#4:1 A2:1 E3:1 A3:1 C#4:1 B2:1 F#3:1 B3:1 D4:1 F#2:1 C#3:1 F#3:1 A3:1 "
        "G2:1 D3:1 G3:1 B3:1 D3:1 A3:1 D4:1 F#4:1 G2:1 D3:1 G3:1 B3:1 A2:1 E3:1 A3:1 C#4:1" } },
    };
    return lib;
}
} // namespace mbox
