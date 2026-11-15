// ============================================================================
//  Cilindro: reproduce una melodia como lo haria el rodillo de puas.
//   - Libreria = melodias escritas a mano (Melodies.h) + generadas desde MIDI
//     (MelodyData.h) + las que el plugin carga de la carpeta del usuario.
//   - Edad: cada pua tiene defectos fijos (mal colocada, floja, rota, doblada).
//   - Wear: averias de velocidad del regulador (lloro, deriva, atascos).
//   - Spring: cuerda que se agota y frena la musica.
// ============================================================================
#pragma once
#include "Melodies.h"
#include "MelodyData.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace mbox
{
struct Pin { double beat; int note; float vel; float jitter; float defect; };

struct Melody
{
    std::string title, composer;   // UTF-8
    double bpm = 90.0;             // negras por minuto (tempo nominal)
    double lengthBeats = 0.0;
    std::vector<Pin> pins;
    int lowNote = 60, highNote = 84;

    void finish (uint32_t seed)
    {
        auto rnd = [&seed] { seed = seed * 1664525u + 1013904223u; return (float) (seed >> 8) / 16777216.0f; };
        std::sort (pins.begin(), pins.end(), [] (const Pin& a, const Pin& b) { return a.beat < b.beat; });
        lowNote = 127; highNote = 0;
        for (auto& p : pins)
        {
            p.jitter = rnd() * 2.0f - 1.0f; p.defect = rnd();
            lowNote = std::min (lowNote, p.note); highNote = std::max (highNote, p.note);
        }
    }
};

inline std::vector<Melody> buildLibrary()
{
    std::vector<Melody> lib;
    for (auto& m : tokenMelodies())
    {
        Melody out; out.title = m.title; out.composer = m.composer; out.bpm = m.bpm;
        double units = 0.0;
        for (auto* voice : m.voices)
        {
            double t = 0.0;
            const char* s = voice;
            while (*s)
            {
                while (*s == ' ') ++s;
                if (! *s) break;
                int notes[6]; int nn = 0; bool rest = false;
                for (;;)
                {
                    if (*s == 'r') { rest = true; ++s; }
                    else
                    {
                        static const int semis[7] = { 9, 11, 0, 2, 4, 5, 7 }; // A..G
                        int semi = semis[*s - 'A']; ++s;
                        if (*s == '#') { ++semi; ++s; } else if (*s == 'b') { --semi; ++s; }
                        const int oct = *s - '0'; ++s;
                        if (nn < 6) notes[nn++] = 12 * (oct + 1) + semi + m.transpose;
                    }
                    if (*s == '+') { ++s; continue; }
                    break;
                }
                double dur = 0.0;
                if (*s == ':') { ++s; while (*s >= '0' && *s <= '9') { dur = dur * 10.0 + (*s - '0'); ++s; } }
                if (! rest)
                    for (int i = 0; i < nn; ++i)
                        out.pins.push_back ({ t / (double) m.unitsPerBeat, notes[i], 0.76f, 0.0f, 0.0f });
                t += dur;
            }
            units = std::max (units, t);
        }
        out.lengthBeats = units / (double) m.unitsPerBeat;
        out.finish (977u + (uint32_t) lib.size() * 7919u);
        lib.push_back (std::move (out));
    }
    for (int i = 0; i < kNumGeneratedMelodies; ++i)
    {
        const auto& g = kGeneratedMelodies[i];
        Melody out; out.title = g.title; out.composer = g.composer; out.bpm = g.bpm; out.lengthBeats = g.lengthBeats;
        out.pins.reserve ((size_t) g.count);
        for (int k = 0; k < g.count; ++k)
            out.pins.push_back ({ (double) g.pins[k].beat, (int) g.pins[k].note, 0.5f + 0.38f * (float) g.pins[k].vel / 127.0f, 0.0f, 0.0f });
        out.finish (977u + (uint32_t) lib.size() * 7919u);
        lib.push_back (std::move (out));
    }
    return lib;
}

class Cylinder
{
public:
    void setMelody (const Melody* m) { mel = m; pos = 0.0; }
    const Melody* melody() const { return mel; }
    void rewind() { pos = 0.0; }
    double position01() const { return (mel && mel->lengthBeats > 0) ? pos / mel->lengthBeats : 0.0; }

    // Avanza numSamples a beatsPerSecond. Llama a fn(sampleOffset, note, vel) por cada pua que
    // llega al peine. Con stopAtEnd se detiene al completar la vuelta y devuelve true.
    template <typename Fn>
    bool advance (int numSamples, double sr, double beatsPerSecond, float age, bool stopAtEnd, Fn&& fn)
    {
        if (mel == nullptr || mel->lengthBeats <= 0.0 || beatsPerSecond <= 0.0) return false;
        const double L = mel->lengthBeats;
        const double perSample = beatsPerSecond / sr, start = pos, len = perSample * numSamples;
        const float a = age;
        const double jitterBeats = (double) (a * a) * 0.07 * mel->bpm / 60.0;   // hasta +-70 ms
        const float older = std::max (0.0f, a - 0.25f), old2 = std::max (0.0f, a - 0.15f), old3 = std::max (0.0f, a - 0.4f);
        const float missing = older * older * 0.4f;            // puas rotas
        const float weak = missing + old2 * old2 * 0.35f;      // puas gastadas
        const float bent = 1.0f - old3 * old3 * 0.28f;         // puas dobladas: pulsan la lamina de al lado
        for (const auto& p : mel->pins)
        {
            double t = p.beat + (double) p.jitter * jitterBeats;
            if (stopAtEnd) t = std::clamp (t, 0.0, L - 1.0e-6);
            else { t = std::fmod (t, L); if (t < 0.0) t += L; }
            double d = t - start;
            if (d < 0.0) { if (stopAtEnd) continue; d += L; }
            if (d >= len) { if (! stopAtEnd && p.beat > start + len + jitterBeats + 1.0e-9 && start + len < L) break; continue; }
            if (p.defect < missing) continue;
            float v = p.vel * (1.0f - 0.25f * a * std::abs (p.jitter));
            if (p.defect < weak) v *= 0.3f;
            int note = p.note;
            if (p.defect > bent) note += (p.jitter > 0.0f ? 1 : -1) * (p.defect > 0.5f * (1.0f + bent) ? 2 : 1);
            fn (std::min (numSamples - 1, (int) (d / perSample)), note, v);
        }
        pos = start + len;
        if (pos >= L) { pos = stopAtEnd ? 0.0 : std::fmod (pos, L); return true; }
        return false;
    }

private:
    const Melody* mel = nullptr;
    double pos = 0.0;
};

// Averias mecanicas ligadas a la edad: el regulador ya no mantiene la velocidad.
struct Wear
{
    double ph1 = 0.0, ph2 = 0.0;
    float walk = 0.0f, stallLeft = 0.0f, surgeLeft = 0.0f, cur = 1.0f;
    uint32_t rng = 0xbeef1234u;
    float rnd() { rng = rng * 1664525u + 1013904223u; return (float) (rng >> 8) / 16777216.0f; }

    // factor de velocidad para el siguiente bloque de dt segundos
    float next (float age, float dt)
    {
        const float a2 = age * age;
        ph1 += dt * 6.2832 * 0.31; if (ph1 > 6.2832) ph1 -= 6.2832;      // lloro lento (muelle)
        ph2 += dt * 6.2832 * 2.7;  if (ph2 > 6.2832) ph2 -= 6.2832;      // excentricidad del engranaje
        walk += -walk * dt * 1.2f + (rnd() - 0.5f) * std::sqrt (dt) * 2.4f;  // deriva aleatoria
        float target = 1.0f + a2 * (0.30f * (float) std::sin (ph1) + 0.09f * (float) std::sin (ph2) + 0.38f * walk);
        if (stallLeft > 0.0f)                      // atasco: casi se para...
        {
            stallLeft -= dt; target = 0.02f;
            if (stallLeft <= 0.0f) surgeLeft = surgeTime;
        }
        else if (surgeLeft > 0.0f) { surgeLeft -= dt; target *= 1.9f; }   // ...y sale disparado
        else if (age > 0.35f)
        {
            const float rate = (age - 0.35f) * (age - 0.35f) * 2.6f;      // atascos por segundo
            if (rnd() < rate * dt) { stallLeft = (0.05f + rnd() * 0.75f) * age; surgeTime = stallLeft * 0.5f; }
        }
        cur += (target - cur) * std::min (1.0f, dt / 0.035f);
        return std::clamp (cur, 0.0f, 3.0f);
    }
    float surgeTime = 0.0f;
};

// Cuerda: 1 = llena. Se gasta al sonar; al final el regulador pierde velocidad y se para.
struct Spring
{
    float level = 1.0f;
    static constexpr float kRunSeconds = 75.0f;
    float speedFactor() const
    {
        if (level <= 0.0f) return 0.0f;
        if (level >= 0.22f) return 1.0f;
        return 0.38f + 0.62f * std::sqrt (level / 0.22f);
    }
    void consume (float seconds) { level = std::max (0.0f, level - seconds / kRunSeconds); }
    void wind (float amount)     { level = std::min (1.0f, level + amount); }
};
} // namespace mbox
