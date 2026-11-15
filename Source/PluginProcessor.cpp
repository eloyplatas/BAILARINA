#include "PluginProcessor.h"
#include "PluginEditor.h"

using APVTS = juce::AudioProcessorValueTreeState;
static juce::String u8 (const char* s) { return juce::String::fromUTF8 (s); }

juce::File BailarinaProcessor::userMelodyFolder()
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile ("Bailarina").getChildFile ("Melodias");
}

// Libreria interna + archivos .mid de la carpeta del usuario
std::vector<mbox::Melody> BailarinaProcessor::loadLibrary()
{
    auto lib = mbox::buildLibrary();
    const auto folder = userMelodyFolder();
    if (! folder.isDirectory()) folder.createDirectory();
    auto files = folder.findChildFiles (juce::File::findFiles, false, "*.mid;*.midi;*.MID");
    files.sort();
    for (const auto& f : files)
    {
        if (lib.size() >= 400) break;
        juce::FileInputStream in (f);
        juce::MidiFile mf;
        if (! in.openedOk() || ! mf.readFrom (in) || mf.getTimeFormat() <= 0) continue;

        double bpm = 120.0;
        juce::MidiMessageSequence tempos;
        mf.findAllTempoEvents (tempos);
        if (tempos.getNumEvents() > 0)
        {
            const double spq = tempos.getEventPointer (0)->message.getTempoSecondsPerQuarterNote();
            if (spq > 0.0) bpm = 60.0 / spq;
        }
        mf.convertTimestampTicksToSeconds();

        mbox::Melody m;
        m.title = f.getFileNameWithoutExtension().replaceCharacter ('_', ' ').toStdString();
        m.composer = "Melod\xc3\xad" "a propia";
        m.bpm = bpm;
        double first = 1.0e9, last = 0.0;
        for (int t = 0; t < mf.getNumTracks(); ++t)
            for (const auto* e : *mf.getTrack (t))
                if (e->message.isNoteOn() && e->message.getChannel() != 10)
                {
                    int note = e->message.getNoteNumber();
                    while (note < 52) note += 12;
                    while (note > 100) note -= 12;
                    const double sec = e->message.getTimeStamp();
                    first = std::min (first, sec); last = std::max (last, sec);
                    m.pins.push_back ({ sec, note, 0.5f + 0.38f * e->message.getFloatVelocity(), 0.0f, 0.0f });
                    if (m.pins.size() >= 20000) break;
                }
        if (m.pins.empty()) continue;
        for (auto& p : m.pins) p.beat = (p.beat - first) * bpm / 60.0;
        m.lengthBeats = std::ceil (((last - first) * bpm / 60.0 + 1.0) / 4.0) * 4.0;
        m.finish (4099u + (uint32_t) lib.size() * 7919u);
        lib.push_back (std::move (m));
    }
    return lib;
}

APVTS::ParameterLayout BailarinaProcessor::createLayout (const std::vector<mbox::Melody>& lib)
{
    using F = juce::AudioParameterFloat;
    using R = juce::NormalisableRange<float>;
    APVTS::ParameterLayout l;
    auto pct = [] (const char* id, const char* name, float def)
    {
        return std::make_unique<F> (juce::ParameterID { id, 1 }, u8 (name), R (0.0f, 1.0f, 0.001f), def,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return juce::String (juce::roundToInt (v * 100.0f)) + " %"; }));
    };
    l.add (pct ("brightness", "Tono", 0.55f));
    R decayRange (0.05f, 30.0f, 0.01f); decayRange.setSkewForCentre (2.5f);
    l.add (std::make_unique<F> (juce::ParameterID { "decay", 1 }, u8 ("Duraci\xc3\xb3n"), decayRange, 2.5f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return juce::String (v, v < 1.0f ? 2 : 1) + " s"; })));
    l.add (pct ("click", "P\xc3\xba""a", 0.35f));
    l.add (pct ("dampers", "Apagadores", 0.6f));
    l.add (pct ("age", "Edad", 0.15f));
    l.add (pct ("body", "Caja", 0.5f));
    l.add (pct ("size", "Tama\xc3\xb1o", 0.5f));
    l.add (pct ("mechanism", "Mecanismo", 0.15f));
    l.add (pct ("width", "Est\xc3\xa9reo", 0.6f));
    l.add (std::make_unique<F> (juce::ParameterID { "tune", 1 }, u8 ("Afinaci\xc3\xb3n"), R (-24.0f, 24.0f, 1.0f), 0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { const int i = juce::roundToInt (v); return (i > 0 ? "+" : "") + juce::String (i) + " st"; })));
    R speedRange (0.1f, 4.0f, 0.01f); speedRange.setSkewForCentre (1.0f);
    l.add (std::make_unique<F> (juce::ParameterID { "speed", 1 }, "Velocidad", speedRange, 1.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return "x" + juce::String (v, 2); })));
    l.add (std::make_unique<F> (juce::ParameterID { "volume", 1 }, "Volumen", R (-36.0f, 6.0f, 0.1f), -3.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return juce::String (v, 1) + " dB"; })));

    juce::StringArray names;
    for (auto& m : lib) names.add (u8 (m.title.c_str()));
    l.add (std::make_unique<juce::AudioParameterInt> (juce::ParameterID { "melody", 1 }, u8 ("Melod\xc3\xad""a"), 0, juce::jmax (1, names.size() - 1), 0,
        juce::AudioParameterIntAttributes().withStringFromValueFunction ([names] (int v, int) { return names[juce::jlimit (0, names.size() - 1, v)]; })));
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "play", 1 }, "Sonar", false));
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "spring", 1 }, "Cuerda real", false));
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "radio", 1 }, "Radio aleatoria", false));
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "sync", 1 }, "Tempo del DAW", false));
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "lume", 1 }, "Lume Mode", false));
    l.add (pct ("lumeDrive", "Lume: distorsi\xc3\xb3n", 0.3f));
    l.add (pct ("lumeRefl", "Lume: reflexiones", 0.3f));
    l.add (pct ("lumeSize", "Lume: sala", 0.4f));
    l.add (pct ("lumeMix", "Lume: dry/wet", 0.5f));
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "lumeRoute", 1 }, "Lume: en paralelo", false,
        juce::AudioParameterBoolAttributes().withStringFromValueFunction ([] (bool v, int) { return v ? "Paralelo" : "Serie"; })));
    return l;
}

