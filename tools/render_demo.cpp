// Render offline del motor (sin JUCE):
//   render_demo <melodia> <segundos> <salida.wav> [edad 0..1] [tamano 0..1] [duracion s] [lume: drive refl sala mix paralelo(0/1)]
//   render_demo list
#include "../Source/MusicBoxEngine.h"
#include "../Source/CylinderPlayer.h"
#include "../Source/LumeFx.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
int main (int argc, char** argv)
{
    auto lib = mbox::buildLibrary();
    if (argc > 1 && ! strcmp (argv[1], "list"))
    {
        for (size_t i = 0; i < lib.size(); ++i)
            printf ("%2zu  %-36s %-26s %6.1f s  %4zu puas\n", i, lib[i].title.c_str(), lib[i].composer.c_str(), lib[i].lengthBeats * 60.0 / lib[i].bpm, lib[i].pins.size());
        return 0;
    }
    const int melody = argc > 1 ? atoi (argv[1]) : 0;
    const double secs = argc > 2 ? atof (argv[2]) : 20.0;
    const char* out = argc > 3 ? argv[3] : "out.wav";
    const double sr = 48000.0; const int bs = 128;
    mbox::Engine eng; eng.prepare (sr);
    mbox::Params p;
    if (argc > 4) p.age = (float) atof (argv[4]);
    if (argc > 5) p.size = (float) atof (argv[5]);
    if (argc > 6) p.decay = (float) atof (argv[6]);
    eng.setParams (p);
    mbox::Cylinder cyl; mbox::Wear wear;
    mbox::LumeFx lume; lume.prepare (sr); mbox::LumeFx::Params lp; const bool useLume = argc > 11;
    if (useLume) { lp.drive = (float) atof (argv[7]); lp.refl = (float) atof (argv[8]); lp.size = (float) atof (argv[9]); lp.mix = (float) atof (argv[10]); lp.parallel = atoi (argv[11]) != 0; }
    cyl.setMelody (&lib[(size_t) std::clamp (melody, 0, (int) lib.size() - 1)]);
    const int total = (int) (secs * sr);
    std::vector<float> L ((size_t) total + bs), R ((size_t) total + bs);
    float peak = 0; int plucks = 0; float minW = 9, maxW = 0;
    for (int n = 0; n < total; n += bs)
    {
        const bool run = n < total - (int) (3 * sr);
        const float w = wear.next (p.age, (float) (bs / sr));
        minW = std::min (minW, w); maxW = std::max (maxW, w);
        eng.setMechanism (run, w);
        if (run) cyl.advance (bs, sr, cyl.melody()->bpm / 60.0 * w, p.age, false, [&] (int, int note, float v) { eng.noteOn (note, v); ++plucks; });
        eng.render (L.data() + n, R.data() + n, bs);
        if (useLume) lume.process (L.data() + n, R.data() + n, bs, lp);
    }
    for (int i = 0; i < total; ++i) { peak = std::max (peak, std::max (std::abs (L[i]), std::abs (R[i]))); if (L[i] != L[i]) { puts ("NaN!"); return 1; } }
    FILE* f = fopen (out, "wb");
    const uint32_t dataBytes = (uint32_t) total * 4, riff = 36 + dataBytes, fmtLen = 16, rate = (uint32_t) sr, byteRate = rate * 4;
    const uint16_t pcm = 1, ch = 2, align = 4, bits = 16;
    fwrite ("RIFF", 1, 4, f); fwrite (&riff, 4, 1, f); fwrite ("WAVEfmt ", 1, 8, f); fwrite (&fmtLen, 4, 1, f);
    fwrite (&pcm, 2, 1, f); fwrite (&ch, 2, 1, f); fwrite (&rate, 4, 1, f); fwrite (&byteRate, 4, 1, f); fwrite (&align, 2, 1, f); fwrite (&bits, 2, 1, f);
    fwrite ("data", 1, 4, f); fwrite (&dataBytes, 4, 1, f);
    for (int i = 0; i < total; ++i) { int16_t s[2] = { (int16_t) (L[i] * 32767.0f), (int16_t) (R[i] * 32767.0f) }; fwrite (s, 2, 2, f); }
    fclose (f);
    printf ("%s  \"%s\"  disparos=%d  pico=%.3f  velocidad %.2f..%.2f\n", out, cyl.melody()->title.c_str(), plucks, peak, minW, maxW);
    return 0;
}
