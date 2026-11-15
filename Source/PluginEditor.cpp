#include "PluginEditor.h"

namespace
{
using juce::Colour;
using juce::Rectangle;
using juce::Point;

juce::String u8 (const char* s) { return juce::String::fromUTF8 (s); }

// --- paleta ---
const Colour woodDark (0xff2a1309), wood (0xff5b2c14), woodLight (0xff8c4c24);
const Colour brass (0xffc8a14b), brassLight (0xfff4dfa0), brassDark (0xff6b4f1e);
const Colour velvet (0xff741428), velvetDark (0xff2e050d);
const Colour ivory (0xfff0e4cb), ink (0xff3b2a1c), inkRed (0xff8c1f24);
const Colour skin (0xfff5e4d6), tights (0xfff7dcd6), rose (0xffe7a3b4), hair (0xff4a2c1a);

// --- geometria (ventana 940 x 660) ---
const Rectangle<float> lidArea  (28, 28, 884, 304);
const Rectangle<float> mechArea (28, 346, 884, 142);
const Rectangle<float> knobArea (28, 502, 884, 130);
const Rectangle<float> cardArea (52, 48, 250, 264);
const Rectangle<float> plateArea (650, 52, 236, 92);
const Point<float> mirrorC (470, 164);
const float mirrorRx = 112.0f, mirrorRy = 114.0f;
const float feetY = 297.0f;
const float cylX0 = 170.0f, cylX1 = 750.0f, cylCy = 438.0f, cylR = 30.0f;
const int toothLo = 48, toothHi = 96;
const Point<float> keyC (832, 426), fanC (98, 398), gearC (98, 446);
const float cardRow0 = 105.0f, cardRowH = 24.0f;
const int kCardRows = 8;

juce::Font serif (float size, int style = juce::Font::plain)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultSerifFontName(), size, style));
}

void fillWood (juce::Graphics& g, Rectangle<float> r, Colour top, Colour bottom, int seed, float grainAlpha)
{
    juce::Graphics::ScopedSaveState ss (g);
    g.reduceClipRegion (r.toNearestInt());
    g.setGradientFill (juce::ColourGradient (top, r.getX(), r.getY(), bottom, r.getX(), r.getBottom(), false));
    g.fillRect (r);
    juce::Random rnd (seed);
    const int lines = (int) (r.getHeight() * 0.5f);
    for (int i = 0; i < lines; ++i)
    {
        const float y0 = r.getY() + rnd.nextFloat() * r.getHeight();
        const float amp = 0.6f + rnd.nextFloat() * 2.6f, freq = 0.004f + rnd.nextFloat() * 0.012f, ph = rnd.nextFloat() * 6.28f;
        const float drift = (rnd.nextFloat() - 0.5f) * 6.0f;
        juce::Path p;
        p.startNewSubPath (r.getX(), y0);
        for (float x = r.getX(); x <= r.getRight() + 16.0f; x += 16.0f)
            p.lineTo (x, y0 + std::sin (x * freq + ph) * amp + drift * (x - r.getX()) / r.getWidth());
        const bool light = rnd.nextFloat() > 0.8f;
        g.setColour ((light ? woodLight.brighter (0.4f) : Colour (0xff120603)).withAlpha (grainAlpha * (0.3f + rnd.nextFloat() * 0.7f)));
        g.strokePath (p, juce::PathStrokeType (0.4f + rnd.nextFloat() * 0.8f));
    }
}

void insetShadow (juce::Graphics& g, Rectangle<float> r, float depth, float alpha)
{
    const Colour c = juce::Colours::black.withAlpha (alpha), t = juce::Colours::transparentBlack;
    g.setGradientFill (juce::ColourGradient (c, r.getX(), r.getY(), t, r.getX(), r.getY() + depth, false));
    g.fillRect (r.withHeight (depth));
    g.setGradientFill (juce::ColourGradient (c, r.getX(), r.getY(), t, r.getX() + depth, r.getY(), false));
    g.fillRect (r.withWidth (depth));
    g.setGradientFill (juce::ColourGradient (t, r.getX(), r.getBottom() - depth * 0.6f, c.withMultipliedAlpha (0.5f), r.getX(), r.getBottom(), false));
    g.fillRect (r.withTop (r.getBottom() - depth * 0.6f));
    g.setGradientFill (juce::ColourGradient (t, r.getRight() - depth * 0.6f, r.getY(), c.withMultipliedAlpha (0.5f), r.getRight(), r.getY(), false));
    g.fillRect (r.withLeft (r.getRight() - depth * 0.6f));
}

void brassFill (juce::Graphics& g, Rectangle<float> r, float corner)
{
    juce::ColourGradient gr (brassLight, r.getX(), r.getY(), brassDark, r.getRight(), r.getBottom(), false);
    gr.addColour (0.35, brass); gr.addColour (0.6, brassLight.darker (0.15f)); gr.addColour (0.8, brass.darker (0.2f));
    g.setGradientFill (gr);
    g.fillRoundedRectangle (r, corner);
    g.setColour (brassDark.darker (0.5f).withAlpha (0.8f));
    g.drawRoundedRectangle (r, corner, 1.0f);
    g.setColour (brassLight.withAlpha (0.5f));
    g.drawRoundedRectangle (r.reduced (1.5f), corner, 0.6f);
}

void screw (juce::Graphics& g, float cx, float cy, float r, float slotAngle)
{
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.fillEllipse (cx - r - 0.5f, cy - r + 0.5f, 2 * r + 1, 2 * r + 1);
    g.setGradientFill (juce::ColourGradient (brassLight, cx - r, cy - r, brassDark, cx + r, cy + r, false));
    g.fillEllipse (cx - r, cy - r, 2 * r, 2 * r);
    g.setColour (brassDark.darker (0.8f));
    g.drawLine (cx - std::cos (slotAngle) * r * 0.8f, cy - std::sin (slotAngle) * r * 0.8f,
                cx + std::cos (slotAngle) * r * 0.8f, cy + std::sin (slotAngle) * r * 0.8f, 1.1f);
}

void engraved (juce::Graphics& g, const juce::String& text, Rectangle<float> r, juce::Font f, Colour c, juce::Justification j = juce::Justification::centred)
{
    g.setFont (f);
    g.setColour (juce::Colours::white.withAlpha (0.35f));
    g.drawText (text, r.translated (0.0f, 1.0f), j, false);
    g.setColour (c);
    g.drawText (text, r, j, false);
}

// manchas de oxido, deslustre y cardenillo sobre una pieza de laton
void tarnish (juce::Graphics& g, const juce::Path& area, int seed, float amount)
{
    juce::Graphics::ScopedSaveState ss (g);
    g.reduceClipRegion (area);
    const auto b = area.getBounds();
    juce::Random r (seed);
    const int n = (int) (b.getWidth() * b.getHeight() / 150.0f * amount);
    for (int i = 0; i < n; ++i)
    {
        float fx = r.nextFloat(), fy = r.nextFloat();
        if (r.nextFloat() < 0.55f) { if (r.nextBool()) fx = fx < 0.5f ? fx * fx : 1.0f - (1.0f - fx) * (1.0f - fx); else fy = fy < 0.5f ? fy * fy : 1.0f - (1.0f - fy) * (1.0f - fy); }
        const float x = b.getX() + fx * b.getWidth(), y = b.getY() + fy * b.getHeight();
        const float k = r.nextFloat();
        if (k < 0.68f)      { const float d = 2.0f + r.nextFloat() * r.nextFloat() * 16.0f; g.setColour (Colour (0xff24160a).withAlpha (0.03f + r.nextFloat() * 0.08f)); g.fillEllipse (x - d * 0.5f, y - d * 0.35f, d, d * 0.7f); }
        else if (k < 0.9f)  { const float d = 1.5f + r.nextFloat() * 7.0f;  g.setColour (Colour (0xff6e2f10).withAlpha (0.04f + r.nextFloat() * 0.08f)); g.fillEllipse (x - d * 0.5f, y - d * 0.5f, d, d); }
        else                { const float d = 0.8f + r.nextFloat() * 2.6f;  g.setColour (Colour (0xff3f8a72).withAlpha (0.12f + r.nextFloat() * 0.22f)); g.fillEllipse (x - d * 0.5f, y - d * 0.5f, d, d); }
    }
}
void tarnishRect (juce::Graphics& g, Rectangle<float> r, float corner, int seed, float amount)
{
    juce::Path p; p.addRoundedRectangle (r, corner); tarnish (g, p, seed, amount);
}

