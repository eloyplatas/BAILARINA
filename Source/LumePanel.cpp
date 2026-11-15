// Panel del Lume Mode: instrumento de medida de los anos 50 (esmalte martele,
// mandos de baquelita, vumetro, piloto de joya y rotulo de maquina de escribir).
#include "PluginEditor.h"
#include <BinaryData.h>

namespace
{
using juce::Colour;
using juce::Rectangle;
using juce::Point;

juce::String u8 (const char* s) { return juce::String::fromUTF8 (s); }

const Colour enamel (0xff78826f), enamelDark (0xff4c5548), cream (0xffe9e1c8), inkBlack (0xff12100d);
const char* const lumeIds[]    = { "lumeDrive", "lumeRefl", "lumeSize", "lumeMix" };
const char* const lumeLabels[] = { "DISTORSIÓN", "REFLEXIONES", "SALA", "DRY / WET" };
const float knobX[] = { 408.0f, 510.0f, 612.0f, 714.0f };
const float knobY = 88.0f;
const Point<float> lampC (302.0f, 46.0f), meterPivot (852.0f, 128.0f);
const Rectangle<float> labelPlate (56.0f, 26.0f, 208.0f, 96.0f), meterBox (786.0f, 22.0f, 132.0f, 124.0f);

juce::Font typewriter (float height)
{
    static juce::Typeface::Ptr face = juce::Typeface::createSystemTypefaceFor (BinaryData::SpecialElite_ttf, BinaryData::SpecialElite_ttfSize);
    return juce::Font (juce::FontOptions (face).withHeight (height));
}

void steelScrew (juce::Graphics& g, float cx, float cy, float r, float angle)
{
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.fillEllipse (cx - r - 1.0f, cy - r + 1.0f, 2 * r + 2, 2 * r + 2.5f);
    juce::ColourGradient gr (Colour (0xffe4e6e3), cx - r, cy - r, Colour (0xff5d625f), cx + r, cy + r, false);
    gr.addColour (0.5, Colour (0xffa9aeaa));
    g.setGradientFill (gr);
    g.fillEllipse (cx - r, cy - r, 2 * r, 2 * r);
    g.setColour (Colour (0xff232523));
    g.drawEllipse (cx - r, cy - r, 2 * r, 2 * r, 0.8f);
    g.drawLine (cx - std::cos (angle) * r * 0.85f, cy - std::sin (angle) * r * 0.85f, cx + std::cos (angle) * r * 0.85f, cy + std::sin (angle) * r * 0.85f, 1.8f);
    g.setColour (juce::Colours::white.withAlpha (0.35f));
    g.drawLine (cx - std::cos (angle) * r * 0.8f + 0.6f, cy - std::sin (angle) * r * 0.8f + 0.9f, cx + std::cos (angle) * r * 0.8f + 0.6f, cy + std::sin (angle) * r * 0.8f + 0.9f, 0.6f);
}

// serigrafia (tinta crema sobre el esmalte), algo comida por el uso
void silk (juce::Graphics& g, const juce::String& text, Rectangle<float> r, float h, juce::Justification j = juce::Justification::centred)
{
    g.setFont (typewriter (h));
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.drawText (text, r.translated (0.6f, 0.9f), j, false);
    g.setColour (cream.withAlpha (0.92f));
    g.drawText (text, r, j, false);
}

// Texto de maquina de escribir vieja con exceso de tinta: tipos desalineados, trazo
// engordado, corros de tinta, ojos de las letras cegados y salpicaduras.
void overInked (juce::Graphics& g, const juce::String& text, float x, float baseline, float height, int seed, float amount)
{
    juce::Random r (seed);
    juce::GlyphArrangement ga;
    ga.addLineOfText (typewriter (height), text, x, baseline);
    for (int i = 0; i < ga.getNumGlyphs(); ++i)
    {
        auto& gl = ga.getGlyph (i);
        if (gl.isWhitespace()) continue;
        juce::Path p; gl.createPath (p);
        const auto b0 = p.getBounds();
        // cada tipo golpea un poco torcido y a distinta altura
        p.applyTransform (juce::AffineTransform::rotation ((r.nextFloat() - 0.5f) * 0.07f, b0.getCentreX(), b0.getCentreY())
                              .translated ((r.nextFloat() - 0.5f) * height * 0.035f, (r.nextFloat() - 0.5f) * height * 0.07f));
        const auto b = p.getBounds();
        const float heavy = 0.6f + r.nextFloat() * 0.8f;          // unos tipos cargan mas tinta que otros
        // cerco de tinta corrida en el papel
        g.setColour (inkBlack.withAlpha (0.10f * amount));
        g.strokePath (p, juce::PathStrokeType (height * 0.16f * heavy, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (inkBlack.withAlpha (0.28f * amount));
        g.strokePath (p, juce::PathStrokeType (height * 0.09f * heavy, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (inkBlack.withAlpha (0.9f));
        g.strokePath (p, juce::PathStrokeType (height * 0.04f * heavy, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (inkBlack);
        g.fillPath (p);
        // corros: la tinta se acumula en los trazos
        const int blobs = (int) (10.0f * amount * heavy);
        for (int k = 0; k < blobs * 6; ++k)
        {
            const Point<float> pt (b.getX() + r.nextFloat() * b.getWidth(), b.getY() + r.nextFloat() * b.getHeight());
            if (! p.contains (pt)) continue;
            const float d = height * (0.05f + r.nextFloat() * 0.11f) * heavy;
            g.setColour (inkBlack.withAlpha (0.75f + r.nextFloat() * 0.25f));
            g.fillEllipse (pt.x - d * 0.5f, pt.y - d * 0.5f, d, d * (0.7f + r.nextFloat() * 0.5f));
        }
        // ojos cegados (e, o, d, a...): el tipo sucio rellena el hueco
        const auto ch = gl.getCharacter();
        if (juce::String ("eoadMLu").containsChar (ch) && r.nextFloat() < 0.75f * amount)
        {
            const bool upper = ch == 'e' || ch == 'M' || ch == 'L';
            const float cx = b.getCentreX() + (r.nextFloat() - 0.5f) * b.getWidth() * 0.12f;
            const float cy = b.getY() + b.getHeight() * (ch == 'e' ? 0.33f : (upper ? 0.45f : 0.55f));
            const float w = b.getWidth() * (ch == 'e' ? 0.34f : 0.3f), h = b.getHeight() * (ch == 'e' ? 0.16f : 0.3f);
            if (ch != 'M' && ch != 'L' && ch != 'u')
            {
                g.setColour (inkBlack.withAlpha (0.82f));
                g.fillEllipse (cx - w * 0.5f, cy - h * 0.5f, w, h);
            }
        }
        // ribete seco: motas sin tinta
        for (int k = 0; k < 5; ++k)
        {
            const Point<float> pt (b.getX() + r.nextFloat() * b.getWidth(), b.getY() + r.nextFloat() * b.getHeight());
            if (p.contains (pt)) { g.setColour (cream.withAlpha (0.35f)); g.fillEllipse (pt.x, pt.y, height * 0.025f, height * 0.025f); }
        }
        // salpicaduras alrededor
        for (int k = 0; k < (int) (4.0f * amount); ++k)
        {
            const float d = height * (0.012f + r.nextFloat() * r.nextFloat() * 0.07f);
            g.setColour (inkBlack.withAlpha (0.35f + r.nextFloat() * 0.5f));
            g.fillEllipse (b.getX() - height * 0.15f + r.nextFloat() * (b.getWidth() + height * 0.3f), b.getY() - height * 0.15f + r.nextFloat() * (b.getHeight() + height * 0.3f), d, d);
        }
    }
}
} // namespace

// ============================================================================
void BakeliteLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                            float startAngle, float endAngle, juce::Slider& s)
{
    const float cx = (float) x + (float) w * 0.5f, cy = (float) y + (float) h * 0.5f;
    const float R = (float) juce::jmin (w, h) * 0.5f - 9.0f;
    const float ang = startAngle + pos * (endAngle - startAngle);

    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.fillEllipse (cx - R - 2.0f, cy - R + 3.0f, 2 * R + 4, 2 * R + 5);
    // faldon acanalado
    juce::ColourGradient sk (Colour (0xff3b3835), cx - R * 0.6f, cy - R * 0.8f, Colour (0xff050505), cx + R, cy + R, true);
    g.setGradientFill (sk);
    g.fillEllipse (cx - R, cy - R, 2 * R, 2 * R);
    for (int i = 0; i < 28; ++i)
    {
        const float a = ang + (float) i * juce::MathConstants<float>::twoPi / 28.0f;
        g.setColour (juce::Colours::white.withAlpha (0.07f));
        g.drawLine (cx + std::sin (a) * R * 0.78f, cy - std::cos (a) * R * 0.78f, cx + std::sin (a) * R * 0.98f, cy - std::cos (a) * R * 0.98f, 1.6f);
    }
    g.setColour (Colour (0xff000000)); g.drawEllipse (cx - R, cy - R, 2 * R, 2 * R, 1.0f);
    // cabeza de "pico de pollo"
    juce::Path head;
    head.startNewSubPath (-R * 0.36f, R * 0.55f);
    head.quadraticTo (-R * 0.46f, 0.0f, -R * 0.16f, -R * 0.9f);
    head.quadraticTo (0.0f, -R * 1.22f, R * 0.16f, -R * 0.9f);
    head.quadraticTo (R * 0.46f, 0.0f, R * 0.36f, R * 0.55f);
    head.quadraticTo (0.0f, R * 0.8f, -R * 0.36f, R * 0.55f);
    const auto tr = juce::AffineTransform::rotation (ang).translated (cx, cy);
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillPath (head, tr.translated (1.5f, 2.5f));
    g.setGradientFill (juce::ColourGradient (Colour (0xff4a4744), cx - R * 0.4f, cy - R * 0.5f, Colour (0xff0a0909), cx + R * 0.5f, cy + R * 0.6f, false));
    g.fillPath (head, tr);
    g.setColour (juce::Colours::black); g.strokePath (head, juce::PathStrokeType (0.8f), tr);
    juce::Path shine; shine.startNewSubPath (-R * 0.2f, R * 0.3f); shine.quadraticTo (-R * 0.3f, -R * 0.2f, -R * 0.1f, -R * 0.8f);
    g.setColour (juce::Colours::white.withAlpha (0.22f)); g.strokePath (shine, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded), tr);
    juce::Path idx; idx.addRoundedRectangle (-1.2f, -R * 1.08f, 2.4f, R * 0.62f, 1.0f);
    g.setColour (s.isMouseOverOrDragging() ? juce::Colours::white : cream); g.fillPath (idx, tr);
    g.setColour (Colour (0xff8f948f)); g.fillEllipse (cx - 2.2f, cy + R * 0.32f - 2.2f, 4.4f, 4.4f);   // prisionero
}

void BakeliteLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool highlighted, bool)
{
    // conmutador de palanca: arriba = serie, abajo = paralelo
    const auto r = b.getLocalBounds().toFloat();
    const float cx = r.getCentreX(), cy = r.getCentreY();
    const bool down = b.getToggleState();
    juce::Path nut; nut.addPolygon ({ cx, cy }, 6, 11.0f, 0.5f);
    g.setColour (juce::Colours::black.withAlpha (0.5f)); g.fillPath (nut, juce::AffineTransform::translation (1.0f, 2.0f));
    g.setGradientFill (juce::ColourGradient (Colour (0xffe6e8e5), cx - 9, cy - 9, Colour (0xff60655f), cx + 10, cy + 10, false));
    g.fillPath (nut);
    g.setColour (Colour (0xff222422)); g.strokePath (nut, juce::PathStrokeType (0.8f));
    g.setColour (Colour (0xff101010)); g.fillEllipse (cx - 5.5f, cy - 5.5f, 11.0f, 11.0f);
    const float dir = down ? 1.0f : -1.0f, tipY = cy + dir * 19.0f;
    juce::Path bat;
    bat.startNewSubPath (cx - 2.6f, cy); bat.lineTo (cx - 4.6f, tipY); bat.quadraticTo (cx, tipY + dir * 6.0f, cx + 4.6f, tipY); bat.lineTo (cx + 2.6f, cy); bat.closeSubPath();
    g.setColour (juce::Colours::black.withAlpha (0.45f)); g.fillPath (bat, juce::AffineTransform::translation (2.0f, 2.5f));
    juce::ColourGradient cg (Colour (0xff7e837d), cx - 5, 0, Colour (0xff4f544f), cx + 5, 0, false);
    cg.addColour (0.35, highlighted ? Colour (0xffffffff) : Colour (0xfff1f3f0));
    g.setGradientFill (cg);
    g.fillPath (bat);
    g.setColour (Colour (0xff1c1e1c)); g.strokePath (bat, juce::PathStrokeType (0.7f));
}

// ============================================================================
LumePanel::LumePanel (BailarinaProcessor& p) : proc (p)
{
    setLookAndFeel (&lnf);
    for (int i = 0; i < kNumLumeKnobs; ++i)
    {
        auto& k = knobs[(size_t) i];
        k.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        k.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        k.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
        addAndMakeVisible (k);
        knobAtt[(size_t) i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, lumeIds[i], k);
    }
    routeSwitch.setClickingTogglesState (true);
    addAndMakeVisible (routeSwitch);
    routeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "lumeRoute", routeSwitch);
    renderBackground();
}

LumePanel::~LumePanel() { setLookAndFeel (nullptr); }

void LumePanel::resized()
{
    for (int i = 0; i < kNumLumeKnobs; ++i) knobs[(size_t) i].setBounds ((int) knobX[i] - 40, (int) knobY - 40, 80, 80);
    routeSwitch.setBounds ((int) lampC.x - 20, 96, 40, 60);
}

void LumePanel::tick()
{
    ++frame;
    // balistica de vumetro: aguja con masa y algo de rebote
    const float lv = proc.uiLumeLevel.load();
    const float target = juce::jlimit (0.0f, 1.08f, std::pow (lv * 1.5f, 0.6f));
    needleVel += (target - needle) * 0.22f; needleVel *= 0.62f; needle += needleVel;
    lamp += 0.25f * ((0.75f + 0.25f * juce::jmin (1.0f, lv * 2.5f)) - lamp);
    repaint();
}

void LumePanel::renderBackground()
{
    const int W = BailarinaEditor::kW, H = kHeight;
    background = juce::Image (juce::Image::ARGB, W, H, true);
    juce::Graphics g (background);
    const Rectangle<float> all (0, 0, (float) W, (float) H);
    juce::Random rnd (1952);

    // chasis: esmalte martele gris verdoso
    g.setGradientFill (juce::ColourGradient (enamel.brighter (0.08f), 0, 0, enamelDark, 0, (float) H, false));
    g.fillRect (all);
    for (int i = 0; i < 16000; ++i)
    {
        const float d = 1.0f + rnd.nextFloat() * 3.4f, x = rnd.nextFloat() * (float) W, y = rnd.nextFloat() * (float) H;
        g.setColour ((rnd.nextBool() ? juce::Colours::white : juce::Colours::black).withAlpha (0.025f + rnd.nextFloat() * 0.05f));
        g.fillEllipse (x, y, d, d * 0.8f);
        if (i % 9 == 0) { g.setColour (juce::Colours::white.withAlpha (0.05f)); g.fillEllipse (x + 0.4f, y + 0.3f, d * 0.4f, d * 0.3f); }
    }
    // bisel del panel y junta con la caja
    g.setColour (juce::Colours::black.withAlpha (0.85f)); g.fillRect (0.0f, 0.0f, (float) W, 4.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.5f), 0, 4, juce::Colours::transparentBlack, 0, 18, false));
    g.fillRect (0.0f, 4.0f, (float) W, 14.0f);
    g.setColour (juce::Colours::white.withAlpha (0.22f)); g.drawRect (all.reduced (5.0f).withTrimmedTop (2.0f), 1.0f);
    g.setColour (juce::Colours::black.withAlpha (0.6f)); g.drawRect (all.reduced (6.0f).withTrimmedTop (2.0f), 1.0f);
    g.drawRect (all, 2.0f);