BailarinaProcessor::BailarinaProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      library (loadLibrary()),
      apvts (*this, nullptr, "BAILARINA", createLayout (library))
{
    for (auto& t : uiTooth) t.store (0.0f);
    radioBag.reserve (library.size());

    pBright = apvts.getRawParameterValue ("brightness"); pDecay = apvts.getRawParameterValue ("decay");
    pClick = apvts.getRawParameterValue ("click");       pDampers = apvts.getRawParameterValue ("dampers");
    pAge = apvts.getRawParameterValue ("age");           pBody = apvts.getRawParameterValue ("body");
    pSize = apvts.getRawParameterValue ("size");         pMech = apvts.getRawParameterValue ("mechanism");
    pWidth = apvts.getRawParameterValue ("width");       pVolume = apvts.getRawParameterValue ("volume");
    pTune = apvts.getRawParameterValue ("tune");         pSpeed = apvts.getRawParameterValue ("speed");
    pMelody = apvts.getRawParameterValue ("melody");     pPlay = apvts.getRawParameterValue ("play");
    pSpring = apvts.getRawParameterValue ("spring");     pRadio = apvts.getRawParameterValue ("radio");
    pSync = apvts.getRawParameterValue ("sync");
    pLume = apvts.getRawParameterValue ("lume");           pLumeDrive = apvts.getRawParameterValue ("lumeDrive");
    pLumeRefl = apvts.getRawParameterValue ("lumeRefl");   pLumeSize = apvts.getRawParameterValue ("lumeSize");
    pLumeMix = apvts.getRawParameterValue ("lumeMix");     pLumeRoute = apvts.getRawParameterValue ("lumeRoute");
}

void BailarinaProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate;
    engine.prepare (sr);
    lume.prepare (sr);
}

bool BailarinaProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void BailarinaProcessor::setActiveMelody (int index)
{
    activeMelody = juce::jlimit (0, numMelodies() - 1, index);
    cyl.setMelody (&library[(size_t) activeMelody]);
    uiMelody.store (activeMelody);
}

// Radio: baraja todas las piezas y no repite hasta haberlas tocado todas
int BailarinaProcessor::nextRadioMelody()
{
    if (radioBag.empty())
    {
        for (int i = 0; i < numMelodies(); ++i) if (i != activeMelody || numMelodies() == 1) radioBag.push_back (i);
        for (int i = (int) radioBag.size() - 1; i > 0; --i) std::swap (radioBag[(size_t) i], radioBag[(size_t) random.nextInt (i + 1)]);
    }
    const int next = radioBag.back();
    radioBag.pop_back();
    return next;
}

void BailarinaProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    buffer.clear();
    if (buffer.getNumChannels() < 2 || n == 0) return;

    mbox::Params p;
    p.brightness = pBright->load(); p.decay = pDecay->load(); p.click = pClick->load();
    p.dampers = pDampers->load();   p.age = pAge->load();     p.body = pBody->load();
    p.size = pSize->load();         p.mechanism = pMech->load(); p.width = pWidth->load();
    p.volume = juce::Decibels::decibelsToGain (pVolume->load());
    p.tune = pTune->load();
    engine.setParams (p);

    // melodia: el parametro o un clic en la tarjeta cambian de pieza; la radio avanza sola
    const int paramMelody = juce::roundToInt (pMelody->load());
    if (const int req = melodyRequest.exchange (-1); req >= 0) { setActiveMelody (req); radioPause = 0; lastParamMelody = paramMelody; }
    else if (paramMelody != lastParamMelody || activeMelody < 0) { lastParamMelody = paramMelody; setActiveMelody (paramMelody); radioPause = 0; }
    const bool radio = pRadio->load() > 0.5f;

    // cuerda
    if (const int w = windRequests.exchange (0); w > 0) spring.wind (0.22f * (float) w);
    const bool realSpring = pSpring->load() > 0.5f;
    const float springFactor = realSpring ? spring.speedFactor() : 1.0f;

    // tempo: manual (el de la pieza) o el del DAW (una negra de la pieza = una negra del proyecto)
    double bpm = cyl.melody()->bpm;
    bool transportOk = true, hostSync = false;
    if (pSync->load() > 0.5f)
        if (auto* ph = getPlayHead())
            if (auto pos = ph->getPosition())
                if (auto hostBpm = pos->getBpm())
                {
                    hostSync = true;
                    bpm = *hostBpm;
                    transportOk = pos->getIsPlaying();
                    if (transportOk && ! hostWasPlaying) { cyl.rewind(); radioPause = 0; }  // arranca con el transporte
                    hostWasPlaying = transportOk;
                }
    if (! hostSync) hostWasPlaying = false;
    uiHostSync.store (hostSync);

    const bool wantRun = pPlay->load() > 0.5f && springFactor > 0.0f && transportOk;
    const bool running = wantRun && radioPause <= 0;
    const float wearFactor = wantRun ? wear.next (p.age, (float) n / (float) sr) : 1.0f;
    const float speed = pSpeed->load() * springFactor * wearFactor;

    // eventos: MIDI externo + puas del cilindro
    int numEv = 0;
    auto addEv = [this, &numEv] (int offset, int note, float vel)
    {
        if (numEv < (int) events.size()) events[(size_t) numEv++] = { offset, note, vel };
    };
    for (const auto meta : midi)
    {
        const auto m = meta.getMessage();
        if (m.isNoteOn()) addEv (juce::jlimit (0, n - 1, meta.samplePosition), m.getNoteNumber(), m.getFloatVelocity());
        else if (m.isAllNotesOff() || m.isAllSoundOff()) engine.allNotesOff();
    }
    if (running)
    {
        const bool ended = cyl.advance (n, sr, bpm / 60.0 * (double) speed, p.age, radio, addEv);
        if (ended && radio) radioPause = (int) (2.0 * sr);      // dos segundos de silencio entre piezas
        if (realSpring) spring.consume ((float) n / (float) sr);
    }
    else if (wantRun && radioPause > 0)
    {
        radioPause -= n;
        if (radioPause <= 0) setActiveMelody (nextRadioMelody());
    }
    engine.setMechanism (running, speed);
    std::sort (events.begin(), events.begin() + numEv, [] (const Ev& a, const Ev& b) { return a.offset < b.offset; });

    float* L = buffer.getWritePointer (0);
    float* R = buffer.getWritePointer (1);
    int pos = 0;
    for (int e = 0; e < numEv; ++e)
    {
        const int off = events[(size_t) e].offset;
        if (off > pos) { engine.render (L + pos, R + pos, off - pos); pos = off; }
        engine.noteOn (events[(size_t) e].note, events[(size_t) e].vel);
    }
    if (pos < n) engine.render (L + pos, R + pos, n - pos);

    if (pLume->load() > 0.5f)
    {
        mbox::LumeFx::Params lp;
        lp.drive = pLumeDrive->load(); lp.refl = pLumeRefl->load(); lp.size = pLumeSize->load();
        lp.mix = pLumeMix->load();     lp.parallel = pLumeRoute->load() > 0.5f;
        lume.process (L, R, n, lp);
    }
    else lume.reset();
    uiLumeLevel.store (lume.outputLevel());

    uiCylPos.store ((float) cyl.position01());
    uiSpring.store (realSpring ? spring.level : 1.0f);
    uiSpeed.store (running ? speed * (float) (bpm / cyl.melody()->bpm) : 0.0f);
    for (int i = 0; i < 128; ++i) uiTooth[(size_t) i].store (engine.toothLevel (i), std::memory_order_relaxed);
}

void BailarinaProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, destData);
}

void BailarinaProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* BailarinaProcessor::createEditor() { return new BailarinaEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new BailarinaProcessor(); }