// trazo de laton con relieve (para volutas y filetes)
void brassStroke (juce::Graphics& g, const juce::Path& p, float w)
{
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.strokePath (p, juce::PathStrokeType (w + 1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded), juce::AffineTransform::translation (0.6f, 1.2f));
    g.setColour (brassDark);
    g.strokePath (p, juce::PathStrokeType (w + 0.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (brass);
    g.strokePath (p, juce::PathStrokeType (w, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (brassLight.withAlpha (0.75f));
    g.strokePath (p, juce::PathStrokeType (w * 0.3f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded), juce::AffineTransform::translation (-0.4f, -0.5f));
}

// voluta en S con hojas; dir = +1 se abre a la derecha, -1 a la izquierda
void scrollOrnament (juce::Graphics& g, float cx, float cy, float s, float dir)
{
    for (float v : { -1.0f, 1.0f })
    {
        juce::Path p;
        p.startNewSubPath (cx, cy + v * s * 0.08f);
        p.cubicTo (cx + dir * s * 0.55f, cy + v * s * 0.05f, cx + dir * s * 0.75f, cy + v * s * 0.55f, cx + dir * s * 0.38f, cy + v * s * 0.92f);
        p.cubicTo (cx + dir * s * 0.12f, cy + v * s * 1.14f, cx - dir * s * 0.06f, cy + v * s * 0.82f, cx + dir * s * 0.16f, cy + v * s * 0.7f);
        p.cubicTo (cx + dir * s * 0.3f, cy + v * s * 0.64f, cx + dir * s * 0.36f, cy + v * s * 0.8f, cx + dir * s * 0.26f, cy + v * s * 0.84f);
        brassStroke (g, p, 2.6f);
        for (int i = 0; i < 3; ++i)   // hojas
        {
            const float t = 0.25f + 0.2f * (float) i;
            const float lx = cx + dir * s * (0.5f + 0.12f * (float) i), ly = cy + v * s * t;
            juce::Path leaf; leaf.addEllipse (-5.5f, -2.0f, 11.0f, 4.0f);
            const auto tr = juce::AffineTransform::rotation (dir * v * (0.5f + 0.35f * (float) i)).translated (lx, ly);
            g.setColour (brassDark.darker (0.4f)); g.fillPath (leaf, tr.translated (0.5f, 1.0f));
            g.setGradientFill (juce::ColourGradient (brassLight, lx - 4, ly - 2, brassDark, lx + 4, ly + 3, false));
            g.fillPath (leaf, tr);
        }
    }
    g.setGradientFill (juce::ColourGradient (brassLight, cx - 3, cy - 3, brassDark, cx + 4, cy + 4, true));
    g.fillEllipse (cx - 4.0f, cy - 4.0f, 8.0f, 8.0f);
    g.setColour (brassDark.darker (0.6f)); g.drawEllipse (cx - 4.0f, cy - 4.0f, 8.0f, 8.0f, 0.7f);
}

// cantonera de laton calado para una esquina; (sx, sy) = orientacion
void cornerPiece (juce::Graphics& g, float x, float y, float sx, float sy, int seed)
{
    const float L = 58.0f;
    juce::Path p;
    p.startNewSubPath (0, 0); p.lineTo (L, 0);
    p.quadraticTo (L * 0.80f, L * 0.05f, L * 0.74f, L * 0.22f); p.quadraticTo (L * 0.52f, L * 0.16f, L * 0.44f, L * 0.30f);
    p.quadraticTo (L * 0.40f, L * 0.40f, L * 0.30f, L * 0.44f); p.quadraticTo (L * 0.16f, L * 0.52f, L * 0.22f, L * 0.74f);
    p.quadraticTo (L * 0.05f, L * 0.80f, 0, L); p.closeSubPath();
    const auto tr = juce::AffineTransform::scale (sx, sy).translated (x, y);
    juce::Path q (p); q.applyTransform (tr);
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillPath (q, juce::AffineTransform::translation (1.5f, 2.5f));
    const auto b = q.getBounds();
    juce::ColourGradient gr (brassLight, b.getX(), b.getY(), brassDark, b.getRight(), b.getBottom(), false);
    gr.addColour (0.4, brass); gr.addColour (0.65, brassLight.darker (0.2f));
    g.setGradientFill (gr);
    g.fillPath (q);
    g.setColour (brassDark.darker (0.7f)); g.strokePath (q, juce::PathStrokeType (1.0f));
    // grabado
    juce::Path e;
    e.addCentredArc (L * 0.2f, L * 0.2f, L * 0.13f, L * 0.13f, 0.0f, 0.0f, 6.2832f, true);
    e.startNewSubPath (L * 0.36f, L * 0.06f); e.quadraticTo (L * 0.55f, L * 0.02f, L * 0.68f, L * 0.1f);
    e.startNewSubPath (L * 0.06f, L * 0.36f); e.quadraticTo (L * 0.02f, L * 0.55f, L * 0.1f, L * 0.68f);
    e.startNewSubPath (L * 0.32f, L * 0.2f); e.quadraticTo (L * 0.34f, L * 0.34f, L * 0.2f, L * 0.32f);
    g.setColour (brassDark.darker (0.8f).withAlpha (0.75f)); g.strokePath (e, juce::PathStrokeType (0.9f), tr);
    g.setColour (brassLight.withAlpha (0.4f)); g.strokePath (e, juce::PathStrokeType (0.5f), tr.translated (0.5f, 0.7f));
    tarnish (g, q, seed, 1.6f);
    const auto c = Point<float> (L * 0.2f, L * 0.2f).transformedBy (tr);
    screw (g, c.x, c.y, 3.4f, (float) seed);
}

const char* const knobIds[]    = { "brightness", "decay", "click", "dampers", "age", "body", "size", "mechanism", "width", "tune", "speed", "volume" };
const char* const knobLabels[] = { "TONO", "DURACIÓN", "PÚA", "APAGADORES", "EDAD", "CAJA", "TAMAÑO", "MECANISMO", "ESTÉREO", "AFINACIÓN", "VELOCIDAD", "VOLUMEN" };
} // namespace

// ============================================================================
BrassLookAndFeel::BrassLookAndFeel()
{
    setColour (juce::BubbleComponent::backgroundColourId, ivory);
    setColour (juce::BubbleComponent::outlineColourId, ink);
    setColour (juce::TooltipWindow::textColourId, ink);
}

void BrassLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                         float startAngle, float endAngle, juce::Slider& s)
{
    const float cx = (float) x + (float) w * 0.5f, cy = (float) y + (float) h * 0.5f;
    const float R = (float) juce::jmin (w, h) * 0.5f - 7.0f;
    const float ang = startAngle + pos * (endAngle - startAngle);

    // sombra
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillEllipse (cx - R - 1.0f, cy - R + 2.5f, 2 * R + 2, 2 * R + 2);
    // corona moleteada
    g.setGradientFill (juce::ColourGradient (brass, cx - R, cy - R, brassDark.darker (0.4f), cx + R, cy + R, false));
    g.fillEllipse (cx - R, cy - R, 2 * R, 2 * R);
    g.setColour (brassDark.darker (0.9f).withAlpha (0.55f));
    for (int i = 0; i < 36; ++i)
    {
        const float a = ang + (float) i * juce::MathConstants<float>::twoPi / 36.0f;
        g.drawLine (cx + std::sin (a) * R * 0.84f, cy - std::cos (a) * R * 0.84f, cx + std::sin (a) * R, cy - std::cos (a) * R, 1.0f);
    }
    // tapa
    const float r2 = R * 0.8f;
    juce::ColourGradient cap (brassLight, cx - r2 * 0.5f, cy - r2 * 0.6f, brassDark, cx + r2 * 0.8f, cy + r2, true);
    cap.addColour (0.45, brass);
    g.setGradientFill (cap);
    g.fillEllipse (cx - r2, cy - r2, 2 * r2, 2 * r2);
    g.setColour (brassDark.darker (0.6f).withAlpha (0.7f));
    g.drawEllipse (cx - r2, cy - r2, 2 * r2, 2 * r2, 0.8f);
    // indicador
    {   // deslustre del uso y aro grabado
        juce::Path capP; capP.addEllipse (cx - r2, cy - r2, 2 * r2, 2 * r2);
        tarnish (g, capP, 900 + s.getX(), 2.4f);
        g.setColour (brassDark.darker (0.7f).withAlpha (0.45f));
        g.drawEllipse (cx - r2 * 0.62f, cy - r2 * 0.62f, r2 * 1.24f, r2 * 1.24f, 0.7f);
        g.setColour (brassLight.withAlpha (0.25f));
        g.drawEllipse (cx - r2 * 0.62f + 0.4f, cy - r2 * 0.62f + 0.7f, r2 * 1.24f, r2 * 1.24f, 0.5f);
    }
    juce::Path ptr;   // indice incrustado de marfil
    ptr.addRoundedRectangle (-1.7f, -r2 * 0.95f, 3.4f, r2 * 0.5f, 1.5f);
    const auto ptrT = juce::AffineTransform::rotation (ang).translated (cx, cy);
    g.setColour (Colour (0xff1a1008));
    g.strokePath (ptr, juce::PathStrokeType (1.2f), ptrT);
    g.setColour (s.isMouseOverOrDragging() ? Colour (0xffffffff) : ivory.darker (0.08f));
    g.fillPath (ptr, ptrT);
}

void BrassLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool highlighted, bool)
{
    auto r = b.getLocalBounds().toFloat();
    const bool on = b.getToggleState();
    auto labelArea = r.removeFromTop (16.0f);
    engraved (g, b.getButtonText(), labelArea, serif (11.5f, juce::Font::bold), brassLight.withAlpha (0.9f));
    // ranura
    auto slot = r.reduced (39.0f, 7.5f);
    {
        const auto plate = slot.expanded (5.0f, 4.0f);
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.fillRoundedRectangle (plate.translated (1.0f, 2.0f), plate.getHeight() * 0.5f);
        brassFill (g, plate, plate.getHeight() * 0.5f);
        tarnishRect (g, plate, plate.getHeight() * 0.5f, 700 + b.getX() + b.getY(), 2.0f);
        screw (g, plate.getX() + 5.0f, plate.getCentreY(), 1.9f, 0.7f);
        screw (g, plate.getRight() - 5.0f, plate.getCentreY(), 1.9f, 2.1f);
    }
    g.setColour (Colour (0xff0c0503));
    g.fillRoundedRectangle (slot, slot.getHeight() * 0.5f);
    g.setColour (brassDark.darker (0.8f));
    g.drawRoundedRectangle (slot, slot.getHeight() * 0.5f, 1.0f);
    // rotulos
    const auto names = juce::StringArray::fromTokens (b.getName(), "|", "");
    g.setFont (serif (10.0f, juce::Font::italic));
    g.setColour (ivory.withAlpha (on ? 0.35f : 0.9f));
    g.drawText (names[0], r.withRight (slot.getX() - 8.0f), juce::Justification::centredRight, false);
    g.setColour (ivory.withAlpha (on ? 0.95f : 0.35f));
    g.drawText (names[1], r.withLeft (slot.getRight() + 8.0f), juce::Justification::centredLeft, false);
    // palanca
    const float d = slot.getHeight() + 1.0f;
    const float kx = on ? slot.getRight() - d * 0.5f - 1.0f : slot.getX() + d * 0.5f + 1.0f, ky = slot.getCentreY();
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillEllipse (kx - d * 0.5f, ky - d * 0.5f + 2.0f, d, d);
    juce::ColourGradient cap (brassLight.brighter (highlighted ? 0.3f : 0.0f), kx - d * 0.25f, ky - d * 0.3f, brassDark, kx + d * 0.5f, ky + d * 0.5f, true);
    cap.addColour (0.5, brass);
    g.setGradientFill (cap);
    g.fillEllipse (kx - d * 0.5f, ky - d * 0.5f, d, d);
    g.setColour (brassDark.darker (0.6f));
    g.drawEllipse (kx - d * 0.5f, ky - d * 0.5f, d, d, 0.8f);
}

// ============================================================================
BailarinaEditor::BailarinaEditor (BailarinaProcessor& p) : AudioProcessorEditor (&p), proc (p), lumePanel (p)
{
    setLookAndFeel (&lnf);
    for (int i = 0; i < kNumKnobs; ++i)
    {
        auto& k = knobs[(size_t) i];
        k.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        k.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        k.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true);
        addAndMakeVisible (k);
        knobAtt[(size_t) i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, knobIds[i], k);
    }
    playButton.setButtonText ("CILINDRO");            playButton.setName ("parar|sonar");
    springButton.setButtonText ("CUERDA REAL");       springButton.setName ("no|" + u8 ("sí"));
    radioButton.setButtonText ("RADIO");              radioButton.setName ("no|" + u8 ("sí"));
    syncButton.setButtonText ("TEMPO");               syncButton.setName ("manual|DAW");
    lumeButton.setButtonText ("LUME MODE");           lumeButton.setName ("no|" + u8 ("sí"));
    for (auto* b : { &playButton, &springButton, &radioButton, &syncButton, &lumeButton }) { b->setClickingTogglesState (true); addAndMakeVisible (*b); }
    lumeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "lume", lumeButton);
    addChildComponent (lumePanel);
    lumeShown = proc.apvts.getRawParameterValue ("lume")->load() > 0.5f;
    lumePanel.setVisible (lumeShown);
    radioAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "radio", radioButton);
    syncAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "sync", syncButton);
    playAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "play", playButton);
    springAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "spring", springButton);

    renderBackground();
    setSize (kW, kH + (lumeShown ? LumePanel::kHeight : 0));
    setResizable (false, false);
    startTimerHz (30);
}