    // asas cromadas
    for (float hx : { 22.0f, (float) W - 36.0f })
    {
        const Rectangle<float> hd (hx, 42.0f, 14.0f, 116.0f);
        g.setColour (juce::Colours::black.withAlpha (0.5f)); g.fillRoundedRectangle (hd.translated (3.0f, 4.0f), 7.0f);
        juce::ColourGradient cg (Colour (0xff6b706c), hd.getX(), 0, Colour (0xff3c403d), hd.getRight(), 0, false);
        cg.addColour (0.3, Colour (0xfff4f6f3)); cg.addColour (0.55, Colour (0xff9da29e));
        g.setGradientFill (cg); g.fillRoundedRectangle (hd, 7.0f);
        g.setColour (Colour (0xff1b1d1b)); g.drawRoundedRectangle (hd, 7.0f, 0.9f);
        for (float fy : { hd.getY() + 2.0f, hd.getBottom() - 12.0f })
        {
            g.setColour (Colour (0xff2a2d2a)); g.fillRoundedRectangle (hd.getX() - 3.0f, fy, 20.0f, 10.0f, 2.0f);
            g.setColour (juce::Colours::white.withAlpha (0.25f)); g.drawRoundedRectangle (hd.getX() - 3.0f, fy, 20.0f, 10.0f, 2.0f, 0.6f);
        }
    }
    for (auto pt : { Point<float> (16, 20), Point<float> ((float) W - 16, 20), Point<float> (16, (float) H - 15), Point<float> ((float) W - 16, (float) H - 15) })
        steelScrew (g, pt.x, pt.y, 6.0f, pt.x * 0.37f + pt.y);

