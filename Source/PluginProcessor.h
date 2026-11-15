#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "MusicBoxEngine.h"
#include "CylinderPlayer.h"
#include "LumeFx.h"

class BailarinaProcessor : public juce::AudioProcessor
{
public:
    BailarinaProcessor();
    ~BailarinaProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Bailarina"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 30.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Carpeta donde el usuario deja sus .mid: se anaden a la libreria al cargar el plugin.
    static juce::File userMelodyFolder();

    // --- libreria (inmutable tras el constructor) ---
    int numMelodies() const { return (int) library.size(); }
    const mbox::Melody& melody (int i) const { return library[(size_t) juce::jlimit (0, numMelodies() - 1, i)]; }

    // --- comunicacion con la interfaz ---
    std::atomic<float> uiCylPos { 0.0f }, uiSpring { 1.0f }, uiSpeed { 0.0f }, uiLumeLevel { 0.0f };
    std::atomic<int> uiMelody { 0 };
    std::atomic<bool> uiHostSync { false };
    std::array<std::atomic<float>, 128> uiTooth;
    void requestWind() { windRequests.fetch_add (1); }
    void requestMelody (int index) { melodyRequest.store (index); }

private:
    struct Ev { int offset; int note; float vel; };
    std::vector<mbox::Melody> library;   // debe declararse antes que apvts
    static std::vector<mbox::Melody> loadLibrary();
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout (const std::vector<mbox::Melody>&);

public:
    juce::AudioProcessorValueTreeState apvts;

private:
    void setActiveMelody (int index);
    int nextRadioMelody();

    mbox::Engine engine;
    mbox::LumeFx lume;
    mbox::Cylinder cyl;
    mbox::Spring spring;
    mbox::Wear wear;
    int activeMelody = -1, lastParamMelody = -1, radioPause = 0;
    bool hostWasPlaying = false;
    std::vector<int> radioBag;
    juce::Random random;
    std::atomic<int> windRequests { 0 }, melodyRequest { -1 };
    std::array<Ev, 1024> events {};
    double sr = 44100.0;

    std::atomic<float>* pBright, *pDecay, *pClick, *pDampers, *pAge, *pBody, *pSize, *pMech, *pWidth,
                      * pVolume, *pTune, *pSpeed, *pMelody, *pPlay, *pSpring, *pRadio, *pSync,
                      * pLume, *pLumeDrive, *pLumeRefl, *pLumeSize, *pLumeMix, *pLumeRoute;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BailarinaProcessor)
};