BailarinaEditor::~BailarinaEditor() { stopTimer(); setLookAndFeel (nullptr); }

void BailarinaEditor::resized()
{
    const float step = knobArea.getWidth() / (float) kNumKnobs;
    for (int i = 0; i < kNumKnobs; ++i)
        knobs[(size_t) i].setBounds ((int) (knobArea.getX() + step * (float) i + (step - 64.0f) * 0.5f), (int) knobArea.getY() + 14, 64, 64);
    playButton.setBounds (648, 160, 120, 50);
    radioButton.setBounds (768, 160, 120, 50);
    springButton.setBounds (648, 226, 120, 50);
    syncButton.setBounds (768, 226, 120, 50);
    lumeButton.setBounds (708, 281, 120, 49);
    lumePanel.setBounds (0, kH, kW, LumePanel::kHeight);
}

float BailarinaEditor::toothX (int note) const
{
    const int n = juce::jlimit (toothLo, toothHi, note) - toothLo;
    return cylX0 + ((float) n + 0.5f) * (cylX1 - cylX0) / (float) (toothHi - toothLo + 1);
}

// ---------------------------------------------------------------------------
void BailarinaEditor::renderBackground()
{
    background = juce::Image (juce::Image::ARGB, kW, kH, true);
    juce::Graphics g (background);
    const Rectangle<float> all (0, 0, (float) kW, (float) kH);

    // caja de caoba
    fillWood (g, all, wood.brighter (0.05f), woodDark.brighter (0.2f), 11, 0.17f);
    {   // veteado: aguas de la madera, nudos y laca cuarteada
        juce::Random rnd (41);
        for (int i = 0; i < 46; ++i)
        {
            const float x = rnd.nextFloat() * (float) kW, y = rnd.nextFloat() * (float) kH, w = 60.0f + rnd.nextFloat() * 220.0f, h = 8.0f + rnd.nextFloat() * 26.0f;
            const bool dark = rnd.nextFloat() < 0.6f;
            g.setGradientFill (juce::ColourGradient ((dark ? Colour (0xff0d0502) : woodLight).withAlpha (dark ? 0.22f : 0.10f), x, y, juce::Colours::transparentBlack, x + w * 0.5f, y + h * 0.5f, true));
            g.fillEllipse (x - w * 0.5f, y - h * 0.5f, w, h);
        }
        for (int i = 0; i < 70; ++i)    // grietas finas de la laca
        {
            float x = rnd.nextFloat() * (float) kW, y = rnd.nextFloat() * (float) kH;
            juce::Path c; c.startNewSubPath (x, y);
            const int segs = 3 + rnd.nextInt (5);
            for (int k = 0; k < segs; ++k) { x += 4.0f + rnd.nextFloat() * 16.0f; y += (rnd.nextFloat() - 0.5f) * 7.0f; c.lineTo (x, y); }
            g.setColour (Colour (0xff050201).withAlpha (0.18f + rnd.nextFloat() * 0.22f));
            g.strokePath (c, juce::PathStrokeType (0.5f));
        }
    }
    {   // marqueteria: filetes de boj y cenefa de rombos de boj y ebano
        g.setColour (brass.withAlpha (0.7f));
        g.drawRoundedRectangle (all.reduced (5.5f), 3.0f, 1.3f);
        g.setColour (Colour (0xffd3ae6c).withAlpha (0.55f));
        g.drawRect (all.reduced (9.5f), 0.8f);
        g.drawRect (all.reduced (20.5f), 0.8f);
        g.setColour (Colour (0xff0e0603).withAlpha (0.6f));
        g.fillRect (all.reduced (10.3f).withHeight (10.2f)); g.fillRect (all.reduced (10.3f).withTop ((float) kH - 20.5f));
        g.fillRect (all.reduced (10.3f).withWidth (10.2f));  g.fillRect (all.reduced (10.3f).withLeft ((float) kW - 20.5f));
        auto diamond = [&g] (float cx, float cy, bool horizontal, int i)
        {
            const float a = horizontal ? 5.2f : 3.2f, b = horizontal ? 3.2f : 5.2f;
            juce::Path d; d.addQuadrilateral (cx - a, cy, cx, cy - b, cx + a, cy, cx, cy + b);
            g.setColour (i % 2 == 0 ? Colour (0xffdcb878) : Colour (0xff9a5a26));
            g.fillPath (d);
            g.setColour (Colour (0xff120703).withAlpha (0.7f));
            g.strokePath (d, juce::PathStrokeType (0.5f));
        };
        const int nx = 76, ny = 52;
        for (int i = 0; i < nx; ++i) { const float x = 22.0f + ((float) kW - 44.0f) * ((float) i + 0.5f) / (float) nx; diamond (x, 15.4f, true, i); diamond (x, (float) kH - 15.4f, true, i + 1); }
        for (int i = 0; i < ny; ++i) { const float y = 22.0f + ((float) kH - 44.0f) * ((float) i + 0.5f) / (float) ny; diamond (15.4f, y, false, i); diamond ((float) kW - 15.4f, y, false, i + 1); }
        g.setColour (brassLight.withAlpha (0.3f));
        g.drawRect (all.reduced (24.5f), 0.6f);
        // perlitas de laton entre los paneles
        for (float by : { 339.0f, 495.0f })
            for (float x = 40.0f; x < (float) kW - 36.0f; x += 9.0f)
            {
                g.setColour (juce::Colours::black.withAlpha (0.5f)); g.fillEllipse (x - 1.4f, by - 0.8f, 3.4f, 3.4f);
                g.setGradientFill (juce::ColourGradient (brassLight, x - 1, by - 1.5f, brassDark, x + 1.5f, by + 1.5f, true));
                g.fillEllipse (x - 1.5f, by - 1.5f, 3.0f, 3.0f);
            }
    }
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.drawRect (all, 2.0f);

    // --- interior de la tapa: terciopelo ---
    {
        juce::Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (lidArea.toNearestInt());
        g.setGradientFill (juce::ColourGradient (velvet.brighter (0.12f), mirrorC.x, mirrorC.y + 20.0f, velvetDark, lidArea.getX(), lidArea.getY(), true));
        g.fillRect (lidArea);
        // capitone
        const float sp = 52.0f;
        for (float d = -lidArea.getHeight(); d < lidArea.getWidth(); d += sp)
        {
            g.setColour (juce::Colours::black.withAlpha (0.16f));
            g.drawLine (lidArea.getX() + d, lidArea.getY(), lidArea.getX() + d + lidArea.getHeight(), lidArea.getBottom(), 1.4f);
            g.drawLine (lidArea.getX() + d + lidArea.getHeight(), lidArea.getY(), lidArea.getX() + d, lidArea.getBottom(), 1.4f);
            g.setColour (juce::Colours::white.withAlpha (0.035f));
            g.drawLine (lidArea.getX() + d + 1.5f, lidArea.getY(), lidArea.getX() + d + lidArea.getHeight() + 1.5f, lidArea.getBottom(), 1.0f);
            g.drawLine (lidArea.getX() + d + lidArea.getHeight() + 1.5f, lidArea.getY(), lidArea.getX() + d + 1.5f, lidArea.getBottom(), 1.0f);
        }
        juce::Random rnd (5);
        for (int i = 0; i < 2600; ++i) // pelusa del terciopelo
        {
            g.setColour ((rnd.nextBool() ? juce::Colours::white : juce::Colours::black).withAlpha (0.025f + rnd.nextFloat() * 0.03f));
            g.fillRect (lidArea.getX() + rnd.nextFloat() * lidArea.getWidth(), lidArea.getY() + rnd.nextFloat() * lidArea.getHeight(), 1.0f, 1.6f);
        }
        {   // desgaste: calvas, manchas y botones del capitone
            juce::Random r2 (63);
            for (int i = 0; i < 26; ++i)
            {
                const float x = lidArea.getX() + r2.nextFloat() * lidArea.getWidth(), y = lidArea.getY() + r2.nextFloat() * lidArea.getHeight();
                const float w = 30.0f + r2.nextFloat() * 120.0f, h = 14.0f + r2.nextFloat() * 50.0f;
                const bool faded = r2.nextFloat() < 0.55f;
                g.setGradientFill (juce::ColourGradient ((faded ? Colour (0xffb98a7c) : Colour (0xff12030a)).withAlpha (faded ? 0.13f : 0.3f), x, y, juce::Colours::transparentBlack, x + w * 0.5f, y + h * 0.5f, true));
                g.fillEllipse (x - w * 0.5f, y - h * 0.5f, w, h);
            }
            for (float by = lidArea.getY(); by < lidArea.getBottom() + 2.0f; by += sp * 0.5f)
            {
                const int rowI = juce::roundToInt ((by - lidArea.getY()) / (sp * 0.5f));
                for (float bx = lidArea.getX() + ((rowI + 2) % 2 == 0 ? 0.0f : sp * 0.5f) + std::fmod (lidArea.getHeight(), sp); bx < lidArea.getRight(); bx += sp)
                {
                    g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.5f), bx, by, juce::Colours::transparentBlack, bx + 9.0f, by + 9.0f, true));
                    g.fillEllipse (bx - 9.0f, by - 9.0f, 18.0f, 18.0f);
                    g.setGradientFill (juce::ColourGradient (velvet.brighter (0.5f), bx - 1.2f, by - 1.5f, velvetDark, bx + 3.0f, by + 3.0f, true));
                    g.fillEllipse (bx - 2.8f, by - 2.8f, 5.6f, 5.6f);
                }
            }
        }
        insetShadow (g, lidArea, 30.0f, 0.85f);
    }
    g.setColour (brassDark.withAlpha (0.9f));
    g.drawRect (lidArea.expanded (1.0f), 1.5f);

    // --- espejo oval ---
    {
        const Rectangle<float> m (mirrorC.x - mirrorRx, mirrorC.y - mirrorRy, 2 * mirrorRx, 2 * mirrorRy);
        scrollOrnament (g, m.getX() - 13.0f, mirrorC.y, 46.0f, -1.0f);
        scrollOrnament (g, m.getRight() + 13.0f, mirrorC.y, 46.0f, 1.0f);
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.fillEllipse (m.expanded (15.0f).translated (0.0f, 4.0f));
        juce::ColourGradient fr (brassLight, m.getX(), m.getY(), brassDark.darker (0.3f), m.getRight(), m.getBottom(), false);
        fr.addColour (0.3, brass); fr.addColour (0.55, brassLight); fr.addColour (0.8, brass.darker (0.25f));
        g.setGradientFill (fr);
        g.fillEllipse (m.expanded (13.0f));
        g.setColour (brassDark.darker (0.6f).withAlpha (0.7f));
        g.drawEllipse (m.expanded (13.0f), 1.0f);
        g.drawEllipse (m.expanded (8.0f), 0.7f);
        for (int i = 0; i < 96; ++i)    // gallones del marco
        {
            const float a = (float) i * juce::MathConstants<float>::twoPi / 96.0f, ca = std::cos (a), sa = std::sin (a);
            g.setColour ((i % 2 == 0 ? brassDark.darker (0.7f) : brassLight).withAlpha (i % 2 == 0 ? 0.5f : 0.35f));
            g.drawLine (mirrorC.x + ca * (mirrorRx + 1.5f), mirrorC.y + sa * (mirrorRy + 1.5f), mirrorC.x + ca * (mirrorRx + 7.5f), mirrorC.y + sa * (mirrorRy + 7.5f), 1.6f);
        }
        {
            juce::Path ring; ring.addEllipse (m.expanded (13.0f)); ring.setUsingNonZeroWinding (false); ring.addEllipse (m);
            tarnish (g, ring, 88, 2.2f);
        }
        for (int i = 0; i < 48; ++i) // perlado del marco
        {
            const float a = (float) i * juce::MathConstants<float>::twoPi / 48.0f;
            const float px = mirrorC.x + std::cos (a) * (mirrorRx + 10.5f), py = mirrorC.y + std::sin (a) * (mirrorRy + 10.5f);
            g.setColour (brassLight.withAlpha (0.85f));
            g.fillEllipse (px - 1.6f, py - 1.9f, 3.2f, 3.2f);
            g.setColour (brassDark.withAlpha (0.6f));
            g.fillEllipse (px - 0.6f, py + 0.1f, 2.2f, 2.0f);
        }
        juce::Path glass; glass.addEllipse (m);
        juce::Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (glass);
        juce::ColourGradient gl (Colour (0xffd4dcdd), m.getX(), m.getY(), Colour (0xff66747a), m.getRight(), m.getBottom(), false);
        gl.addColour (0.5, Colour (0xffa9b6b9));
        g.setGradientFill (gl);
        g.fillRect (m);
        g.setColour (velvet.withAlpha (0.18f)); // el terciopelo se refleja
        g.fillRect (m);
        juce::Path sheen;
        sheen.addQuadrilateral (m.getX() + 30, m.getY() - 10, m.getX() + 70, m.getY() - 10, m.getX() - 10, m.getBottom() - 60, m.getX() - 40, m.getBottom() - 40);
        sheen.addQuadrilateral (m.getX() + 95, m.getY() - 10, m.getX() + 108, m.getY() - 10, m.getX() + 8, m.getBottom() - 10, m.getX() - 4, m.getBottom() - 10);
        g.setColour (juce::Colours::white.withAlpha (0.16f));
        g.fillPath (sheen);
        juce::Random rnd (21);
        for (int i = 0; i < 90; ++i) // picaduras del azogue
        {
            const float a = rnd.nextFloat() * 6.283f, rr = 0.86f + rnd.nextFloat() * 0.14f, d = 1.0f + rnd.nextFloat() * 3.0f;
            g.setColour (Colour (0xff2a2520).withAlpha (0.08f + rnd.nextFloat() * 0.14f));
            g.fillEllipse (mirrorC.x + std::cos (a) * mirrorRx * rr - d * 0.5f, mirrorC.y + std::sin (a) * mirrorRy * rr - d * 0.5f, d, d);
        }
        for (int i = 0; i < 520; ++i) // azogue comido por los bordes
        {
            const float a = rnd.nextFloat() * 6.283f, rr = 0.78f + std::sqrt (rnd.nextFloat()) * 0.24f, d = 2.0f + rnd.nextFloat() * rnd.nextFloat() * 20.0f;
            g.setColour ((rnd.nextFloat() < 0.75f ? Colour (0xff1c1a17) : Colour (0xff5a4a30)).withAlpha (0.04f + rnd.nextFloat() * 0.1f));
            g.fillEllipse (mirrorC.x + std::cos (a) * mirrorRx * rr - d * 0.5f, mirrorC.y + std::sin (a) * mirrorRy * rr - d * 0.5f, d, d * (0.5f + rnd.nextFloat() * 0.6f));
        }
        for (int i = 0; i < 5; ++i)   // velos
        {
            const float x = m.getX() + rnd.nextFloat() * m.getWidth(), y = m.getY() + rnd.nextFloat() * m.getHeight(), w = 50.0f + rnd.nextFloat() * 80.0f;
            g.setGradientFill (juce::ColourGradient (Colour (0xff6b6250).withAlpha (0.16f), x, y, juce::Colours::transparentBlack, x + w * 0.5f, y, true));
            g.fillEllipse (x - w * 0.5f, y - w * 0.35f, w, w * 0.7f);
        }
        g.setGradientFill (juce::ColourGradient (juce::Colours::transparentBlack, mirrorC.x, mirrorC.y, juce::Colours::black.withAlpha (0.5f), m.getX(), m.getY(), true));
        g.fillRect (m);
    }

    // --- peana de la bailarina ---
    {
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.fillEllipse (mirrorC.x - 56, feetY - 2, 112, 30);
        g.setGradientFill (juce::ColourGradient (brassDark, mirrorC.x - 50, 0, brassDark.darker (0.5f), mirrorC.x + 50, 0, false));
        g.fillEllipse (mirrorC.x - 50, feetY - 2, 100, 24);
        juce::ColourGradient top (brassLight, mirrorC.x - 40, feetY - 10, brassDark, mirrorC.x + 50, feetY + 10, false);
        top.addColour (0.5, brass);
        g.setGradientFill (top);
        g.fillEllipse (mirrorC.x - 50, feetY - 9, 100, 22);
        g.setColour (brassDark.darker (0.5f).withAlpha (0.7f));
        g.drawEllipse (mirrorC.x - 50, feetY - 9, 100, 22, 0.8f);
        g.drawEllipse (mirrorC.x - 38, feetY - 6.5f, 76, 16.5f, 0.6f);
        for (int i = 1; i < 24; ++i)   // gallones del canto
        {
            const float a = juce::MathConstants<float>::pi * (float) i / 24.0f;
            const float x = mirrorC.x - std::cos (a) * 50.0f, y = feetY + 2.0f + std::sin (a) * 11.0f;
            g.setColour (brassDark.darker (0.8f).withAlpha (0.55f)); g.drawLine (x, y + 0.5f, x, y + 7.0f, 1.0f);
            g.setColour (brassLight.withAlpha (0.3f)); g.drawLine (x + 1.2f, y + 0.5f, x + 1.2f, y + 6.0f, 0.7f);
        }
        juce::Path pad; pad.addEllipse (mirrorC.x - 30, feetY - 4.5f, 60, 12.5f);
        g.setGradientFill (juce::ColourGradient (velvet.brighter (0.15f), mirrorC.x - 10, feetY - 2, velvetDark, mirrorC.x + 30, feetY + 8, true));
        g.fillPath (pad);
        g.setColour (brassDark.darker (0.6f)); g.strokePath (pad, juce::PathStrokeType (0.7f));
    }

    // --- tarjeta de melodias (papel envejecido) ---
    {
        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillRect (cardArea.translated (3.0f, 4.0f));
        g.setGradientFill (juce::ColourGradient (ivory, cardArea.getCentreX(), cardArea.getCentreY(), Colour (0xffcdb98e), cardArea.getX(), cardArea.getY(), true));
        g.fillRect (cardArea);
        juce::Random rnd (9);
        for (int i = 0; i < 160; ++i)
        {
            const float d = 1.0f + rnd.nextFloat() * 5.0f;
            g.setColour (Colour (0xff7a5a2a).withAlpha (0.02f + rnd.nextFloat() * 0.05f));
            g.fillEllipse (cardArea.getX() + rnd.nextFloat() * cardArea.getWidth(), cardArea.getY() + rnd.nextFloat() * cardArea.getHeight(), d, d);
        }
        {   // papel viejo: bordes tostados, moho, cerco de taza, dobleces, esquina doblada
            juce::Graphics::ScopedSaveState ss (g);
            g.reduceClipRegion (cardArea.toNearestInt());
            for (int i = 0; i < 520; ++i)
            {
                float fx = rnd.nextFloat(), fy = rnd.nextFloat();
                if (rnd.nextBool()) fx = fx < 0.5f ? fx * fx * fx : 1.0f - std::pow (1.0f - fx, 3.0f); else fy = fy < 0.5f ? fy * fy * fy : 1.0f - std::pow (1.0f - fy, 3.0f);
                const float d = 3.0f + rnd.nextFloat() * 22.0f;
                g.setColour (Colour (0xff6a4514).withAlpha (0.02f + rnd.nextFloat() * 0.05f));
                g.fillEllipse (cardArea.getX() + fx * cardArea.getWidth() - d * 0.5f, cardArea.getY() + fy * cardArea.getHeight() - d * 0.5f, d, d);
            }
            for (int i = 0; i < 70; ++i)
            {
                const float d = 0.8f + rnd.nextFloat() * 2.4f;
                g.setColour (Colour (0xff5a3410).withAlpha (0.12f + rnd.nextFloat() * 0.22f));
                g.fillEllipse (cardArea.getX() + rnd.nextFloat() * cardArea.getWidth(), cardArea.getY() + rnd.nextFloat() * cardArea.getHeight(), d, d);
            }
            g.setColour (Colour (0xff6b4312).withAlpha (0.13f));
            g.drawEllipse (cardArea.getRight() - 96.0f, cardArea.getBottom() - 86.0f, 62.0f, 58.0f, 3.2f);
            g.setColour (Colour (0xff6b4312).withAlpha (0.05f));
            g.fillEllipse (cardArea.getRight() - 96.0f, cardArea.getBottom() - 86.0f, 62.0f, 58.0f);
            const float cy = cardArea.getCentreY() + 9.0f, cx = cardArea.getCentreX() - 4.0f;
            g.setColour (Colour (0xff3a2408).withAlpha (0.16f)); g.fillRect (cardArea.getX(), cy, cardArea.getWidth(), 0.9f); g.fillRect (cx, cardArea.getY(), 0.9f, cardArea.getHeight());
            g.setColour (juce::Colours::white.withAlpha (0.22f)); g.fillRect (cardArea.getX(), cy + 1.0f, cardArea.getWidth(), 0.8f); g.fillRect (cx + 1.0f, cardArea.getY(), 0.8f, cardArea.getHeight());
            juce::Path ear; ear.addTriangle (cardArea.getRight() - 20.0f, cardArea.getBottom(), cardArea.getRight(), cardArea.getBottom() - 20.0f, cardArea.getRight(), cardArea.getBottom());
            g.setColour (velvetDark); g.fillPath (ear);
            juce::Path fold; fold.addTriangle (cardArea.getRight() - 20.0f, cardArea.getBottom(), cardArea.getRight(), cardArea.getBottom() - 20.0f, cardArea.getRight() - 17.0f, cardArea.getBottom() - 17.0f);
            g.setGradientFill (juce::ColourGradient (Colour (0xffe4d3ae), cardArea.getRight() - 18.0f, cardArea.getBottom() - 18.0f, Colour (0xffa58e60), cardArea.getRight() - 6.0f, cardArea.getBottom() - 6.0f, false));
            g.fillPath (fold);
            g.setColour (ink.withAlpha (0.35f)); g.strokePath (fold, juce::PathStrokeType (0.5f));
        }
        for (auto pt : { cardArea.getTopLeft() + Point<float> (13, 12), cardArea.getTopRight() + Point<float> (-13, 12) })
        {   // chinchetas de laton
            g.setColour (juce::Colours::black.withAlpha (0.45f)); g.fillEllipse (pt.x - 3.0f, pt.y - 2.0f, 8.5f, 8.5f);
            g.setGradientFill (juce::ColourGradient (brassLight, pt.x - 2, pt.y - 2.5f, brassDark, pt.x + 3.5f, pt.y + 3.5f, true));
            g.fillEllipse (pt.x - 3.6f, pt.y - 3.6f, 7.2f, 7.2f);
            g.setColour (brassDark.darker (0.7f)); g.drawEllipse (pt.x - 3.6f, pt.y - 3.6f, 7.2f, 7.2f, 0.6f);
        }
        {   // florituras de las esquinas del recuadro
            const auto in = cardArea.reduced (11.0f);
            for (auto c : { in.getTopLeft(), in.getTopRight(), in.getBottomLeft(), in.getBottomRight() })
            {
                const float sx = c.x < cardArea.getCentreX() ? 1.0f : -1.0f, sy = c.y < cardArea.getCentreY() ? 1.0f : -1.0f;
                if (sx < 0 && sy < 0) continue;   // esquina doblada
                juce::Path f; f.startNewSubPath (c.x, c.y + sy * 13.0f); f.quadraticTo (c.x, c.y, c.x + sx * 13.0f, c.y);
                f.startNewSubPath (c.x + sx * 3.0f, c.y + sy * 9.0f); f.quadraticTo (c.x + sx * 3.0f, c.y + sy * 3.0f, c.x + sx * 9.0f, c.y + sy * 3.0f);
                g.setColour (ink.withAlpha (0.6f)); g.strokePath (f, juce::PathStrokeType (0.7f));
            }
        }
        g.setColour (ink.withAlpha (0.8f));
        g.drawRect (cardArea.reduced (5.0f), 1.3f);
        g.drawRect (cardArea.reduced (8.5f), 0.5f);
        g.setFont (serif (17.0f, juce::Font::bold));
        g.drawText (u8 ("MELODÍAS"), cardArea.withHeight (30.0f).translated (0, 9.0f), juce::Justification::centred, false);
        g.setFont (serif (9.5f, juce::Font::italic));
        g.setColour (ink.withAlpha (0.7f));
        g.drawText (juce::String (proc.numMelodies()) + u8 (" aires · gira la rueda del ratón"), cardArea.withHeight (14.0f).translated (0, 34.0f), juce::Justification::centred, false);
        const float ly = cardArea.getY() + 52.0f, cxm = cardArea.getCentreX();
        g.drawLine (cxm - 80, ly, cxm - 8, ly, 0.7f); g.drawLine (cxm + 8, ly, cxm + 80, ly, 0.7f);
        juce::Path dia; dia.addQuadrilateral (cxm, ly - 3.5f, cxm + 4.5f, ly, cxm, ly + 3.5f, cxm - 4.5f, ly);
        g.fillPath (dia);
    }

    // --- placa de laton ---
    {
        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillRoundedRectangle (plateArea.translated (2.0f, 3.0f), 5.0f);
        brassFill (g, plateArea, 5.0f);
        tarnishRect (g, plateArea, 5.0f, 17, 2.0f);
        {
            const auto in = plateArea.reduced (5.0f);
            g.setColour (Colour (0xff2c1c0c).withAlpha (0.6f)); g.drawRoundedRectangle (in, 3.0f, 0.8f);
            g.setColour (brassLight.withAlpha (0.4f)); g.drawRoundedRectangle (in.translated (0.5f, 0.8f), 3.0f, 0.5f);
            {   // rombo central del filete
                const float mx = plateArea.getCentreX(), my = plateArea.getY() + 75.5f;
                juce::Path dia; dia.addQuadrilateral (mx - 5.0f, my, mx, my - 3.5f, mx + 5.0f, my, mx, my + 3.5f);
                g.setColour (Colour (0xff2c1c0c).withAlpha (0.8f)); g.fillPath (dia);
            }
            for (float side : { -1.0f, 1.0f })   // filete con florituras
            {
                const float x0 = plateArea.getCentreX() + side * 9.0f, y0 = plateArea.getY() + 75.5f;
                juce::Path f; f.startNewSubPath (x0, y0); f.cubicTo (x0 + side * 16.0f, y0 - 6.0f, x0 + side * 30.0f, y0 + 6.0f, x0 + side * 46.0f, y0);
                f.cubicTo (x0 + side * 54.0f, y0 - 3.0f, x0 + side * 52.0f, y0 - 8.0f, x0 + side * 47.0f, y0 - 5.0f);
                g.setColour (Colour (0xff2c1c0c).withAlpha (0.75f)); g.strokePath (f, juce::PathStrokeType (0.8f));
            }
        }
        for (auto pt : { plateArea.getTopLeft() + Point<float> (9, 9), plateArea.getTopRight() + Point<float> (-9, 9),
                         plateArea.getBottomLeft() + Point<float> (9, -9), plateArea.getBottomRight() + Point<float> (-9, -9) })
            screw (g, pt.x, pt.y, 3.2f, pt.x * 0.7f);
        engraved (g, "BAILARINA", plateArea.withHeight (44.0f).translated (0, 8.0f), serif (31.0f, juce::Font::bold), Colour (0xff2c1c0c));
        engraved (g, u8 ("CAJA DE MÚSICA · MODELADO FÍSICO"), plateArea.withHeight (14.0f).translated (0, 50.0f), serif (9.6f), Colour (0xff3a2610));
    }

    // --- hueco del mecanismo ---
    {
        g.setGradientFill (juce::ColourGradient (Colour (0xff1b130d), 0, mechArea.getY(), Colour (0xff0a0705), 0, mechArea.getBottom(), false));
        g.fillRect (mechArea);
        const auto bed = mechArea.reduced (10.0f, 7.0f);
        juce::ColourGradient bg (Colour (0xff6d5626), bed.getX(), bed.getY(), Colour (0xff3a2c12), bed.getRight(), bed.getBottom(), false);
        bg.addColour (0.5, Colour (0xff56421c));
        g.setGradientFill (bg);
        g.fillRoundedRectangle (bed, 4.0f);
        juce::Random rnd (3);
        for (int i = 0; i < 300; ++i) // cepillado
        {
            const float y = bed.getY() + rnd.nextFloat() * bed.getHeight(), x = bed.getX() + rnd.nextFloat() * bed.getWidth();
            g.setColour ((rnd.nextBool() ? brassLight : juce::Colours::black).withAlpha (0.04f + rnd.nextFloat() * 0.05f));
            g.fillRect (x, y, 30.0f + rnd.nextFloat() * 90.0f, 0.7f);
        }
        g.setColour (juce::Colours::black.withAlpha (0.6f));
        g.drawRoundedRectangle (bed, 4.0f, 1.0f);
        for (auto pt : { bed.getTopLeft() + Point<float> (10, 10), bed.getTopRight() + Point<float> (-10, 10),
                         bed.getBottomLeft() + Point<float> (10, -10), bed.getBottomRight() + Point<float> (-10, -10) })
            screw (g, pt.x, pt.y, 4.0f, pt.y * 0.3f);
        tarnishRect (g, bed, 4.0f, 29, 1.5f);
        for (int i = 0; i < 9; ++i)   // manchas de aceite viejo
        {
            const float x = bed.getX() + rnd.nextFloat() * bed.getWidth(), y = bed.getY() + rnd.nextFloat() * bed.getHeight(), w = 20.0f + rnd.nextFloat() * 60.0f;
            g.setGradientFill (juce::ColourGradient (Colour (0xff0b0704).withAlpha (0.35f), x, y, juce::Colours::transparentBlack, x + w * 0.5f, y, true));
            g.fillEllipse (x - w * 0.5f, y - w * 0.22f, w, w * 0.44f);
        }
        engraved (g, u8 ("MECANISMO DE 49 LÁMINAS · CILINDRO DE PÚAS"), Rectangle<float> (cylX0, 473.2f, cylX1 - cylX0, 8.0f), serif (7.0f), Colour (0xff1d1406).withAlpha (0.85f));
        insetShadow (g, mechArea, 18.0f, 0.85f);
        g.setColour (brassDark.withAlpha (0.9f));
        g.drawRect (mechArea.expanded (1.0f), 1.5f);

        // soportes del cilindro
        for (float bx : { cylX0 - 22.0f, cylX1 + 8.0f })
        {
            brassFill (g, { bx, cylCy - 24.0f, 14.0f, 58.0f }, 3.0f);
            screw (g, bx + 7.0f, cylCy + 27.0f, 2.6f, 0.6f);
        }
        // base del peine (acero), con corte diagonal: graves largos, agudos cortos
        juce::Path base;
        base.startNewSubPath (cylX0 - 6, 356); base.lineTo (cylX1 + 6, 356);
        base.lineTo (cylX1 + 6, 395); base.lineTo (cylX0 - 6, 369); base.closeSubPath();
        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillPath (base, juce::AffineTransform::translation (1.5f, 2.5f));
        juce::ColourGradient st (Colour (0xffdfe3e6), 0, 356, Colour (0xff7c848b), 0, 395, false);
        st.addColour (0.4, Colour (0xffaab1b7));
        g.setGradientFill (st);
        g.fillPath (base);
        g.setColour (Colour (0xff3d4348));
        g.strokePath (base, juce::PathStrokeType (0.8f));
        for (int i = 0; i < 7; ++i) screw (g, cylX0 + 20.0f + (float) i * 90.0f, 362.5f, 3.0f, (float) i * 1.3f);

        g.setFont (serif (9.5f, juce::Font::italic));
        g.setColour (brassLight.withAlpha (0.55f));
        g.drawText ("regulador", Rectangle<float> (fanC.x - 40, 468, 80, 12), juce::Justification::centred, false);
        g.drawText ("cuerda", Rectangle<float> (keyC.x - 40, 453, 80, 12), juce::Justification::centred, false);
    }

    // --- panel de mandos ---
    {
        fillWood (g, knobArea, woodDark.brighter (0.12f), Colour (0xff1c0c05), 77, 0.13f);
        insetShadow (g, knobArea, 14.0f, 0.7f);
        g.setColour (brassDark.withAlpha (0.9f));
        g.drawRect (knobArea.expanded (1.0f), 1.5f);
        g.setColour (brass.withAlpha (0.35f));
        g.drawRect (knobArea.reduced (5.0f), 0.7f);
        const float step = knobArea.getWidth() / (float) kNumKnobs;
        for (int i = 0; i < kNumKnobs; ++i)
        {
            const float cx = knobArea.getX() + step * ((float) i + 0.5f), cy = knobArea.getY() + 46.0f, R = 32.5f;
            // esfera de laton grabada bajo cada mando
            g.setColour (juce::Colours::black.withAlpha (0.55f));
            g.fillEllipse (cx - R - 1.0f, cy - R + 2.0f, 2 * R + 2, 2 * R + 2);
            juce::ColourGradient dg (brass.darker (0.15f), cx - R, cy - R, brassDark.darker (0.35f), cx + R, cy + R, false);
            dg.addColour (0.5, brass.darker (0.35f));
            g.setGradientFill (dg);
            g.fillEllipse (cx - R, cy - R, 2 * R, 2 * R);
            g.setColour (brassDark.darker (0.8f)); g.drawEllipse (cx - R, cy - R, 2 * R, 2 * R, 0.9f);
            g.setColour (brassLight.withAlpha (0.35f)); g.drawEllipse (cx - R + 1.5f, cy - R + 1.5f, 2 * R - 3, 2 * R - 3, 0.5f);
            for (int k = 0; k <= 20; ++k)
            {
                const float a = juce::MathConstants<float>::pi * (1.2f + 1.6f * (float) k / 20.0f);
                const float r0 = k % 5 == 0 ? R - 6.5f : R - 4.5f;
                g.setColour (Colour (0xff1d1206).withAlpha (0.85f));
                g.drawLine (cx + std::sin (a) * r0, cy - std::cos (a) * r0, cx + std::sin (a) * (R - 1.5f), cy - std::cos (a) * (R - 1.5f), k % 5 == 0 ? 1.2f : 0.7f);
            }
            { juce::Path d; d.addEllipse (cx - R, cy - R, 2 * R, 2 * R); tarnish (g, d, 300 + i, 1.8f); }
            // plaquita con el rotulo
            const Rectangle<float> pl (cx - 35.0f, knobArea.getY() + 82.5f, 70.0f, 13.5f);
            g.setColour (juce::Colours::black.withAlpha (0.5f)); g.fillRoundedRectangle (pl.translated (1.0f, 1.5f), 2.0f);
            brassFill (g, pl, 2.0f);
            tarnishRect (g, pl, 2.0f, 500 + i, 1.5f);
            engraved (g, u8 (knobLabels[i]), pl, serif (8.8f, juce::Font::bold), Colour (0xff24160a));
        }
        // bocallave en el frente
        {
            const float cx = (float) kW * 0.5f, cy = (float) kH - 15.0f;
            juce::Path esc; esc.addEllipse (cx - 9.0f, cy - 10.0f, 18.0f, 20.0f);
            g.setColour (juce::Colours::black.withAlpha (0.5f)); g.fillPath (esc, juce::AffineTransform::translation (1.0f, 1.5f));
            g.setGradientFill (juce::ColourGradient (brassLight, cx - 7, cy - 9, brassDark, cx + 8, cy + 9, false)); g.fillPath (esc);
            g.setColour (brassDark.darker (0.7f)); g.strokePath (esc, juce::PathStrokeType (0.8f));
            tarnish (g, esc, 71, 2.0f);
            g.setColour (Colour (0xff070403)); g.fillEllipse (cx - 2.6f, cy - 5.5f, 5.2f, 5.2f);
            juce::Path slot; slot.addTriangle (cx, cy - 3.0f, cx - 2.4f, cy + 6.0f, cx + 2.4f, cy + 6.0f); g.fillPath (slot);
        }
    }

    // cantoneras
    cornerPiece (g, 2.0f, 2.0f, 1.0f, 1.0f, 3);
    cornerPiece (g, (float) kW - 2.0f, 2.0f, -1.0f, 1.0f, 4);
    cornerPiece (g, 2.0f, (float) kH - 2.0f, 1.0f, -1.0f, 5);
    cornerPiece (g, (float) kW - 2.0f, (float) kH - 2.0f, -1.0f, -1.0f, 6);

    // patina general: polvo, aranazos, vineta y tono sepia. Se pinta encima de todo.
    patina = juce::Image (juce::Image::ARGB, kW, kH, true);
    {
        juce::Graphics pg (patina);
        juce::Random r (1887);
        pg.fillAll (Colour (0xff5a3a12).withAlpha (0.07f));
        for (int i = 0; i < 4200; ++i)
        {
            const float d = 0.6f + r.nextFloat() * r.nextFloat() * 1.8f;
            pg.setColour (Colour (0xffe9dcc0).withAlpha (0.03f + r.nextFloat() * 0.09f));
            pg.fillEllipse (r.nextFloat() * (float) kW, r.nextFloat() * (float) kH, d, d);
        }
        for (int i = 0; i < 150; ++i)
        {
            const float x = r.nextFloat() * (float) kW, y = r.nextFloat() * (float) kH, a = r.nextFloat() * 6.283f, len = 8.0f + r.nextFloat() * r.nextFloat() * 130.0f;
            juce::Path sc; sc.startNewSubPath (x, y);
            sc.quadraticTo (x + std::cos (a) * len * 0.5f + (r.nextFloat() - 0.5f) * 6.0f, y + std::sin (a) * len * 0.5f + (r.nextFloat() - 0.5f) * 6.0f, x + std::cos (a) * len, y + std::sin (a) * len);
            pg.setColour ((r.nextFloat() < 0.7f ? Colour (0xfff3e8cf) : Colour (0xff0a0502)).withAlpha (0.04f + r.nextFloat() * 0.09f));
            pg.strokePath (sc, juce::PathStrokeType (0.4f + r.nextFloat() * 0.5f));
        }
        for (int i = 0; i < 14; ++i)   // cercos y manchas grandes
        {
            const float x = r.nextFloat() * (float) kW, y = r.nextFloat() * (float) kH, w = 60.0f + r.nextFloat() * 180.0f;
            pg.setGradientFill (juce::ColourGradient (Colour (0xff120a04).withAlpha (0.14f), x, y, juce::Colours::transparentBlack, x + w * 0.5f, y, true));
            pg.fillEllipse (x - w * 0.5f, y - w * 0.3f, w, w * 0.6f);
        }
        juce::ColourGradient vg (juce::Colours::transparentBlack, (float) kW * 0.5f, (float) kH * 0.46f, Colour (0xff050201).withAlpha (0.5f), 0.0f, 0.0f, true);
        vg.addColour (0.55, juce::Colours::transparentBlack);
        pg.setGradientFill (vg);
        pg.fillAll();
    }
}