    // --- rotulo: chapa esmaltada en crema, mecanografiada con demasiada tinta ---
    {
        g.setColour (juce::Colours::black.withAlpha (0.5f)); g.fillRoundedRectangle (labelPlate.translated (2.0f, 3.0f), 3.0f);
        g.setGradientFill (juce::ColourGradient (cream.brighter (0.05f), labelPlate.getX(), labelPlate.getY(), Colour (0xffcfc39f), labelPlate.getRight(), labelPlate.getBottom(), false));
        g.fillRoundedRectangle (labelPlate, 3.0f);
        {
            juce::Graphics::ScopedSaveState ss (g);
            g.reduceClipRegion (labelPlate.toNearestInt());
            for (int i = 0; i < 260; ++i)   // amarilleo y oxido que asoma por los bordes
            {
                float fx = rnd.nextFloat(), fy = rnd.nextFloat();
                if (rnd.nextBool()) fx = fx < 0.5f ? fx * fx * fx : 1.0f - std::pow (1.0f - fx, 3.0f); else fy = fy < 0.5f ? fy * fy * fy : 1.0f - std::pow (1.0f - fy, 3.0f);
                const float d = 2.0f + rnd.nextFloat() * 16.0f;
                g.setColour ((rnd.nextFloat() < 0.8f ? Colour (0xff7a5a1c) : Colour (0xff5a2a0c)).withAlpha (0.03f + rnd.nextFloat() * 0.07f));
                g.fillEllipse (labelPlate.getX() + fx * labelPlate.getWidth() - d * 0.5f, labelPlate.getY() + fy * labelPlate.getHeight() - d * 0.5f, d, d);
            }
        }
        g.setColour (Colour (0xff2b2a24)); g.drawRoundedRectangle (labelPlate, 3.0f, 1.0f);
        g.setColour (inkBlack.withAlpha (0.55f)); g.drawRect (labelPlate.reduced (5.0f), 0.7f);
        for (auto pt : { labelPlate.getTopLeft() + Point<float> (9, 9), labelPlate.getTopRight() + Point<float> (-9, 9), labelPlate.getBottomLeft() + Point<float> (9, -9), labelPlate.getBottomRight() + Point<float> (-9, -9) })
        {
            g.setColour (juce::Colours::black.withAlpha (0.4f)); g.fillEllipse (pt.x - 2.6f, pt.y - 1.8f, 6.0f, 6.0f);
            g.setGradientFill (juce::ColourGradient (Colour (0xffd9dbd6), pt.x - 2, pt.y - 2, Colour (0xff5f635e), pt.x + 3, pt.y + 3, true));
            g.fillEllipse (pt.x - 2.8f, pt.y - 2.8f, 5.6f, 5.6f);
        }
        overInked (g, "Lume Mode", labelPlate.getX() + 20.0f, labelPlate.getY() + 50.0f, 31.5f, 7, 1.0f);
        overInked (g, u8 ("aparato n.º 52 — serie B"), labelPlate.getX() + 21.0f, labelPlate.getY() + 76.0f, 11.5f, 21, 0.45f);
        // subrayado a maquina, con la cinta gastada
        for (float x = labelPlate.getX() + 20.0f; x < labelPlate.getRight() - 22.0f; x += 6.4f)
        {
            g.setColour (inkBlack.withAlpha (0.45f + rnd.nextFloat() * 0.5f));
            g.fillRect (x, labelPlate.getY() + 58.5f + (rnd.nextFloat() - 0.5f) * 0.8f, 5.6f, 1.5f + rnd.nextFloat() * 0.8f);
        }
    }
    silk (g, u8 ("DISTORSIÓN · PRIMERAS REFLEXIONES"), Rectangle<float> (labelPlate.getX(), 132.0f, labelPlate.getWidth(), 12.0f), 10.0f);
    silk (g, u8 ("220 V ~ 50 c/s     FUS. 1 A"), Rectangle<float> (labelPlate.getX(), 150.0f, labelPlate.getWidth(), 12.0f), 9.5f);

