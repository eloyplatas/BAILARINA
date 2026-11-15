// ============================================================================
//  Lume Mode: distorsion (de overdrive suave a destruccion) seguida de una
//  reverb de primeras reflexiones (de un toque de sala a un enjambre metalico).
//  El modulo va en serie o en paralelo con el sonido original, con dry/wet.
//  C++17 puro, sin JUCE.
// ============================================================================
#pragma once
#include <algorithm>
#include <cmath>
#include <vector>

namespace mbox
{
class LumeFx
{
public:
    struct Params
    {
        float drive = 0.3f;     // 0..1  overdrive suave -> distorsion extrema
        float refl = 0.3f;      // 0..1  cantidad de reflexiones (y realimentacion al final del recorrido)
        float size = 0.4f;      // 0..1  tamano de la sala (de 15 ms a 480 ms de dispersion)
        float mix = 0.5f;       // 0..1  dry/wet
        bool parallel = false;  // false: serie (mezcla cruzada)  true: paralelo (el efecto se suma al original)
    };

    void prepare (double sampleRate)
    {
        sr = sampleRate;
        int len = 1; while (len < (int) (sr * 0.75)) len <<= 1;
        mask = len - 1;
        for (auto& d : dl) d.assign ((size_t) len, 0.0f);
        w = 0; sizeSm = 0.4f;
        for (int c = 0; c < 2; ++c) { hp[c] = prev[c] = os1[c] = os2[c] = tone[c] = fbLp[c] = 0.0f; }
        // retardos y ganancias de las reflexiones: irregulares, distintos en cada canal
        unsigned s = 12345u;
        auto rnd = [&s] { s = s * 1664525u + 1013904223u; return (float) (s >> 8) / 16777216.0f; };
        for (int c = 0; c < 2; ++c)
            for (int k = 0; k < kTaps; ++k)
            {
                const float base = std::pow (((float) k + 0.6f + 0.7f * rnd()) / (float) kTaps, 1.5f);
                tapT[c][k] = std::clamp (base, 0.02f, 1.0f);
                tapG[c][k] = (rnd() < 0.5f ? -1.0f : 1.0f) * std::pow (0.84f, (float) k) * (0.7f + 0.6f * rnd());
            }
        level = 0.0f; dirty = false;
    }

    void reset()   // al apagar el modo, para que no quede cola vieja en la linea de retardo
    {
        if (! dirty) return;
        for (auto& d : dl) std::fill (d.begin(), d.end(), 0.0f);
        for (int c = 0; c < 2; ++c) { hp[c] = prev[c] = os1[c] = os2[c] = tone[c] = fbLp[c] = 0.0f; }
        level = 0.0f; dirty = false;
    }

    float outputLevel() const { return level; }

