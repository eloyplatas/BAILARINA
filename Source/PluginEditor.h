#pragma once
#include "PluginProcessor.h"

// Aspecto de laton para mandos y palancas
class BrassLookAndFeel : public juce::LookAndFeel_V4
{
public:
    BrassLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
};

// --- Lume Mode: panel de instrumento de medida de los anos 50 ---
class BakeliteLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
};

class LumePanel : public juce::Component
{
public:
    explicit LumePanel (BailarinaProcessor&);
    ~LumePanel() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    void tick();                       // lo llama el temporizador del editor
    static constexpr int kHeight = 200;

private:
    void renderBackground();
    BailarinaProcessor& proc;
    BakeliteLookAndFeel lnf;
    juce::Image background;
    static constexpr int kNumLumeKnobs = 4;
    std::array<juce::Slider, kNumLumeKnobs> knobs;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, kNumLumeKnobs> knobAtt;
    juce::ToggleButton routeSwitch;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> routeAtt;
    float needle = 0.0f, needleVel = 0.0f, lamp = 0.0f;
    int frame = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LumePanel)
};

class BailarinaEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit BailarinaEditor (BailarinaProcessor&);
    ~BailarinaEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

    static constexpr int kW = 940, kH = 660;

private:
    void timerCallback() override;
    void renderBackground();
    void drawBallerina (juce::Graphics&, float cx, float feetY, float scale, float angle, float alpha) const;
    void drawTuneCard (juce::Graphics&) const;
    void drawMechanism (juce::Graphics&) const;
    void drawKnobLabels (juce::Graphics&) const;
    int cardRowAt (juce::Point<int>) const;
    float toothX (int note) const;

    BailarinaProcessor& proc;
    BrassLookAndFeel lnf;
    juce::Image background, patina;   // patina: polvo, aranazos y vineta, por encima de todo

    static constexpr int kNumKnobs = 12;
    std::array<juce::Slider, kNumKnobs> knobs;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, kNumKnobs> knobAtt;
    juce::ToggleButton playButton, springButton, radioButton, syncButton, lumeButton;
    LumePanel lumePanel;
    bool lumeShown = false;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> playAtt, springAtt, radioAtt, syncAtt, lumeAtt;

    // animacion
    float spin = 0.0f, spinSpeed = 0.0f, fanAngle = 0.0f, keyAngle = 0.0f, keyTarget = 0.0f;
    float cylPos = 0.0f, springLevel = 1.0f;
    std::array<float, 128> tooth {};
    int frame = 0, hoverRow = -1, cardScroll = 0, shownMelody = -1;
    void scrollCard (int delta);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BailarinaEditor)
};