    // --- piloto y conmutador ---
    {
        const float r = 15.0f;
        g.setColour (juce::Colours::black.withAlpha (0.5f)); g.fillEllipse (lampC.x - r, lampC.y - r + 2.5f, 2 * r + 1, 2 * r + 1);
        juce::ColourGradient cg (Colour (0xfff1f3f0), lampC.x - r, lampC.y - r, Colour (0xff4f544f), lampC.x + r, lampC.y + r, false);
        cg.addColour (0.5, Colour (0xff9ca19c));
        g.setGradientFill (cg); g.fillEllipse (lampC.x - r, lampC.y - r, 2 * r, 2 * r);
        g.setColour (Colour (0xff1a1c1a)); g.drawEllipse (lampC.x - r, lampC.y - r, 2 * r, 2 * r, 0.9f);
        g.setColour (Colour (0xff0c0c0c)); g.fillEllipse (lampC.x - 10.0f, lampC.y - 10.0f, 20.0f, 20.0f);
        silk (g, "LUME", Rectangle<float> (lampC.x - 30.0f, lampC.y + 17.0f, 60.0f, 11.0f), 10.0f);
        silk (g, "SERIE", Rectangle<float> (lampC.x - 40.0f, 86.0f, 80.0f, 11.0f), 10.0f);
        silk (g, "PARALELO", Rectangle<float> (lampC.x - 40.0f, 157.0f, 80.0f, 11.0f), 10.0f);
    }