// ---------------------------------------------------------------------------
void BailarinaEditor::drawBallerina (juce::Graphics& g, float cx, float fy, float S, float angle, float alpha) const
{
    const float c = std::cos (angle);
    const float sx = (c >= 0 ? 1.0f : -1.0f) * (0.32f + 0.68f * std::abs (c));
    const auto body = juce::AffineTransform::scale (sx * S, S).translated (cx, fy);
    const auto flat = juce::AffineTransform::scale (S, S).translated (cx, fy);
    const float shade = 0.82f + 0.18f * std::abs (c); // algo mas oscura de perfil
    auto col = [alpha, shade] (Colour base) { return base.withMultipliedBrightness (shade).withAlpha (alpha); };
    const juce::PathStrokeType round5 (5.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);

    // pierna de apoyo, en punta
    juce::Path leg;
    leg.startNewSubPath (-4.6f, -82); leg.quadraticTo (-3.2f, -40, -1.3f, -7); leg.lineTo (0.2f, 0);
    leg.lineTo (2.3f, -8); leg.quadraticTo (4.2f, -40, 5.6f, -82); leg.closeSubPath();
    g.setColour (col (tights)); g.fillPath (leg, body);
    juce::Path shoe; shoe.addEllipse (-1.6f, -9.5f, 3.8f, 9.5f);
    g.setColour (col (rose)); g.fillPath (shoe, body);

    // pierna en passé
    juce::Path thigh; thigh.startNewSubPath (3, -80); thigh.lineTo (23, -57);
    juce::Path shin;  shin.startNewSubPath (23, -57); shin.lineTo (6, -41);
    juce::Path t2, s2;
    juce::PathStrokeType (6.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded).createStrokedPath (t2, thigh);
    round5.createStrokedPath (s2, shin);
    g.setColour (col (tights)); g.fillPath (t2, body); g.fillPath (s2, body);
    juce::Path shoe2; shoe2.addEllipse (2.5f, -45.5f, 7.0f, 4.6f);
    g.setColour (col (rose)); g.fillPath (shoe2, body);

    // tutu (simetrico: no se estrecha al girar), con festones que giran
    {
        juce::Path under0; under0.addEllipse (-42, -94.5f, 84, 22);
        g.setColour (col (Colour (0xfffbe9ec)).withMultipliedAlpha (0.28f)); g.fillPath (under0, flat);
        for (int i = 0; i < 22; ++i)   // segunda capa de tul
        {
            const float a = -angle * 0.6f + (float) i * juce::MathConstants<float>::twoPi / 22.0f;
            juce::Path sc; sc.addEllipse (std::cos (a) * 40.0f - 5.0f, -83.5f + std::sin (a) * 10.5f - 3.0f, 10.0f, 6.0f);
            g.setColour (col (Colour (0xfff7dfe4)).withMultipliedAlpha (0.4f)); g.fillPath (sc, flat);
        }
        juce::Path under; under.addEllipse (-37, -93.5f, 74, 19);
        g.setColour (col (Colour (0xffffffff)).withMultipliedAlpha (0.55f)); g.fillPath (under, flat);
        for (int i = 0; i < 16; ++i)
        {
            const float a = angle + (float) i * juce::MathConstants<float>::twoPi / 16.0f;
            const float ex = std::cos (a) * 36.0f, ey = -84.5f + std::sin (a) * 9.0f;
            juce::Path sc; sc.addEllipse (ex - 6.5f, ey - 3.6f, 13.0f, 7.2f);
            g.setColour (col (std::sin (a) > 0 ? Colour (0xfffff6f6) : Colour (0xfff2cfd6)).withMultipliedAlpha (0.9f));
            g.fillPath (sc, flat);
        }
        juce::Path top; top.addEllipse (-33, -92.5f, 66, 15.5f);
        g.setGradientFill (juce::ColourGradient (col (Colour (0xfffffafa)), cx, fy - 92 * S, col (Colour (0xfff3c4cf)), cx, fy - 77 * S, false));
        g.fillPath (top, flat);
        for (int i = 0; i < 12; ++i)   // lentejuelas
        {
            const float a = angle + 0.4f + (float) i * juce::MathConstants<float>::twoPi / 12.0f, rr = i % 2 == 0 ? 24.0f : 15.0f;
            juce::Path sp; sp.addEllipse (std::cos (a) * rr - 0.8f, -85.0f + std::sin (a) * rr * 0.235f - 0.8f, 1.6f, 1.6f);
            g.setColour (col (Colour (0xffe2c063))); g.fillPath (sp, flat);
        }
    }

    // corpiño
    juce::Path bod;
    bod.startNewSubPath (-6, -84); bod.lineTo (6, -84); bod.quadraticTo (6.5f, -100, 10.5f, -118);
    bod.lineTo (-10.5f, -118); bod.quadraticTo (-6.5f, -100, -6, -84); bod.closeSubPath();
    g.setColour (col (rose)); g.fillPath (bod, body);
    juce::Path trim; trim.addRectangle (-10.5f, -119.5f, 21.0f, 2.2f);
    g.setColour (col (Colour (0xfffff2f2))); g.fillPath (trim, body);

    // hombros, cuello, cabeza
    juce::Path sh; sh.addEllipse (-10.5f, -123.5f, 21, 7);
    juce::Path neck; neck.addRectangle (-2.3f, -131, 4.6f, 10);
    juce::Path head; head.addEllipse (-7, -146.5f, 14, 17);
    g.setColour (col (skin)); g.fillPath (sh, body); g.fillPath (neck, body); g.fillPath (head, body);
    juce::Path hr; hr.addPieSegment (-7.6f, -147.5f, 15.2f, 15.0f, -1.75f, 1.75f, 0.0f);
    juce::Path bun; bun.addEllipse (-4.3f, -153.5f, 8.6f, 8.6f);
    g.setColour (col (hair)); g.fillPath (hr, body); g.fillPath (bun, body);

    if (c > 0.3f)   // de frente: carita de porcelana
    {
        juce::Path eyes; eyes.addEllipse (-3.6f, -139.6f, 1.7f, 1.3f); eyes.addEllipse (1.9f, -139.6f, 1.7f, 1.3f);
        g.setColour (col (Colour (0xff2a1a12))); g.fillPath (eyes, body);
        juce::Path lips; lips.addEllipse (-1.3f, -134.6f, 2.6f, 1.2f);
        g.setColour (col (Colour (0xffb8424e))); g.fillPath (lips, body);
        juce::Path blush; blush.addEllipse (-5.6f, -137.6f, 3.0f, 2.2f); blush.addEllipse (2.6f, -137.6f, 3.0f, 2.2f);
        g.setColour (col (rose).withMultipliedAlpha (0.45f)); g.fillPath (blush, body);
    }
    else if (c < -0.15f)   // de espaldas: el pelo recogido cubre la cabeza
    {
        juce::Path back; back.addEllipse (-7.2f, -147.0f, 14.4f, 13.5f);
        g.setColour (col (hair)); g.fillPath (back, body);
    }
    {   // diadema, ribete dorado del corpino y cintas de la zapatilla
        juce::Path gold;
        gold.addRectangle (-6.5f, -146.2f, 13.0f, 1.1f);
        gold.addRectangle (-6.2f, -86.2f, 12.4f, 1.6f);
        for (int i = 0; i < 3; ++i) gold.addEllipse (-0.8f, -112.0f + 8.0f * (float) i, 1.6f, 1.6f);
        g.setColour (col (Colour (0xffd9b65a))); g.fillPath (gold, body);
        juce::Path rib;
        rib.startNewSubPath (-2.6f, -24.0f); rib.lineTo (3.4f, -17.0f); rib.startNewSubPath (3.2f, -24.0f); rib.lineTo (-2.2f, -17.0f);
        rib.startNewSubPath (-2.0f, -15.0f); rib.lineTo (2.8f, -10.0f); rib.startNewSubPath (2.8f, -15.0f); rib.lineTo (-1.8f, -10.0f);
        juce::Path rs; juce::PathStrokeType (0.9f).createStrokedPath (rs, rib);
        g.setColour (col (rose.darker (0.15f))); g.fillPath (rs, body);
        // sombreado lateral de porcelana
        juce::Path shadeP; shadeP.addRectangle (2.0f, -150.0f, 9.0f, 150.0f);
        g.setGradientFill (juce::ColourGradient (juce::Colours::transparentBlack, cx, fy, Colour (0xff3a1c1c).withAlpha (0.16f * alpha), cx + 10.0f * S, fy, false));
        g.fillPath (leg, body);
    }

    // brazos en quinta posicion
    for (float side : { -1.0f, 1.0f })
    {
        juce::Path arm; arm.startNewSubPath (side * 9.5f, -120);
        arm.quadraticTo (side * 29, -140, side * 5, -165);
        juce::Path a2;
        juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded).createStrokedPath (a2, arm);
        g.setColour (col (skin)); g.fillPath (a2, body);
    }
}