    void process (float* L, float* R, int n, const Params& p)
    {
        dirty = true;
        const float d = std::clamp (p.drive, 0.0f, 1.0f);
        const float gain = 1.5f * std::pow (10.0f, d * 2.15f);            // x1.5 .. x210
        const float bias = 0.22f * d;
        const float biasOut = std::tanh (bias);
        float t = std::clamp ((d - 0.5f) * 2.0f, 0.0f, 1.0f); t = t * t * (3.0f - 2.0f * t);   // mezcla hacia el recorte duro + plegado
        const float comp = (1.0f - 0.72f * std::sqrt (d)) / 1.5f * (1.0f + 0.5f * (1.0f - d));
        const float hpC = 1.0f - std::exp (-6.2832f * 110.0f / (float) sr);
        const float toneC = 1.0f - std::exp (-6.2832f * (9000.0f - 3500.0f * d) / (float) sr);

        const float a = std::clamp (p.refl, 0.0f, 1.0f);
        const float erLevel = a * (0.9f + 1.3f * a);
        const float fb = a * a * 0.86f;
        const int taps = 5 + (int) (a * (float) (kTaps - 5));
        const float fbC = 1.0f - std::exp (-6.2832f * (6500.0f - 3500.0f * a) / (float) sr);
        const float sizeTarget = std::clamp (p.size, 0.0f, 1.0f);
        const float mix = std::clamp (p.mix, 0.0f, 1.0f);
        float peak = 0.0f;
        float* io[2] = { L, R };

        for (int i = 0; i < n; ++i)
        {
            sizeSm += 0.0004f * (sizeTarget - sizeSm);
            const float spread = (0.015f + sizeSm * sizeSm * 0.465f) * (float) sr;   // muestras
            float dist[2], er[2];

            for (int c = 0; c < 2; ++c)
            {
                // --- distorsion, 4x sobremuestreada ---
                const float x = io[c][i];
                hp[c] += hpC * (x - hp[c]);
                const float xh = x - hp[c] * 0.85f;
                float y = 0.0f;
                for (int k = 1; k <= 4; ++k)
                {
                    const float xi = (prev[c] + (xh - prev[c]) * 0.25f * (float) k) * gain;
                    float s = std::tanh (xi + bias) - biasOut;
                    if (t > 0.0f)
                    {
                        const float hard = std::clamp (xi * 1.6f, -1.0f, 1.0f);
                        const float fold = std::sin (std::clamp (xi, -14.0f, 14.0f) * 0.85f);
                        s += t * (0.55f * hard + 0.45f * fold - s);
                    }
                    os1[c] += 0.36f * (s - os1[c]);
                    os2[c] += 0.36f * (os1[c] - os2[c]);
                    y = os2[c];
                }
                prev[c] = xh;
                tone[c] += toneC * (y - tone[c]);
                dist[c] = tone[c] * comp;
            }

            // --- primeras reflexiones ---
            for (int c = 0; c < 2; ++c)
            {
                float sum = 0.0f;
                for (int k = 0; k < taps; ++k) sum += tapG[c][k] * read (c, tapT[c][k] * spread);
                er[c] = sum;
            }
            for (int c = 0; c < 2; ++c)
            {
                const float back = read (1 - c, spread * 0.93f);          // realimentacion cruzada
                fbLp[c] += fbC * (back - fbLp[c]);
                dl[c][(size_t) w] = dist[c] + sat (fbLp[c] * fb);
            }
            w = (w + 1) & mask;

            for (int c = 0; c < 2; ++c)
            {
                const float fx = sat (dist[c] + er[c] * erLevel * 0.42f);
                const float dry = io[c][i];
                const float wet = fx * mix;
                const float out = p.parallel ? dry + wet : dry * (1.0f - mix) + wet;
                io[c][i] = limit (out);
                peak = std::max (peak, std::abs (wet));
            }
        }
        level = std::max (peak, level * std::exp (-(float) n / (0.25f * (float) sr)));
    }

private:
    static constexpr int kTaps = 18;
    inline float read (int c, float delaySamples) const
    {
        const float pos = (float) w - std::max (1.0f, delaySamples);
        const int i0 = (int) std::floor (pos);
        const float f = pos - (float) i0;
        const auto& b = dl[c];
        return b[(size_t) (i0 & mask)] * (1.0f - f) + b[(size_t) ((i0 + 1) & mask)] * f;
    }
    static inline float sat (float x) { return std::tanh (x); }
    static inline float limit (float x)
    {
        const float ax = std::abs (x);
        if (ax < 0.7f) return x;
        const float o = 0.7f + 0.3f * std::tanh ((ax - 0.7f) / 0.3f);
        return x < 0 ? -o : o;
    }

    double sr = 44100.0;
    std::vector<float> dl[2];
    int w = 0, mask = 0;
    float tapT[2][kTaps] {}, tapG[2][kTaps] {};
    float hp[2] {}, prev[2] {}, os1[2] {}, os2[2] {}, tone[2] {}, fbLp[2] {};
    float sizeSm = 0.4f, level = 0.0f;
    bool dirty = false;
};
} // namespace mbox