    // --- mandos: escalas serigrafiadas ---
    g.setColour (cream.withAlpha (0.55f));
    g.drawRoundedRectangle (348.0f, 16.0f, 424.0f, 166.0f, 6.0f, 1.0f);
    for (int i = 0; i < kNumLumeKnobs; ++i)
    {
        const float cx = knobX[i];
        for (int k = 0; k <= 50; ++k)
        {
            const float a = juce::MathConstants<float>::pi * (-0.75f + 1.5f * (float) k / 50.0f);
            const bool major = k % 5 == 0;
            const float r0 = 37.0f, r1 = major ? 44.0f : 41.0f;
            g.setColour (cream.withAlpha (major ? 0.95f : 0.7f));
            g.drawLine (cx + std::sin (a) * r0, knobY - std::cos (a) * r0, cx + std::sin (a) * r1, knobY - std::cos (a) * r1, major ? 1.4f : 0.8f);
        }
        if (i == 0)   // zona roja al final de la distorsion
        {
            juce::Path arc; arc.addCentredArc (cx, knobY, 46.5f, 46.5f, 0.0f, juce::MathConstants<float>::pi * 0.36f, juce::MathConstants<float>::pi * 0.75f, true);
            g.setColour (Colour (0xffc23a2b)); g.strokePath (arc, juce::PathStrokeType (2.4f));
        }
        g.setFont (typewriter (9.5f));
        for (int k = 0; k <= 10; ++k)
        {
            const float a = juce::MathConstants<float>::pi * (-0.75f + 1.5f * (float) k / 10.0f);
            g.setColour (cream.withAlpha (0.9f));
            g.drawText (juce::String (k), Rectangle<float> (cx + std::sin (a) * 51.0f - 9.0f, knobY - std::cos (a) * 51.0f - 5.5f, 18.0f, 11.0f), juce::Justification::centred, false);
        }
        silk (g, u8 (lumeLabels[i]), Rectangle<float> (cx - 50.0f, 160.0f, 100.0f, 14.0f), 11.5f);
    }