int BailarinaEditor::cardRowAt (Point<int> p) const
{
    const auto pf = p.toFloat();
    if (! cardArea.reduced (9.0f, 0.0f).contains (pf)) return -1;
    const int row = (int) std::floor ((pf.y - cardRow0) / cardRowH);
    return (row >= 0 && row < kCardRows && row + cardScroll < proc.numMelodies()) ? row + cardScroll : -1;
}

void BailarinaEditor::drawTuneCard (juce::Graphics& g) const
{
    const int current = shownMelody;
    const int total = proc.numMelodies();
    // barra de posicion de la lista
    {
        const float x = cardArea.getRight() - 12.5f, y0 = cardRow0 + 2.0f, h = cardRowH * (float) kCardRows - 4.0f;
        g.setColour (ink.withAlpha (0.18f));
        g.fillRect (x, y0, 1.5f, h);
        const float th = juce::jmax (12.0f, h * (float) kCardRows / (float) juce::jmax (kCardRows, total));
        const float ty = y0 + (h - th) * (float) cardScroll / (float) juce::jmax (1, total - kCardRows);
        g.setColour (ink.withAlpha (0.65f));
        g.fillRoundedRectangle (x - 1.0f, ty, 3.5f, th, 1.5f);
    }
    for (int i = cardScroll; i < juce::jmin (total, cardScroll + kCardRows); ++i)
    {
        const auto& mel = proc.melody (i);
        Rectangle<float> row (cardArea.getX() + 12.0f, cardRow0 + cardRowH * (float) (i - cardScroll), cardArea.getWidth() - 28.0f, cardRowH);
        const bool sel = i == current;
        if (sel)        { g.setColour (inkRed.withAlpha (0.13f)); g.fillRect (row.reduced (0.0f, 1.0f)); }
        else if (i == hoverRow) { g.setColour (ink.withAlpha (0.07f)); g.fillRect (row.reduced (0.0f, 1.0f)); }
        g.setColour (sel ? inkRed : ink.withAlpha (0.9f));
        g.setFont (serif (10.5f, sel ? juce::Font::bold : juce::Font::plain));
        g.drawText (juce::String (i + 1), row.withWidth (26.0f).withHeight (14.0f).translated (0, 1.0f), juce::Justification::centredRight, false);
        g.setFont (serif (13.2f, sel ? juce::Font::bold : juce::Font::plain));
        g.drawText (u8 (mel.title.c_str()), row.withTrimmedLeft (33.0f).withHeight (14.0f).translated (0, 0.5f), juce::Justification::centredLeft, false);
        g.setFont (serif (9.6f, juce::Font::italic));
        g.setColour ((sel ? inkRed : ink).withAlpha (0.72f));
        g.drawText (u8 (mel.composer.c_str()), row.withTrimmedLeft (33.0f).withTop (row.getY() + 13.0f), juce::Justification::centredLeft, false);
    }
}

