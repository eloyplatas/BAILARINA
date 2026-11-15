// ============================================================================
//  Bailarina - motor de caja de musica por modelado fisico (sintesis modal)
//  Sin dependencias: C++17 puro, se puede usar fuera de JUCE.
//
//  Modelo:
//   - Cada nota MIDI es una lamina del peine: barra empotrada-libre
//     (Euler-Bernoulli). Parciales inarmonicos 1 : 6.267 : 17.547 : 34.386.
//   - Cada modo es un fasor complejo amortiguado (forma acoplada), estable
//     y barato. La salida es la componente "velocidad" (arranque en seno).
//   - Pua: fase de contacto (amortigua la vibracion previa y genera el
//     zumbido caracteristico del re-pulsado) y suelta (desplazamiento inicial).
//   - Apagadores: reducen zumbido y acortan el contacto.
//   - Caja: banco paralelo de resonadores (modos de la madera).
//   - Mecanismo: zumbido del regulador de aspas + tics del engranaje.
// ============================================================================
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace mbox
{
struct Params
{
    float brightness = 0.55f; // 0..1   peso de los parciales agudos
    float decay      = 2.5f;  // s      T60 de la fundamental en La4 (0.05 .. 30)
    float click      = 0.35f; // 0..1   ruido de la pua
    float dampers    = 0.6f;  // 0..1   apagadores (0 = zumba al re-pulsar)
    float age        = 0.2f;  // 0..1   desgaste: desafinacion, laminas oxidadas, mecanismo aspero
    float body       = 0.5f;  // 0..1   cantidad de caja de madera
    float size       = 0.5f;  // 0..1   tamano de la caja
    float mechanism  = 0.15f; // 0..1   ruido del mecanismo
    float width      = 0.6f;  // 0..1   anchura estereo del peine
    float volume     = 0.8f;  // ganancia lineal
    float tune       = 0.0f;  // semitonos
};

class Engine
{
public:
    static constexpr int kNotes = 128;
    static constexpr int kModes = 4;
    static constexpr int kBody  = 10;

    void prepare (double sampleRate)
    {
        sr = sampleRate;
        for (auto& t : teeth) t = Tooth{};
        numActive = 0;
        for (int n = 0; n < kNotes; ++n)
        {
            // desafinacion fija y repetible por lamina (hash de la nota)
            uint32_t h = (uint32_t) n * 2654435761u; h ^= h >> 15; h *= 2246822519u; h ^= h >> 13;
            teeth[n].detuneRnd = ((h & 0xffff) / 32767.5f) - 1.0f;
            teeth[n].rustRnd = (float) ((h >> 16) & 0xffff) / 65535.0f;
        }
        for (auto& b : bodyModes) b = BodyMode{};
        clickEnv = mechGain = 0.0f; mechPhase = 0.0; tickCounter = 0.0;
        clickBp = Biquad{}; mechBp = Biquad{};
        clickBp.setBandpass (4200.0, 0.9, sr);
        mechBp.setBandpass (1150.0, 1.4, sr);
        clickDecay = std::exp (-6.908f / (0.004f * (float) sr));
        updateBody();
    }

    void setParams (const Params& np)
    {
        const bool bodyChanged = std::abs (np.size - p.size) > 1.0e-4f;
        p = np;
        if (bodyChanged) updateBody();
    }

    // estado del cilindro (para el ruido del mecanismo). speed = 1 nominal
    void setMechanism (bool running, float speed) { mechRunning = running; mechSpeed = speed; }

    void noteOn (int note, float vel)
    {
        if (note < 0 || note >= kNotes) return;
        auto& t = teeth[note];
        t.pendingVel = std::clamp (vel, 0.02f, 1.0f);
        if (! t.active) { t.active = true; activeList[numActive++] = note; }

        if (t.level > 0.003f)
        {
            // la lamina aun vibra: la pua la toca antes de soltarla
            const float ms = 4.0f + 26.0f * (1.0f - p.dampers) * (1.0f - p.dampers);
            t.contact = std::max (1, (int) (ms * 0.001f * (float) sr));
            const float dB = 34.0f - 28.0f * (1.0f - p.dampers); // atenuacion total durante el contacto
            t.contactDamp = std::pow (10.0f, -dB / (20.0f * (float) t.contact));
            t.buzzAmt = (1.0f - p.dampers);
            t.buzzLp = 0.0f;
        }
        else
        {
            t.contact = 0;
            release (t, note);
        }
    }

    void allNotesOff()
    {
        for (int i = 0; i < numActive; ++i) { auto& t = teeth[activeList[i]]; auto dr = t.detuneRnd; auto rr = t.rustRnd; t = Tooth{}; t.detuneRnd = dr; t.rustRnd = rr; }
        numActive = 0;
    }

    float toothLevel (int note) const { return (note >= 0 && note < kNotes) ? teeth[note].level : 0.0f; }

    void render (float* L, float* R, int n)
    {
        const float bodyMix = p.body;
        const float dryGain = 1.0f - 0.96f * bodyMix * bodyMix;
        const float bodyGain = bodyMix * (1.2f + 1.8f * bodyMix);
        const float mechTarget = mechRunning ? 1.0f : 0.0f;
        const float mechSmooth = 1.0f - std::exp (-1.0f / (0.12f * (float) sr));
        const float mechLevel = p.mechanism * p.mechanism * 0.22f * (1.0f + 2.0f * p.age);
        const double fanInc = 2.0 * M_PI * 21.0 * (double) mechSpeed / sr;
        const double tickInc = 8.5 * (double) mechSpeed / sr;

        for (int i = 0; i < n; ++i)
        {
            float l = 0.0f, r = 0.0f;

            for (int a = 0; a < numActive; ++a)
            {
                const int note = activeList[a];
                auto& t = teeth[note];
                float v = 0.0f;
                if (t.contact > 0)
                {
                    const float d = t.contactDamp;
                    for (int k = 0; k < t.nModes; ++k)
                    {
                        const float nx = t.c[k] * t.x[k] - t.s[k] * t.y[k];
                        const float ny = t.s[k] * t.x[k] + t.c[k] * t.y[k];
                        t.x[k] = nx * d; t.y[k] = ny * d;
                        v += t.g[k] * ny;
                    }
                    // la lamina golpea la pua por un solo lado: rectificacion de media onda
                    const float h = std::max (v, 0.0f);
                    t.buzzLp += 0.01f * (h - t.buzzLp);
                    v = (1.0f - t.buzzAmt) * v + t.buzzAmt * 2.2f * (h - t.buzzLp);
                    if (--t.contact == 0) release (t, note);
                }
                else
                {
                    for (int k = 0; k < t.nModes; ++k)
                    {
                        const float nx = t.c[k] * t.x[k] - t.s[k] * t.y[k];
                        const float ny = t.s[k] * t.x[k] + t.c[k] * t.y[k];
                        t.x[k] = nx; t.y[k] = ny;
                        v += t.g[k] * ny;
                    }
                }
                l += v * t.panL; r += v * t.panR;
            }

            // ruido de la pua
            const float white = nextNoise();
            float extra = clickBp.process (white * clickEnv);
            clickEnv *= clickDecay;

            // mecanismo: aspas del regulador + tics de engranaje
            mechGain += mechSmooth * (mechTarget - mechGain);
            if (mechGain > 1.0e-4f && mechLevel > 0.0f)
            {
                mechPhase += fanInc; if (mechPhase > 2.0 * M_PI) mechPhase -= 2.0 * M_PI;
                const float am = 0.62f + 0.38f * (float) std::sin (mechPhase);
                extra += mechBp.process (nextNoise()) * am * mechGain * mechLevel;
                tickCounter += tickInc;
                if (tickCounter >= 1.0) { tickCounter -= 1.0; clickEnv += mechGain * mechLevel * 4.0f; }
            }

            const float mono = 0.5f * (l + r) + extra;
            float bl = 0.0f, br = 0.0f;
            if (bodyMix > 0.0f)
            {
                for (auto& b : bodyModes)
                {
                    const float o = b.bq.process (mono);
                    bl += o * b.gl; br += o * b.gr;
                }
            }
            float outL = (l * dryGain + extra * 0.6f + bl * bodyGain) * p.volume;
            float outR = (r * dryGain + extra * 0.6f + br * bodyGain) * p.volume;
            L[i] = softClip (outL);
            R[i] = softClip (outR);
        }

        // mantenimiento por bloque: niveles para la interfaz y laminas apagadas
        const float blockT = (float) n / (float) sr;
        for (int a = 0; a < numActive;)
        {
            auto& t = teeth[activeList[a]];
            t.level *= std::exp (-6.908f * blockT / t.t60);
            float e = 0.0f;
            for (int k = 0; k < t.nModes; ++k) e += t.x[k] * t.x[k] + t.y[k] * t.y[k];
            if (e < 1.0e-10f && t.contact == 0)
            {
                t.active = false; t.level = 0.0f;
                for (int k = 0; k < kModes; ++k) t.x[k] = t.y[k] = 0.0f;
                activeList[a] = activeList[--numActive];
            }
            else ++a;
        }
    }

private:
    struct Biquad
    {
        float b0 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
        void setBandpass (double f, double q, double sr)
        {
            f = std::min (f, sr * 0.45);
            const double w = 2.0 * M_PI * f / sr, alpha = std::sin (w) / (2.0 * q), a0 = 1.0 + alpha;
            b0 = (float) (alpha / a0); b2 = -b0;
            a1 = (float) (-2.0 * std::cos (w) / a0); a2 = (float) ((1.0 - alpha) / a0);
        }
        inline float process (float x)
        {
            const float y = b0 * x + z1;
            z1 = -a1 * y + z2;
            z2 = b2 * x - a2 * y + 1.0e-20f;
            return y;
        }
    };
    struct BodyMode { Biquad bq; float gl = 0, gr = 0; };

    struct Tooth
    {
        float x[kModes] {}, y[kModes] {}, c[kModes] {}, s[kModes] {}, g[kModes] {};
        int nModes = 0, contact = 0;
        bool active = false;
        float pendingVel = 0, contactDamp = 1, buzzAmt = 0, buzzLp = 0;
        float level = 0, t60 = 1, panL = 0.7f, panR = 0.7f, detuneRnd = 0, rustRnd = 1;
    };

    void release (Tooth& t, int note)
    {
        static constexpr float ratio[kModes] = { 1.0f, 6.2669f, 17.5475f, 34.3861f };
        const float vel = t.pendingVel;
        const float cents = p.age * p.age * 140.0f * t.detuneRnd;             // hasta +-140 cents
        const bool rusty = t.rustRnd < std::max (0.0f, p.age - 0.45f) * 0.5f;   // laminas oxidadas: sordas y cortas
        const float f0 = 440.0f * std::pow (2.0f, ((float) note - 69.0f + p.tune + cents * 0.01f) / 12.0f);

        t.t60 = std::clamp (p.decay * std::pow (440.0f / f0, 0.45f) * (rusty ? 0.1f : 1.0f), 0.02f, 60.0f);
        // exponente de caida espectral: mas brillo y mas fuerza = mas parciales
        const float e = std::max (-0.45f, 2.6f - 3.0f * p.brightness - 0.35f * (vel - 0.7f) + (rusty ? 1.0f : 0.0f));
        const float amp = std::pow (vel, 1.3f) * 0.22f * std::pow (440.0f / f0, 0.12f);

        t.nModes = 0;
        for (int k = 0; k < kModes; ++k)
        {
            const float f = f0 * ratio[k];
            if (f > 0.42f * (float) sr || f > 19000.0f) break;
            const float t60k = t.t60 / std::pow (ratio[k], 0.75f);
            const float rr = std::exp (-6.908f / (t60k * (float) sr));
            const float w = 2.0f * (float) M_PI * f / (float) sr;
            t.c[k] = rr * std::cos (w); t.s[k] = rr * std::sin (w);
            t.g[k] = std::pow (ratio[k], -e);
            t.x[k] += amp;          // desplazamiento inicial; la salida (y) arranca en cero
            t.nModes = k + 1;
        }
        const float pos = std::clamp (((float) note - 36.0f) / 60.0f, 0.0f, 1.0f);
        const float ang = (0.5f + (pos - 0.5f) * p.width) * 0.5f * (float) M_PI;
        t.panL = std::cos (ang) * 1.4142f; t.panR = std::sin (ang) * 1.4142f;
        t.level = std::max (t.level, vel);
        clickEnv += vel * std::pow (p.click, 1.5f) * 0.9f * (rusty ? 2.0f : 1.0f);
    }

    void updateBody()
    {
        static constexpr float f[kBody]  = { 214, 338, 512, 771, 1043, 1418, 1893, 2510, 3320, 4310 };
        static constexpr float q[kBody]  = { 9, 12, 14, 16, 18, 18, 20, 20, 22, 22 };
        static constexpr float g[kBody]  = { 0.55f, 0.8f, 1.0f, 0.9f, 0.85f, 0.7f, 0.6f, 0.5f, 0.38f, 0.28f };
        static constexpr float pn[kBody] = { 0.5f, 0.3f, 0.7f, 0.2f, 0.8f, 0.4f, 0.65f, 0.25f, 0.75f, 0.5f };
        const float scale = std::pow (2.0f, 2.3f - 4.6f * p.size); // de dedal (x4.9) a baul (x0.2); 0.5 = x1
        const float qScale = 0.55f + 0.9f * p.size + 1.6f * p.size * p.size;
        for (int i = 0; i < kBody; ++i)
        {
            bodyModes[i].bq.setBandpass (f[i] * scale, q[i] * qScale, sr);
            bodyModes[i].gl = g[i] * std::cos (pn[i] * 0.5f * (float) M_PI) * 1.4142f;
            bodyModes[i].gr = g[i] * std::sin (pn[i] * 0.5f * (float) M_PI) * 1.4142f;
        }
    }

    inline float nextNoise()
    {
        rng = rng * 1664525u + 1013904223u;
        return (float) (int32_t) rng * (1.0f / 2147483648.0f);
    }
    static inline float softClip (float x)
    {
        const float ax = std::abs (x);
        if (ax < 0.6f) return x;                       // lineal hasta -4.4 dB
        const float o = 0.6f + 0.4f * std::tanh ((ax - 0.6f) / 0.4f);
        return x < 0 ? -o : o;
    }

    double sr = 44100.0;
    Params p;
    std::array<Tooth, kNotes> teeth;
    std::array<int, kNotes> activeList {};
    int numActive = 0;
    std::array<BodyMode, kBody> bodyModes;
    Biquad clickBp, mechBp;
    float clickEnv = 0, clickDecay = 0.99f, mechGain = 0, mechSpeed = 1;
    bool mechRunning = false;
    double mechPhase = 0, tickCounter = 0;
    uint32_t rng = 0x1234567u;
};
} // namespace mbox