    // --- vumetro ---
    {
        g.setColour (juce::Colours::black.withAlpha (0.55f)); g.fillRoundedRectangle (meterBox.translated (2.0f, 4.0f), 8.0f);
        g.setGradientFill (juce::ColourGradient (Colour (0xff34312e), meterBox.getX(), meterBox.getY(), Colour (0xff070707), meterBox.getRight(), meterBox.getBottom(), false));
        g.fillRoundedRectangle (meterBox, 8.0f);
        g.setColour (juce::Colours::white.withAlpha (0.14f)); g.drawRoundedRectangle (meterBox.reduced (1.0f), 7.0f, 1.0f);
        const auto face = meterBox.reduced (9.0f, 9.0f).withTrimmedBottom (20.0f);
        g.setGradientFill (juce::ColourGradient (Colour (0xfff2e7c4), face.getX(), face.getY(), Colour (0xffd2c08c), face.getRight(), face.getBottom(), false));
        g.fillRoundedRectangle (face, 3.0f);
        {
            juce::Graphics::ScopedSaveState ss (g);
            g.reduceClipRegion (face.toNearestInt());
            for (int i = 0; i < 90; ++i)
            {
                const float d = 2.0f + rnd.nextFloat() * 12.0f;
                g.setColour (Colour (0xff7a5a1c).withAlpha (0.03f + rnd.nextFloat() * 0.06f));
                g.fillEllipse (face.getX() + rnd.nextFloat() * face.getWidth(), face.getY() + rnd.nextFloat() * face.getHeight(), d, d);
            }
            const float R = 78.0f, a0 = -0.82f, a1 = 0.82f;
            juce::Path arc; arc.addCentredArc (meterPivot.x, meterPivot.y, R, R, 0.0f, a0, a0 + (a1 - a0) * 0.72f, true);
            g.setColour (inkBlack); g.strokePath (arc, juce::PathStrokeType (1.2f));
            juce::Path red; red.addCentredArc (meterPivot.x, meterPivot.y, R, R, 0.0f, a0 + (a1 - a0) * 0.72f, a1, true);
            g.setColour (Colour (0xffb5281c)); g.strokePath (red, juce::PathStrokeType (3.0f));
            static const char* const marks[] = { "20", "10", "7", "5", "3", "0", "+3" };
            static const float markPos[] = { 0.0f, 0.2f, 0.33f, 0.46f, 0.59f, 0.72f, 1.0f };
            g.setFont (typewriter (8.5f));
            for (int k = 0; k < 7; ++k)
            {
                const float a = a0 + (a1 - a0) * markPos[k];
                g.setColour (k >= 5 ? Colour (0xffb5281c) : inkBlack);
                g.drawLine (meterPivot.x + std::sin (a) * (R - 5.0f), meterPivot.y - std::cos (a) * (R - 5.0f), meterPivot.x + std::sin (a) * (R + 4.0f), meterPivot.y - std::cos (a) * (R + 4.0f), 1.1f);
                g.drawText (marks[k], Rectangle<float> (meterPivot.x + std::sin (a) * (R + 11.0f) - 10.0f, meterPivot.y - std::cos (a) * (R + 11.0f) - 5.0f, 20.0f, 10.0f), juce::Justification::centred, false);
            }
            g.setColour (inkBlack);
            g.setFont (typewriter (15.0f));
            g.drawText ("VU", Rectangle<float> (meterPivot.x - 20.0f, face.getBottom() - 30.0f, 40.0f, 14.0f), juce::Justification::centred, false);
        }
        g.setColour (Colour (0xff0b0b0b)); g.drawRoundedRectangle (face, 3.0f, 1.2f);
        silk (g, "SALIDA LUME", Rectangle<float> (meterBox.getX(), 153.0f, meterBox.getWidth(), 12.0f), 10.0f);
        steelScrew (g, meterBox.getCentreX(), meterBox.getBottom() - 13.0f, 4.0f, 0.4f);
    }