void BailarinaEditor::drawMechanism (juce::Graphics& g) const
{
    const auto& mel = proc.melody (shownMelody);

    // eje
    g.setColour (Colour (0xff8d9399));
    g.fillRect (gearC.x, cylCy - 2.5f, keyC.x - gearC.x - 16.0f, 5.0f);
    g.setColour (juce::Colours::white.withAlpha (0.35f));
    g.fillRect (gearC.x, cylCy - 2.0f, keyC.x - gearC.x - 16.0f, 1.2f);

    // cilindro
    const Rectangle<float> c (cylX0, cylCy - cylR, cylX1 - cylX0, 2 * cylR);
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillRoundedRectangle (c.translated (0.0f, 5.0f), 4.0f);
    juce::ColourGradient cg (brassDark.darker (0.55f), 0, c.getY(), brassDark.darker (0.75f), 0, c.getBottom(), false);
    cg.addColour (0.12, brassDark); cg.addColour (0.33, brass); cg.addColour (0.42, brassLight);
    cg.addColour (0.55, brass); cg.addColour (0.8, brassDark);
    g.setGradientFill (cg);
    g.fillRoundedRectangle (c, 4.0f);
    g.setColour (brassDark.darker (0.7f).withAlpha (0.6f));
    g.fillRect (c.getX() + 5.0f, c.getY(), 1.2f, c.getHeight());
    g.fillRect (c.getRight() - 6.2f, c.getY(), 1.2f, c.getHeight());

    // puas: una vuelta del cilindro = la melodia entera. Llegan arriba (al peine) cuando suenan.
    const double loop = mel.lengthBeats;
    if (loop > 0.0)
    {
        for (const auto& pin : mel.pins)
        {
            double d = pin.beat / loop - (double) cylPos; d -= std::floor (d);
            if (d > 0.5) continue;
            const float phi = juce::MathConstants<float>::halfPi - juce::MathConstants<float>::twoPi * (float) d;
            const float depth = std::cos (phi);
            const float px = toothX (pin.note), py = cylCy - cylR * std::sin (phi);
            const float h = 1.4f + 2.4f * depth;
            g.setColour (Colour (0xff1c1206).withAlpha (0.55f * depth + 0.2f));
            g.fillEllipse (px - 1.9f, py - h * 0.5f + 1.0f, 3.8f, h + 0.6f);
            g.setColour (Colour (0xffe9edf0).withAlpha (0.35f + 0.65f * depth));
            g.fillEllipse (px - 1.5f, py - h * 0.5f, 3.0f, h);
        }
    }

    // laminas del peine
    const float pitch = (cylX1 - cylX0) / (float) (toothHi - toothLo + 1);
    for (int n = toothLo; n <= toothHi; ++n)
    {
        const float f = (float) (n - toothLo) / (float) (toothHi - toothLo);
        const float x = toothX (n), yBase = 368.5f + f * 26.0f, yTip = cylCy - cylR - 0.5f;
        const float lv = juce::jmin (1.0f, tooth[(size_t) n] * 1.3f);
        const float wob = lv > 0.004f ? std::sin ((float) frame * 2.4f + (float) n) * (0.4f + 2.6f * lv) : 0.0f;
        const float wB = pitch * 0.74f, wT = pitch * 0.6f;
        juce::Path t;
        t.startNewSubPath (x - wB * 0.5f, yBase - 2.0f); t.lineTo (x + wB * 0.5f, yBase - 2.0f);
        t.lineTo (x + wT * 0.5f + wob, yTip); t.lineTo (x - wT * 0.5f + wob, yTip); t.closeSubPath();
        juce::ColourGradient tg (Colour (0xffc9ced2), x - wB * 0.5f, 0, Colour (0xff70787f), x + wB * 0.5f, 0, false);
        g.setGradientFill (tg);
        g.fillPath (t);
        if (lv > 0.004f)
        {
            g.setColour (Colour (0xfffff3c4).withAlpha (juce::jmin (0.9f, 0.25f + lv)));
            g.fillPath (t);
        }
        g.setColour (Colour (0xff2f3438).withAlpha (0.8f));
        g.strokePath (t, juce::PathStrokeType (0.5f));
    }

    // regulador: engranaje + aspas
    {
        const float gr = 15.0f;
        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillEllipse (gearC.x - gr - 2, gearC.y - gr + 1, 2 * gr + 4, 2 * gr + 4);
        juce::Path gear;
        gear.addStar (gearC, 18, gr - 2.5f, gr + 2.5f, -fanAngle * 0.13f);
        g.setGradientFill (juce::ColourGradient (brassLight, gearC.x - gr, gearC.y - gr, brassDark, gearC.x + gr, gearC.y + gr, false));
        g.fillPath (gear);
        g.setColour (brassDark.darker (0.6f));
        g.strokePath (gear, juce::PathStrokeType (0.6f));
        g.fillEllipse (gearC.x - 3, gearC.y - 3, 6, 6);
        // eje vertical y aspas (vistas de canto: su anchura aparente cambia al girar)
        g.setColour (Colour (0xff8d9399));
        g.fillRect (fanC.x - 1.5f, fanC.y - 6.0f, 3.0f, gearC.y - fanC.y - 6.0f);
        const float w = 30.0f * std::abs (std::cos (fanAngle)) + 3.0f;
        const bool blur = spinSpeed > 0.25f;
        for (float side : { -1.0f, 1.0f })
        {
            Rectangle<float> blade (side > 0 ? fanC.x + 2.0f : fanC.x - 2.0f - w, fanC.y - 22.0f, w, 19.0f);
            g.setGradientFill (juce::ColourGradient (brassLight.withAlpha (blur ? 0.75f : 1.0f), blade.getX(), blade.getY(), brass.darker (0.3f).withAlpha (blur ? 0.75f : 1.0f), blade.getRight(), blade.getBottom(), false));
            g.fillRoundedRectangle (blade, 2.0f);
            g.setColour (brassDark.darker (0.5f).withAlpha (0.8f));
            g.drawRoundedRectangle (blade, 2.0f, 0.6f);
        }
        if (blur)
        {
            g.setColour (brassLight.withAlpha (0.10f));
            g.fillRoundedRectangle (fanC.x - 33.0f, fanC.y - 22.0f, 66.0f, 19.0f, 2.0f);
        }
    }

    // llave de cuerda (mariposa) y reserva de cuerda
    {
        const float kw = 0.3f + 0.7f * std::abs (std::cos (keyAngle));
        g.setColour (juce::Colours::black.withAlpha (0.4f));
        g.fillEllipse (keyC.x - 26, keyC.y - 14 + 4, 52, 34);
        g.setColour (Colour (0xff8d9399));
        g.fillRect (keyC.x - 3.0f, keyC.y - 3.0f, 6.0f, 14.0f);
        for (float side : { -1.0f, 1.0f })
        {
            juce::Path wing;
            wing.addEllipse (side > 0 ? 2.0f : -2.0f - 24.0f * kw, -17.0f, 24.0f * kw, 30.0f);
            juce::Path hole;
            hole.addEllipse (side > 0 ? 2.0f + 7.0f * kw : -2.0f - 17.0f * kw, -9.0f, 10.0f * kw, 14.0f);
            wing.setUsingNonZeroWinding (false);
            wing.addPath (hole);
            const auto tr = juce::AffineTransform::translation (keyC.x, keyC.y);
            g.setGradientFill (juce::ColourGradient (brassLight, keyC.x - 26, keyC.y - 17, brassDark, keyC.x + 26, keyC.y + 13, false));
            g.fillPath (wing, tr);
            g.setColour (brassDark.darker (0.6f));
            g.strokePath (wing, juce::PathStrokeType (0.7f), tr);
        }
        g.setGradientFill (juce::ColourGradient (brassLight, keyC.x - 5, keyC.y - 8, brassDark, keyC.x + 5, keyC.y + 6, false));
        g.fillRoundedRectangle (keyC.x - 4.5f, keyC.y - 8.0f, 9.0f, 14.0f, 2.0f);

        const Rectangle<float> bar (keyC.x - 34.0f, 467.0f, 68.0f, 7.0f);
        g.setColour (Colour (0xff120805));
        g.fillRoundedRectangle (bar, 3.5f);
        const bool real = proc.apvts.getRawParameterValue ("spring")->load() > 0.5f;
        g.setColour ((springLevel < 0.22f ? inkRed.brighter (0.4f) : brassLight).withAlpha (real ? 1.0f : 0.4f));
        g.fillRoundedRectangle (bar.reduced (1.2f).withWidth (juce::jmax (2.0f, (bar.getWidth() - 2.4f) * springLevel)), 2.5f);
        g.setColour (brass.withAlpha (0.8f));
        g.drawRoundedRectangle (bar, 3.5f, 0.8f);
    }
}