    // --- desgaste: desconchones, oxido, aranazos, mugre ---
    for (int i = 0; i < 40; ++i)
    {
        const bool edge = rnd.nextFloat() < 0.75f;
        float x = rnd.nextFloat() * (float) W, y = rnd.nextFloat() * (float) H;
        if (edge) { if (rnd.nextBool()) x = rnd.nextBool() ? rnd.nextFloat() * 14.0f : (float) W - rnd.nextFloat() * 14.0f; else y = rnd.nextBool() ? 4.0f + rnd.nextFloat() * 10.0f : (float) H - rnd.nextFloat() * 12.0f; }
        juce::Path chip; const int pts = 5 + rnd.nextInt (4); const float rr = 1.5f + rnd.nextFloat() * rnd.nextFloat() * 7.0f;
        for (int k = 0; k < pts; ++k)
        {
            const float a = (float) k * 6.2832f / (float) pts, r2 = rr * (0.6f + rnd.nextFloat() * 0.7f);
            if (k == 0) chip.startNewSubPath (x + std::cos (a) * r2, y + std::sin (a) * r2); else chip.lineTo (x + std::cos (a) * r2, y + std::sin (a) * r2);
        }
        chip.closeSubPath();
        g.setColour (Colour (0xff2c2e2b)); g.fillPath (chip);
        g.setColour (Colour (0xff7a3a16).withAlpha (0.55f)); g.fillPath (chip, juce::AffineTransform::scale (0.6f, 0.6f, x, y));
        g.setColour (juce::Colours::white.withAlpha (0.2f)); g.strokePath (chip, juce::PathStrokeType (0.5f));
    }
    for (int i = 0; i < 110; ++i)
    {
        const float x = rnd.nextFloat() * (float) W, y = rnd.nextFloat() * (float) H, a = rnd.nextFloat() * 6.283f, len = 6.0f + rnd.nextFloat() * rnd.nextFloat() * 110.0f;
        g.setColour ((rnd.nextFloat() < 0.6f ? juce::Colours::white : juce::Colours::black).withAlpha (0.05f + rnd.nextFloat() * 0.1f));
        g.drawLine (x, y, x + std::cos (a) * len, y + std::sin (a) * len, 0.4f + rnd.nextFloat() * 0.5f);
    }
    for (int i = 0; i < 12; ++i)
    {
        const float x = rnd.nextFloat() * (float) W, y = rnd.nextFloat() * (float) H, w = 50.0f + rnd.nextFloat() * 160.0f;
        g.setGradientFill (juce::ColourGradient (Colour (0xff15120c).withAlpha (0.2f), x, y, juce::Colours::transparentBlack, x + w * 0.5f, y, true));
        g.fillEllipse (x - w * 0.5f, y - w * 0.3f, w, w * 0.6f);
    }
    juce::ColourGradient vg (juce::Colours::transparentBlack, (float) W * 0.5f, (float) H * 0.5f, juce::Colours::black.withAlpha (0.45f), 0.0f, 0.0f, true);
    vg.addColour (0.6, juce::Colours::transparentBlack);
    g.setGradientFill (vg); g.fillRect (all);
}

void LumePanel::paint (juce::Graphics& g)
{
    g.drawImageAt (background, 0, 0);

    // piloto de joya roja, encendido mientras el modo esta activo
    {
        const float glow = lamp * (0.94f + 0.06f * std::sin ((float) frame * 0.9f));
        g.setGradientFill (juce::ColourGradient (Colour (0xffff5a2a).withAlpha (0.38f * glow), lampC.x, lampC.y, juce::Colours::transparentBlack, lampC.x + 34.0f, lampC.y, true));
        g.fillEllipse (lampC.x - 34.0f, lampC.y - 34.0f, 68.0f, 68.0f);
        juce::ColourGradient jw (Colour (0xffffd9a0).interpolatedWith (Colour (0xff8a1a10), 1.0f - glow), lampC.x - 2.0f, lampC.y - 2.0f, Colour (0xff5a0c08).interpolatedWith (Colour (0xffd0301c), glow * 0.8f), lampC.x + 9.0f, lampC.y + 9.0f, true);
        g.setGradientFill (jw);
        g.fillEllipse (lampC.x - 9.0f, lampC.y - 9.0f, 18.0f, 18.0f);
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        for (int i = 0; i < 6; ++i) { const float a = (float) i * 1.0472f; g.drawLine (lampC.x, lampC.y, lampC.x + std::cos (a) * 9.0f, lampC.y + std::sin (a) * 9.0f, 0.6f); }
        g.setColour (juce::Colours::white.withAlpha (0.55f)); g.fillEllipse (lampC.x - 5.0f, lampC.y - 6.0f, 4.0f, 3.0f);
    }

    // aguja del vumetro
    {
        const auto face = meterBox.reduced (9.0f, 9.0f).withTrimmedBottom (20.0f);
        juce::Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (face.toNearestInt());
        const float a = -0.82f + 1.64f * juce::jlimit (-0.03f, 1.05f, needle * 0.95f);
        const Point<float> tip (meterPivot.x + std::sin (a) * 84.0f, meterPivot.y - std::cos (a) * 84.0f);
        g.setColour (juce::Colours::black.withAlpha (0.25f)); g.drawLine (meterPivot.x + 2.0f, meterPivot.y + 3.0f, tip.x + 2.0f, tip.y + 3.0f, 1.6f);
        g.setColour (Colour (0xff141210)); g.drawLine (meterPivot.x, meterPivot.y, tip.x, tip.y, 1.3f);
        g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.22f), face.getX(), face.getY(), juce::Colours::transparentWhite, face.getX() + 60.0f, face.getY() + 50.0f, false));
        g.fillRect (face);   // reflejo del cristal
    }
}