void BailarinaEditor::drawKnobLabels (juce::Graphics& g) const
{
    const float step = knobArea.getWidth() / (float) kNumKnobs;
    g.setFont (serif (10.5f, juce::Font::italic));
    g.setColour (ivory.withAlpha (0.72f));
    for (int i = 0; i < kNumKnobs; ++i)
        if (auto* prm = proc.apvts.getParameter (knobIds[i]))
            g.drawText (prm->getCurrentValueAsText(), Rectangle<float> (knobArea.getX() + step * (float) i, knobArea.getY() + 99.0f, step, 14.0f), juce::Justification::centred, false);
}

void BailarinaEditor::paint (juce::Graphics& g)
{
    g.drawImageAt (background, 0, 0);

    // reflejo en el espejo (se ve la espalda: gira al reves)
    {
        juce::Graphics::ScopedSaveState ss (g);
        juce::Path glass; glass.addEllipse (mirrorC.x - mirrorRx, mirrorC.y - mirrorRy, 2 * mirrorRx, 2 * mirrorRy);
        g.reduceClipRegion (glass);
        drawBallerina (g, mirrorC.x + 6.0f, feetY - 22.0f, 0.86f, -spin + juce::MathConstants<float>::pi, 0.30f);
    }
    // sombra y figura
    g.setColour (juce::Colours::black.withAlpha (0.28f));
    g.fillEllipse (mirrorC.x - 20, feetY - 3.5f, 40, 8);
    drawBallerina (g, mirrorC.x, feetY, 1.1f, spin, 1.0f);

    drawTuneCard (g);
    drawMechanism (g);
    drawKnobLabels (g);

    g.setFont (serif (9.5f, juce::Font::italic));
    g.setColour (ivory.withAlpha (0.45f));
    g.drawText (u8 ("toca el espejo para abrir la música · la llave da cuerda"), Rectangle<float> (310, 312, 320, 14), juce::Justification::centred, false);
    g.drawImageAt (patina, 0, 0);
    if (proc.apvts.getRawParameterValue ("sync")->load() > 0.5f && ! proc.uiHostSync.load())
        g.drawText (u8 ("sin tempo del DAW: velocidad manual"), Rectangle<float> (648, 145, 240, 13), juce::Justification::centred, false);
}

void BailarinaEditor::timerCallback()
{
    ++frame;
    const float target = proc.uiSpeed.load();
    spinSpeed += 0.12f * (target - spinSpeed);
    spin = std::fmod (spin + spinSpeed * 0.075f, juce::MathConstants<float>::twoPi);
    fanAngle = std::fmod (fanAngle + spinSpeed * 1.9f, 1000.0f * juce::MathConstants<float>::twoPi);
    keyAngle += 0.2f * (keyTarget - keyAngle);
    if (const bool on = proc.apvts.getRawParameterValue ("lume")->load() > 0.5f; on != lumeShown)
    {
        lumeShown = on;                                   // el panel Lume aparece bajo la caja
        lumePanel.setVisible (on);
        setSize (kW, kH + (on ? LumePanel::kHeight : 0));
    }
    if (lumeShown) lumePanel.tick();
    cylPos = proc.uiCylPos.load();
    if (const int m = proc.uiMelody.load(); m != shownMelody)
    {
        shownMelody = m;                                   // la radio cambia de pieza: la lista la sigue
        if (m < cardScroll || m >= cardScroll + kCardRows) { cardScroll = m - kCardRows / 2; scrollCard (0); }
    }
    springLevel = proc.uiSpring.load();
    for (int i = 0; i < 128; ++i)
    {
        const float v = proc.uiTooth[(size_t) i].load (std::memory_order_relaxed);
        tooth[(size_t) i] = v > tooth[(size_t) i] ? v : tooth[(size_t) i] * 0.8f + v * 0.2f;
    }
    repaint();
}

void BailarinaEditor::mouseDown (const juce::MouseEvent& e)
{
    const auto p = e.getPosition();
    if (const int row = cardRowAt (p); row >= 0)
    {
        proc.requestMelody (row);
        if (auto* prm = proc.apvts.getParameter ("melody"))
        {
            prm->beginChangeGesture();
            prm->setValueNotifyingHost (prm->convertTo0to1 ((float) row));
            prm->endChangeGesture();
        }
        return;
    }
    if (p.toFloat().getDistanceFrom (keyC) < 34.0f)
    {
        proc.requestWind();
        keyTarget += juce::MathConstants<float>::pi;
        return;
    }
    const auto d = p.toFloat() - mirrorC;
    if ((d.x * d.x) / (mirrorRx * mirrorRx) + (d.y * d.y) / (mirrorRy * mirrorRy) < 1.25f)
        playButton.setToggleState (! playButton.getToggleState(), juce::sendNotificationSync);
}

void BailarinaEditor::mouseMove (const juce::MouseEvent& e)
{
    hoverRow = cardRowAt (e.getPosition());
    const bool hot = hoverRow >= 0 || e.position.getDistanceFrom (keyC) < 34.0f;
    setMouseCursor (hot ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
}

void BailarinaEditor::mouseExit (const juce::MouseEvent&) { hoverRow = -1; }

void BailarinaEditor::scrollCard (int delta)
{
    cardScroll = juce::jlimit (0, juce::jmax (0, proc.numMelodies() - kCardRows), cardScroll + delta);
}

void BailarinaEditor::mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& w)
{
    if (cardArea.contains (e.position) && w.deltaY != 0.0f)
    {
        scrollCard (w.deltaY > 0.0f ? -2 : 2);
        hoverRow = cardRowAt (e.getPosition());
    }
}
