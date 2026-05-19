#include "PluginEditor.h"
#include "BinaryData.h"

#include <cmath>
#include <initializer_list>

namespace
{
namespace Theme
{
const juce::Colour background { 0xff061619 };
const juce::Colour panel { 0xff071d21 };
const juce::Colour panelDeep { 0xff020b0d };
const juce::Colour stroke { 0xff16434a };
const juce::Colour cyan { 0xff2de6ef };
const juce::Colour blue { 0xff2b61d6 };
const juce::Colour purple { 0xff8e45ee };
const juce::Colour text { 0xffd9d8ce };
const juce::Colour mutedText { 0xffaeb6ad };
const juce::Colour labelText { 0xffb3a69e };
const juce::Colour valueText { 0xffbeb1a8 };

constexpr int designWidth = 1672;
constexpr int designHeight = 941;
constexpr int editorWidth = 1208;
constexpr int editorHeight = 680;
constexpr float scaleX = (float) editorWidth / (float) designWidth;
constexpr float scaleY = (float) editorHeight / (float) designHeight;

juce::Colour layerColour(int layerIndex)
{
    static const juce::Colour colours[] = {
        juce::Colour(0xff7e35f2), juce::Colour(0xff8741f5),
        juce::Colour(0xff6262f0), juce::Colour(0xff2b61d6),
        juce::Colour(0xff2586ea), juce::Colour(0xff20b5e8),
        juce::Colour(0xff36dde9), juce::Colour(0xffb8f4f0)
    };
    return colours[(size_t) juce::jlimit(0, 7, layerIndex)];
}

juce::Colour layerHighlightColour(int layerIndex)
{
    static const juce::Colour colours[] = {
        juce::Colour(0xffb18aff), juce::Colour(0xffb99bff),
        juce::Colour(0xff9aa9ff), juce::Colour(0xff7ea8ff),
        juce::Colour(0xff78beff), juce::Colour(0xff81ddf7),
        juce::Colour(0xffa0f6f7), juce::Colour(0xffe7fffa)
    };
    return colours[(size_t) juce::jlimit(0, 7, layerIndex)];
}

void fillVerticalGradient(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour top, juce::Colour bottom)
{
    juce::ColourGradient gradient(top, bounds.getCentreX(), bounds.getY(), bottom, bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, 10.0f);
}
}

juce::Image getFigmaUiImage()
{
    return juce::ImageCache::getFromMemory(BinaryData::atmocycle_figma_ui_png,
                                           BinaryData::atmocycle_figma_ui_pngSize);
}

void makeHitZoneOnly(juce::Component& component)
{
    component.setAlpha(0.01f);
}

int scaledX(float value)
{
    return (int) std::round(value * Theme::scaleX);
}

int scaledY(float value)
{
    return (int) std::round(value * Theme::scaleY);
}

float scaledFont(float value)
{
    return value * Theme::scaleY;
}

juce::Rectangle<int> scaledBounds(float x, float y, float width, float height)
{
    return { scaledX(x), scaledY(y), scaledX(width), scaledY(height) };
}

juce::Rectangle<float> scaledBoundsF(float x, float y, float width, float height)
{
    return { x * Theme::scaleX, y * Theme::scaleY, width * Theme::scaleX, height * Theme::scaleY };
}

void drawFolderIcon(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour accent)
{
    bounds = bounds.reduced(0.5f);
    juce::Path folder;
    folder.startNewSubPath(bounds.getX(), bounds.getY() + bounds.getHeight() * 0.36f);
    folder.lineTo(bounds.getX() + bounds.getWidth() * 0.34f, bounds.getY() + bounds.getHeight() * 0.36f);
    folder.lineTo(bounds.getX() + bounds.getWidth() * 0.43f, bounds.getY() + bounds.getHeight() * 0.22f);
    folder.lineTo(bounds.getX() + bounds.getWidth() * 0.73f, bounds.getY() + bounds.getHeight() * 0.22f);
    folder.lineTo(bounds.getX() + bounds.getWidth() * 0.80f, bounds.getY() + bounds.getHeight() * 0.36f);
    folder.lineTo(bounds.getRight(), bounds.getY() + bounds.getHeight() * 0.36f);
    folder.lineTo(bounds.getRight(), bounds.getBottom() - 0.5f);
    folder.lineTo(bounds.getX(), bounds.getBottom() - 0.5f);
    folder.lineTo(bounds.getX(), bounds.getBottom());
    folder.closeSubPath();

    g.setColour(accent.withAlpha(0.12f));
    g.fillPath(folder);
    g.setColour(accent.withAlpha(0.84f));
    g.strokePath(folder, juce::PathStrokeType(1.25f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void drawDiskIcon(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour accent)
{
    bounds = bounds.reduced(0.5f);
    g.setColour(accent.withAlpha(0.12f));
    g.fillRoundedRectangle(bounds, 1.8f);
    g.setColour(accent.withAlpha(0.84f));
    g.drawRoundedRectangle(bounds, 1.8f, 1.25f);

    auto notch = bounds.withTrimmedLeft(bounds.getWidth() * 0.58f)
                       .withTrimmedRight(bounds.getWidth() * 0.15f)
                       .withTrimmedBottom(bounds.getHeight() * 0.62f);
    g.drawRoundedRectangle(notch, 0.8f, 1.0f);

    const auto lineLeft = bounds.getX() + bounds.getWidth() * 0.24f;
    const auto lineRight = bounds.getRight() - bounds.getWidth() * 0.20f;
    g.drawLine(lineLeft, bounds.getBottom() - bounds.getHeight() * 0.27f,
               lineRight, bounds.getBottom() - bounds.getHeight() * 0.27f, 1.1f);
    g.drawLine(lineLeft, bounds.getBottom() - bounds.getHeight() * 0.39f,
               lineRight, bounds.getBottom() - bounds.getHeight() * 0.39f, 1.1f);
}

void drawTinyFileBadge(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour accent)
{
    juce::ColourGradient fill(accent.withAlpha(0.78f), bounds.getX(), bounds.getY(),
                              Theme::panelDeep.withAlpha(0.94f), bounds.getRight(), bounds.getBottom(), false);
    g.setGradientFill(fill);
    g.fillRoundedRectangle(bounds, 1.6f);
    g.setColour(accent.withAlpha(0.76f));
    g.drawRoundedRectangle(bounds, 1.6f, 0.8f);

    juce::Path mark;
    const auto c = bounds.getCentre();
    mark.startNewSubPath(c.x - bounds.getWidth() * 0.17f, c.y - bounds.getHeight() * 0.18f);
    mark.lineTo(c.x + bounds.getWidth() * 0.18f, c.y);
    mark.lineTo(c.x - bounds.getWidth() * 0.17f, c.y + bounds.getHeight() * 0.18f);
    mark.closeSubPath();
    g.setColour(Theme::text.withAlpha(0.82f));
    g.fillPath(mark);
}

void drawDiceIcon(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    const auto cyan = Theme::cyan.withAlpha(0.90f);
    const auto purple = Theme::purple.withAlpha(0.88f);
    juce::Path front;
    juce::Path top;
    juce::Path side;

    const auto x = bounds.getX();
    const auto y = bounds.getY();
    const auto w = bounds.getWidth();
    const auto h = bounds.getHeight();

    front.startNewSubPath(x + w * 0.28f, y + h * 0.32f);
    front.lineTo(x + w * 0.62f, y + h * 0.44f);
    front.lineTo(x + w * 0.62f, y + h * 0.82f);
    front.lineTo(x + w * 0.28f, y + h * 0.66f);
    front.closeSubPath();

    top.startNewSubPath(x + w * 0.28f, y + h * 0.32f);
    top.lineTo(x + w * 0.52f, y + h * 0.14f);
    top.lineTo(x + w * 0.84f, y + h * 0.28f);
    top.lineTo(x + w * 0.62f, y + h * 0.44f);
    top.closeSubPath();

    side.startNewSubPath(x + w * 0.62f, y + h * 0.44f);
    side.lineTo(x + w * 0.84f, y + h * 0.28f);
    side.lineTo(x + w * 0.84f, y + h * 0.64f);
    side.lineTo(x + w * 0.62f, y + h * 0.82f);
    side.closeSubPath();

    g.setColour(Theme::cyan.withAlpha(0.09f));
    g.fillPath(front);
    g.fillPath(top);
    g.fillPath(side);
    g.setColour(cyan);
    g.strokePath(front, juce::PathStrokeType(1.0f));
    g.strokePath(top, juce::PathStrokeType(1.0f));
    g.setColour(purple);
    g.strokePath(side, juce::PathStrokeType(1.0f));

    auto dot = [&g] (float cx, float cy, float r, juce::Colour c)
    {
        g.setColour(c);
        g.fillEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f);
    };

    dot(x + w * 0.42f, y + h * 0.52f, 1.7f, Theme::text.withAlpha(0.80f));
    dot(x + w * 0.52f, y + h * 0.68f, 1.7f, Theme::text.withAlpha(0.80f));
    dot(x + w * 0.55f, y + h * 0.26f, 1.6f, Theme::text.withAlpha(0.82f));
    dot(x + w * 0.72f, y + h * 0.33f, 1.6f, Theme::text.withAlpha(0.82f));
    dot(x + w * 0.75f, y + h * 0.49f, 1.5f, Theme::text.withAlpha(0.74f));
    dot(x + w * 0.72f, y + h * 0.63f, 1.5f, Theme::text.withAlpha(0.74f));
}

void drawTopButtonContent(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& text, bool save, bool highlighted)
{
    const auto area = bounds.toFloat();
    const auto accent = Theme::purple.interpolatedWith(Theme::cyan, save ? 0.14f : 0.34f);
    const auto radius = scaledY(7.0f);

    juce::ColourGradient fill(juce::Colour(0xff071c21).withAlpha(highlighted ? 0.68f : 0.38f),
                              area.getX(), area.getY(),
                              juce::Colour(0xff02080a).withAlpha(highlighted ? 0.76f : 0.50f),
                              area.getRight(), area.getBottom(), false);
    g.setGradientFill(fill);
    g.fillRoundedRectangle(area.reduced(1.0f), radius);
    g.setColour(accent.withAlpha(highlighted ? 0.48f : 0.25f));
    g.drawRoundedRectangle(area.reduced(1.0f), radius, highlighted ? 1.15f : 0.75f);

    const auto iconSize = juce::jmin(area.getHeight() * 0.36f, area.getWidth() * 0.20f);
    const auto icon = juce::Rectangle<float>(area.getX() + area.getWidth() * 0.16f,
                                             area.getCentreY() - iconSize * 0.5f,
                                             iconSize * 1.22f,
                                             iconSize);

    if (save)
        drawDiskIcon(g, icon, accent);
    else
        drawFolderIcon(g, icon, accent);

    g.setColour(Theme::labelText.withAlpha(highlighted ? 0.96f : 0.84f));
    g.setFont(juce::Font("Avenir Next", scaledFont(14.2f), juce::Font::plain));
    g.drawFittedText(text, bounds.withTrimmedLeft(scaledX(50)).reduced(scaledX(4), 0),
                     juce::Justification::centredLeft, 1);
}

void drawTextValue(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& text,
                   float fontSize = 14.0f, juce::Justification justification = juce::Justification::centredLeft)
{
    g.setColour(Theme::valueText.withAlpha(0.92f));
    g.setFont(juce::Font("Avenir Next", fontSize, juce::Font::plain));
    g.drawFittedText(text, bounds.reduced(6, 0), justification, 1);
}

float trackedTextWidth(const juce::Font& font, const juce::String& text, float tracking)
{
    float width = 0.0f;
    for (int i = 0; i < text.length(); ++i)
        width += font.getStringWidthFloat(text.substring(i, i + 1));

    return width + juce::jmax(0, text.length() - 1) * tracking;
}

void drawTrackedText(juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& text,
                     float fontSize, juce::Justification justification = juce::Justification::centred,
                     float tracking = 1.2f, juce::Colour colour = Theme::labelText, bool shadow = true)
{
    auto font = juce::Font("Avenir Next", fontSize, juce::Font::plain);
    const auto textWidth = trackedTextWidth(font, text, tracking);
    auto x = bounds.getX();

    if (justification.testFlags(juce::Justification::horizontallyCentred))
        x = bounds.getCentreX() - textWidth * 0.5f;
    else if (justification.testFlags(juce::Justification::right))
        x = bounds.getRight() - textWidth;

    const auto y = bounds.getCentreY() - font.getHeight() * 0.5f;
    auto draw = [&] (juce::Colour c, float dx, float dy)
    {
        g.setColour(c);
        g.setFont(font);
        auto cx = x + dx;
        for (int i = 0; i < text.length(); ++i)
        {
            const auto ch = text.substring(i, i + 1);
            g.drawSingleLineText(ch, (int) std::round(cx), (int) std::round(y + dy + font.getAscent()));
            cx += font.getStringWidthFloat(ch) + tracking;
        }
    };

    if (shadow)
        draw(juce::Colours::black.withAlpha(0.32f), 0.0f, 1.0f);

    draw(colour, 0.0f, 0.0f);
}

void drawUiLabel(juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& text,
                 float fontSize = scaledFont(11.5f), juce::Justification justification = juce::Justification::centred,
                 float tracking = scaledX(1.05f))
{
    drawTrackedText(g, bounds, text, fontSize, justification, tracking, Theme::labelText.withAlpha(0.96f), true);
}

juce::String sliderValueText(juce::Slider& slider)
{
    return slider.getTextFromValue(slider.getValue());
}

juce::String levelToDbText(float level)
{
    if (level <= 0.00001f)
        return "-inf dB";

    return juce::String(juce::Decibels::gainToDecibels(level), 1) + " dB";
}

juce::String formatTime(double seconds, bool countdown)
{
    if (seconds < 0.0)
        return "--:--";

    auto totalSeconds = (int) (countdown ? std::ceil(seconds) : std::floor(seconds + 0.5));
    totalSeconds = juce::jmax(0, totalSeconds);

    const int hours = totalSeconds / 3600;
    const int minutes = (totalSeconds / 60) % 60;
    const int secs = totalSeconds % 60;

    if (hours > 0)
        return juce::String::formatted("%02d:%02d:%02d", hours, minutes, secs);

    return juce::String::formatted("%02d:%02d", minutes, secs);
}

class AtmocycleLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AtmocycleLookAndFeel()
    {
        setColour(juce::Slider::textBoxTextColourId, Theme::text);
        setColour(juce::Slider::textBoxOutlineColourId, Theme::stroke.withAlpha(0.75f));
        setColour(juce::Slider::textBoxBackgroundColourId, Theme::panelDeep);
        setColour(juce::TextButton::buttonColourId, Theme::panel.withMultipliedBrightness(1.2f));
        setColour(juce::TextButton::buttonOnColourId, Theme::purple);
        setColour(juce::TextButton::textColourOffId, Theme::text);
        setColour(juce::TextButton::textColourOnId, Theme::text);
        setColour(juce::ToggleButton::textColourId, Theme::text);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override
    {
        const auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) width, (float) height).reduced(2.0f);
        const auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
        const auto r = bounds.withSizeKeepingCentre(size, size).reduced(1.5f);
        const auto radius = r.getWidth() * 0.5f;
        const auto centre = r.getCentre();
        const auto accent = slider.findColour(juce::Slider::rotarySliderFillColourId);
        const bool invertGlow = slider.getName().containsIgnoreCase("HighCut");
        const auto displayPos = juce::jlimit(0.0f, 1.0f, invertGlow ? 1.0f - sliderPos : sliderPos);
        const auto glow = displayPos;

        const auto innerGlow = 0.8f + glow * 0.45f;
        const auto outerGlow = 1.4f + glow * 0.35f;
        g.setColour(accent.withAlpha(0.035f + glow * 0.045f));
        g.fillEllipse(r.expanded(innerGlow));
        g.setColour(accent.withAlpha(0.012f + glow * 0.025f));
        g.fillEllipse(r.expanded(outerGlow));

        g.setColour(juce::Colours::black.withAlpha(0.58f));
        g.fillEllipse(r.translated(0.0f, 3.0f));

        juce::ColourGradient knobGradient(juce::Colour(0xff14282d), centre.x - radius * 0.38f, centre.y - radius * 0.44f,
                                          juce::Colour(0xff000203), centre.x + radius * 0.42f, centre.y + radius * 0.52f, true);
        knobGradient.addColour(0.55, juce::Colour(0xff061115));
        g.setGradientFill(knobGradient);
        g.fillEllipse(r);

        g.setColour(juce::Colours::white.withAlpha(0.10f));
        g.fillEllipse(r.reduced(radius * 0.24f).withTrimmedBottom(radius * 0.55f).translated(-radius * 0.10f, -radius * 0.10f));

        g.setColour(accent.withAlpha(0.12f + glow * 0.12f));
        g.drawEllipse(r.expanded(1.4f), 1.0f);
        g.setColour(Theme::stroke.withAlpha(0.48f));
        g.drawEllipse(r, 0.8f);

        juce::Path track;
        track.addCentredArc(centre.x, centre.y, radius - 3.4f, radius - 3.4f, 0.0f,
                            rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colours::black.withAlpha(0.42f));
        g.strokePath(track, juce::PathStrokeType(3.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        juce::Path value;
        value.addCentredArc(centre.x, centre.y, radius - 3.4f, radius - 3.4f, 0.0f,
                            rotaryStartAngle, rotaryStartAngle + displayPos * (rotaryEndAngle - rotaryStartAngle), true);
        g.setColour(accent.withAlpha(0.035f + glow * 0.055f));
        g.strokePath(value, juce::PathStrokeType(4.5f + glow * 0.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(accent.withAlpha(0.32f + glow * 0.22f));
        g.strokePath(value, juce::PathStrokeType(1.7f + glow * 0.3f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        const auto indicator = centre + juce::Point<float>(std::cos(angle - juce::MathConstants<float>::halfPi),
                                                           std::sin(angle - juce::MathConstants<float>::halfPi)) * (radius - 9.0f);
        g.setColour(accent.withAlpha(0.34f));
        g.fillEllipse(indicator.x - 3.6f, indicator.y - 3.6f, 7.2f, 7.2f);
        g.setColour(Theme::text.withAlpha(0.86f));
        g.fillEllipse(indicator.x - 1.8f, indicator.y - 1.8f, 3.6f, 3.6f);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          float, float, const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        if (style != juce::Slider::LinearHorizontal)
        {
            juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, 0.0f, 0.0f, style, slider);
            return;
        }

        const auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) width, (float) height).reduced(5.0f, 8.0f);
        const auto cy = bounds.getCentreY();
        const auto accent = slider.findColour(juce::Slider::trackColourId);
        const auto track = bounds.withHeight(6.0f).withCentre(juce::Point<float>(bounds.getCentreX(), cy));
        const auto shell = track.expanded(2.0f, 2.2f);

        g.setColour(juce::Colours::black.withAlpha(0.34f));
        g.fillRoundedRectangle(shell.translated(0.0f, 1.2f), 4.5f);

        juce::ColourGradient trough(juce::Colour(0xff061316), track.getX(), track.getY(),
                                    juce::Colour(0xff010607), track.getX(), track.getBottom(), false);
        trough.addColour(0.38, juce::Colour(0xff102b31).withAlpha(0.80f));
        g.setGradientFill(trough);
        g.fillRoundedRectangle(shell, 4.5f);

        g.setColour(juce::Colours::white.withAlpha(0.10f));
        g.fillRoundedRectangle(shell.withTrimmedBottom(shell.getHeight() * 0.55f).reduced(1.6f, 1.2f), 3.0f);
        g.setColour(Theme::stroke.withAlpha(0.28f));
        g.drawRoundedRectangle(shell, 4.5f, 0.7f);

        const auto fillWidth = juce::jmax(0.0f, sliderPos - bounds.getX());
        juce::ColourGradient fill(accent.withAlpha(0.96f), bounds.getX(), cy,
                                  Theme::cyan.withAlpha(0.96f), sliderPos, cy, false);
        fill.addColour(0.45, Theme::blue.withAlpha(0.92f));
        g.setGradientFill(fill);
        g.fillRoundedRectangle(juce::Rectangle<float>(bounds.getX(), cy - 2.4f, fillWidth, 4.8f), 2.4f);

        g.setColour(accent.withAlpha(0.12f));
        g.fillRoundedRectangle(juce::Rectangle<float>(bounds.getX(), cy - 4.6f, fillWidth, 9.2f), 4.6f);
        g.setColour(juce::Colours::white.withAlpha(0.22f));
        g.fillRoundedRectangle(juce::Rectangle<float>(bounds.getX() + 1.0f, cy - 2.5f, juce::jmax(0.0f, fillWidth - 2.0f), 1.2f), 0.6f);

        const auto thumbArea = juce::Rectangle<float>(sliderPos - 4.0f, cy - 10.0f, 8.0f, 20.0f);
        g.setColour(accent.withAlpha(0.16f));
        g.fillRoundedRectangle(thumbArea.expanded(2.2f, 1.6f), 3.4f);

        juce::ColourGradient thumb(juce::Colour(0xffeef4ef), thumbArea.getX(), thumbArea.getY(),
                                   juce::Colour(0xff536265), thumbArea.getRight(), thumbArea.getBottom(), false);
        thumb.addColour(0.32, juce::Colour(0xffc9d5d3));
        thumb.addColour(0.62, juce::Colour(0xff6b7779));
        g.setGradientFill(thumb);
        g.fillRoundedRectangle(thumbArea, 2.4f);

        g.setColour(juce::Colours::white.withAlpha(0.30f));
        g.drawLine(thumbArea.getX() + 2.2f, thumbArea.getY() + 3.0f,
                   thumbArea.getX() + 2.2f, thumbArea.getBottom() - 3.0f, 0.7f);
        g.setColour(juce::Colours::black.withAlpha(0.36f));
        g.drawRoundedRectangle(thumbArea, 2.4f, 0.7f);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&, bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
        if (button.getButtonText().equalsIgnoreCase("Load"))
        {
            const auto active = shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;
            const auto radius = 5.0f;
            const auto accent = Theme::purple.interpolatedWith(Theme::cyan, 0.25f);

            g.setColour(juce::Colours::black.withAlpha(0.22f));
            g.fillRoundedRectangle(bounds.translated(0.0f, 1.0f), radius);

            juce::ColourGradient fill(juce::Colour(0xff102b31).withAlpha(active ? 0.86f : 0.66f),
                                      bounds.getX(), bounds.getY(),
                                      juce::Colour(0xff020709).withAlpha(active ? 0.94f : 0.80f),
                                      bounds.getRight(), bounds.getBottom(), false);
            g.setGradientFill(fill);
            g.fillRoundedRectangle(bounds, radius);

            g.setColour(juce::Colours::white.withAlpha(active ? 0.08f : 0.04f));
            g.fillRoundedRectangle(bounds.reduced(1.5f).withTrimmedBottom(bounds.getHeight() * 0.58f), radius - 1.2f);
            g.setColour(accent.withAlpha(active ? 0.50f : 0.28f));
            g.drawRoundedRectangle(bounds, radius, active ? 1.05f : 0.8f);
            g.setColour(Theme::text.withAlpha(active ? 0.92f : 0.78f));
            g.setFont(juce::Font(9.8f, juce::Font::plain));
            g.drawFittedText("LOAD", button.getLocalBounds().reduced(5, 2), juce::Justification::centred, 1);
            return;
        }

        auto base = shouldDrawButtonAsDown ? Theme::purple.withAlpha(0.45f) : Theme::panel;
        if (shouldDrawButtonAsHighlighted)
            base = base.brighter(0.12f);

        Theme::fillVerticalGradient(g, bounds, base.brighter(0.18f), Theme::panelDeep);
        g.setColour(Theme::cyan.withAlpha(0.06f));
        g.fillRoundedRectangle(bounds.reduced(2.0f).withTrimmedBottom(bounds.getHeight() * 0.55f), 5.0f);
        g.setColour((button.getToggleState() ? Theme::purple : Theme::stroke).withAlpha(0.95f));
        g.drawRoundedRectangle(bounds, 6.0f, 1.1f);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool) override
    {
        const auto isLoad = button.getButtonText().equalsIgnoreCase("Load");
        if (isLoad)
            return;

        g.setColour(Theme::text.withAlpha(button.isEnabled() ? (isLoad ? 0.90f : 0.84f) : 0.35f));
        g.setFont(juce::Font(isLoad ? 10.0f : 9.5f));
        g.drawFittedText(button.getButtonText(), button.getLocalBounds().reduced(5, 2),
                         juce::Justification::centred, 1);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool highlighted, bool down) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
        const auto on = button.getToggleState();
        if (button.getButtonText() == "On")
        {
            auto circle = bounds.withSizeKeepingCentre(22.0f, 22.0f);
            const auto accent = on ? Theme::purple : Theme::stroke;

            g.setColour(juce::Colours::black.withAlpha(0.34f));
            g.fillEllipse(circle.translated(0.0f, 1.2f));

            juce::ColourGradient fill(juce::Colour(0xff143039), circle.getX(), circle.getY(),
                                      juce::Colour(0xff020507), circle.getRight(), circle.getBottom(), true);
            fill.addColour(0.55, juce::Colour(0xff071219));
            g.setGradientFill(fill);
            g.fillEllipse(circle);

            if (on)
            {
                g.setColour(accent.withAlpha(0.12f));
                g.fillEllipse(circle.reduced(1.4f));
            }

            g.setColour(juce::Colours::white.withAlpha(0.10f));
            g.fillEllipse(circle.reduced(6.0f).withTrimmedBottom(circle.getHeight() * 0.62f).translated(-2.0f, -1.5f));
            g.setColour(accent.withAlpha(on ? 0.68f : 0.30f));
            g.drawEllipse(circle.reduced(0.8f), on ? 1.1f : 0.8f);

            const auto centre = circle.getCentre();
            juce::Path powerArc;
            powerArc.addCentredArc(centre.x, centre.y + 1.0f, 7.2f, 7.2f, 0.0f,
                                   -2.35f, 2.35f, true);
            g.setColour((on ? Theme::text : Theme::mutedText).withAlpha(on ? 0.94f : 0.62f));
            g.strokePath(powerArc, juce::PathStrokeType(2.25f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.drawLine(centre.x, centre.y - 9.0f, centre.x, centre.y - 1.5f, 2.25f);
            return;
        }

        if (button.getButtonText() == "S")
        {
            auto box = bounds.reduced(1.0f).withSizeKeepingCentre(22.0f, 22.0f);
            const auto accent = on ? Theme::purple : Theme::stroke;

            if (on)
            {
                g.setColour(Theme::purple.withAlpha(0.10f));
                g.fillRoundedRectangle(box.reduced(1.0f), 5.0f);
            }

            juce::ColourGradient fill(on ? juce::Colour(0xff17203d) : juce::Colour(0xff102b32),
                                      box.getX(), box.getY(),
                                      juce::Colour(0xff02070a), box.getRight(), box.getBottom(), false);
            g.setGradientFill(fill);
            g.fillRoundedRectangle(box, 5.5f);
            g.setColour(accent.withAlpha(on ? 0.70f : 0.46f));
            g.drawRoundedRectangle(box, 5.5f, 1.0f);
            g.setColour((on ? Theme::text : Theme::mutedText).withAlpha(on ? 0.95f : 0.70f));
            g.setFont(juce::Font(15.0f, juce::Font::plain));
            g.drawFittedText("S", box.toNearestInt().reduced(3), juce::Justification::centred, 1);
            return;
        }

        if (button.getButtonText() == "AP")
        {
            auto circle = bounds.withSizeKeepingCentre(22.0f, 22.0f);
            const auto accent = on ? Theme::cyan : Theme::stroke;

            g.setColour(juce::Colours::black.withAlpha(0.34f));
            g.fillEllipse(circle.translated(0.0f, 1.2f));

            juce::ColourGradient fill(juce::Colour(0xff123039), circle.getX(), circle.getY(),
                                      juce::Colour(0xff020609), circle.getRight(), circle.getBottom(), true);
            g.setGradientFill(fill);
            g.fillEllipse(circle);

            if (on)
            {
                g.setColour(accent.withAlpha(0.10f));
                g.fillEllipse(circle.reduced(1.4f));
            }

            g.setColour(accent.withAlpha(on ? 0.70f : 0.30f));
            g.drawEllipse(circle.reduced(0.8f), on ? 1.05f : 0.8f);

            juce::Path wave;
            const auto waveArea = circle.reduced(6.0f, 8.0f);
            for (int i = 0; i < 24; ++i)
            {
                const float t = (float) i / 23.0f;
                const float xx = waveArea.getX() + t * waveArea.getWidth();
                const float yy = waveArea.getCentreY() + std::sin(t * juce::MathConstants<float>::twoPi) * waveArea.getHeight() * 0.36f;
                if (i == 0)
                    wave.startNewSubPath(xx, yy);
                else
                    wave.lineTo(xx, yy);
            }
            g.setColour((on ? Theme::cyan : Theme::mutedText).withAlpha(on ? 0.94f : 0.62f));
            g.strokePath(wave, juce::PathStrokeType(1.9f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            return;
        }

        auto base = on ? Theme::panel.brighter(0.08f) : Theme::panelDeep;
        if (highlighted || down)
            base = base.brighter(0.12f);

        juce::ColourGradient gradient(base.brighter(0.12f), bounds.getX(), bounds.getY(),
                                      Theme::panelDeep, bounds.getRight(), bounds.getBottom(), false);
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(bounds, 5.0f);
        g.setColour((on ? Theme::cyan : Theme::stroke).withAlpha(on ? 0.58f : 0.38f));
        g.drawRoundedRectangle(bounds, 5.0f, 1.0f);
        g.setColour(Theme::text.withAlpha(on ? 0.86f : 0.62f));
        g.setFont(juce::Font(9.2f));
        g.drawFittedText(button.getButtonText(), button.getLocalBounds().reduced(2), juce::Justification::centred, 1);
    }
};
}

SceneLooperAudioProcessorEditor::SceneLooperAudioProcessorEditor(SceneLooperAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    lookAndFeel = std::make_unique<AtmocycleLookAndFeel>();
    setLookAndFeel(lookAndFeel.get());
    tooltipWindow = std::make_unique<juce::TooltipWindow>(this, 850);

    setResizable(false, false);
    setResizeLimits(Theme::editorWidth, Theme::editorHeight, Theme::editorWidth, Theme::editorHeight);

    titleLabel.setText("Atmocycle", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(22.0f, juce::Font::plain));
    titleLabel.setColour(juce::Label::textColourId, Theme::text);
    bylineLabel.setText("by Echosynthesis", juce::dontSendNotification);
    bylineLabel.setFont(juce::Font(10.5f, juce::Font::plain));
    bylineLabel.setColour(juce::Label::textColourId, Theme::text.withAlpha(0.76f));
    taglineLabel.setText("Multi-Layer Ambience Engine", juce::dontSendNotification);
    taglineLabel.setFont(juce::Font(10.0f, juce::Font::plain));
    taglineLabel.setColour(juce::Label::textColourId, Theme::purple);
    sceneCaptionLabel.setText("SCENE", juce::dontSendNotification);
    sceneCaptionLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    sceneCaptionLabel.setJustificationType(juce::Justification::centred);
    sceneCaptionLabel.setColour(juce::Label::textColourId, Theme::mutedText);
    auto sceneTitle = processor.getCurrentSceneName();
    sceneNameLabel.setText(sceneTitle == "Untitled Scene" ? "Project State" : sceneTitle, juce::dontSendNotification);
    sceneNameLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    sceneNameLabel.setJustificationType(juce::Justification::centred);
    sceneNameLabel.setColour(juce::Label::textColourId, Theme::text);
    masterLabel.setText("MASTER OUTPUT", juce::dontSendNotification);
    globalXFadeLabel.setText("GLOBAL CROSSFADE", juce::dontSendNotification);
    masterLowCutLabel.setText("MASTER LOW CUT", juce::dontSendNotification);
    masterHighCutLabel.setText("MASTER HIGH CUT", juce::dontSendNotification);
    randomizationLabel.setText("RANDOMIZATION", juce::dontSendNotification);
    randomStartLabel.setText("RANDOM START", juce::dontSendNotification);
    masterMeterLabel.setText("MASTER METER", juce::dontSendNotification);

    for (auto* label : { &masterLabel, &globalXFadeLabel, &masterLowCutLabel, &masterHighCutLabel,
                         &randomizationLabel, &randomStartLabel, &masterMeterLabel })
    {
        label->setFont(juce::Font(9.0f, juce::Font::plain));
        label->setColour(juce::Label::textColourId, Theme::mutedText);
        label->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*label);
    }

    addAndMakeVisible(titleLabel);
    addAndMakeVisible(bylineLabel);
    addAndMakeVisible(taglineLabel);
    addAndMakeVisible(sceneCaptionLabel);
    addAndMakeVisible(sceneNameLabel);
    addAndMakeVisible(saveSceneButton);
    addAndMakeVisible(loadSceneButton);
    loadSceneButton.setTooltip("Load Scene: open a saved Atmocycle scene file.");
    saveSceneButton.setTooltip("Save Scene: save the current layer setup as a scene file.");

    auto setupMacro = [] (juce::Slider& s, const juce::String& suffix)
    {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        s.setTextValueSuffix(suffix);
        s.setColour(juce::Slider::rotarySliderFillColourId, Theme::purple);
    };

    setupMacro(masterSlider, " dB");
    setupMacro(globalXFadeSlider, " s");
    setupMacro(masterLowCutSlider, " Hz");
    setupMacro(masterHighCutSlider, "");
    setupMacro(randomStartSlider, "%");
    masterSlider.setName("MasterOutput");
    globalXFadeSlider.setName("GlobalCrossfade");
    masterLowCutSlider.setName("MasterLowCut");
    masterHighCutSlider.setName("MasterHighCut");
    randomStartSlider.setName("RandomStart");
    globalXFadeSlider.setColour(juce::Slider::rotarySliderFillColourId, Theme::cyan);
    masterLowCutSlider.setColour(juce::Slider::rotarySliderFillColourId, Theme::blue);
    masterHighCutSlider.setColour(juce::Slider::rotarySliderFillColourId, Theme::purple);
    randomStartSlider.setColour(juce::Slider::rotarySliderFillColourId, Theme::purple);
    masterHighCutSlider.textFromValueFunction = [] (double value)
    {
        return juce::String(value / 1000.0, 1) + " kHz";
    };
    masterHighCutSlider.valueFromTextFunction = [] (const juce::String& text)
    {
        return text.retainCharacters("0123456789.").getDoubleValue() * 1000.0;
    };
    masterLowCutSlider.textFromValueFunction = [] (double value)
    {
        return juce::String((int) std::round(value)) + " Hz";
    };
    addAndMakeVisible(masterSlider);
    addAndMakeVisible(globalXFadeSlider);
    addAndMakeVisible(masterLowCutSlider);
    addAndMakeVisible(masterHighCutSlider);
    addAndMakeVisible(randomStartSlider);
    addAndMakeVisible(randomizeButton);

    masterAttachment = std::make_unique<SliderAttachment>(processor.apvts, "masterVolume", masterSlider);
    globalXFadeAttachment = std::make_unique<SliderAttachment>(processor.apvts, "globalXFade", globalXFadeSlider);
    masterLowCutAttachment = std::make_unique<SliderAttachment>(processor.apvts, "masterLowCut", masterLowCutSlider);
    masterHighCutAttachment = std::make_unique<SliderAttachment>(processor.apvts, "masterHighCut", masterHighCutSlider);
    randomStartAttachment = std::make_unique<SliderAttachment>(processor.apvts, "randomStart", randomStartSlider);

    randomizeButton.onClick = [this]
    {
        processor.randomizeLayerStarts();
    };
    randomizeButton.setTooltip("Randomization: randomize layer start positions using the Random Start amount.");

    saveSceneButton.onClick = [this]
    {
        sceneFileChooser = std::make_unique<juce::FileChooser>("Save Scene", juce::File{}, "*.scene");
        sceneFileChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& chooser)
            {
                auto file = chooser.getResult();
                if (file == juce::File{})
                    return;

                if (file.getFileExtension() != ".scene")
                    file = file.withFileExtension(".scene");

                juce::String error;
                if (! processor.saveSceneToFile(file, error))
                {
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Atmocycle", error);
                }
                else
                {
                    processor.setCurrentSceneName(file.getFileNameWithoutExtension());
                    sceneNameLabel.setText(processor.getCurrentSceneName(), juce::dontSendNotification);
                }
            });
    };

    loadSceneButton.onClick = [this]
    {
        sceneFileChooser = std::make_unique<juce::FileChooser>("Load Scene", juce::File{}, "*.scene");
        sceneFileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& chooser)
            {
                const auto file = chooser.getResult();
                if (! file.existsAsFile())
                    return;

                juce::String error;
                if (! processor.loadSceneFromFile(file, error))
                {
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Atmocycle", error);
                }
                else
                {
                    processor.setCurrentSceneName(file.getFileNameWithoutExtension());
                    sceneNameLabel.setText(processor.getCurrentSceneName(), juce::dontSendNotification);
                }

                refreshLayerNames();
                refreshLayerTimes();
            });
    };

    for (int i = 0; i < SceneLooperAudioProcessor::numLayers; ++i)
    {
        rows[(size_t) i] = std::make_unique<LayerRow>(processor, i);
        addAndMakeVisible(*rows[(size_t) i]);
    }

    for (auto* label : { &titleLabel, &bylineLabel, &taglineLabel, &sceneCaptionLabel, &sceneNameLabel,
                         &masterLabel, &globalXFadeLabel, &masterLowCutLabel, &masterHighCutLabel,
                         &randomizationLabel, &randomStartLabel, &masterMeterLabel })
    {
        label->setVisible(false);
    }

    for (auto* component : { static_cast<juce::Component*>(&saveSceneButton),
                             static_cast<juce::Component*>(&loadSceneButton),
                             static_cast<juce::Component*>(&randomizeButton) })
    {
        makeHitZoneOnly(*component);
    }

    setSize(Theme::editorWidth, Theme::editorHeight);
    resized();
    startTimerHz(20);
}

SceneLooperAudioProcessorEditor::~SceneLooperAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void SceneLooperAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff000506));

    const auto figmaUi = getFigmaUiImage();
    if (figmaUi.isValid())
    {
        g.drawImage(figmaUi, getLocalBounds().toFloat(), juce::RectanglePlacement::stretchToFit, false);
        return;
    }

    auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient fallback(juce::Colour(0xff001014), bounds.getX(), bounds.getY(),
                                  juce::Colour(0xff000304), bounds.getRight(), bounds.getBottom(), false);
    g.setGradientFill(fallback);
    g.fillRect(bounds);
}

void SceneLooperAudioProcessorEditor::paintOverChildren(juce::Graphics& g)
{
    drawTrackedText(g, scaledBoundsF(132.0f, 18.0f, 300.0f, 35.0f), "Atmocycle", scaledFont(30.0f),
                    juce::Justification::centredLeft, scaledX(1.6f), juce::Colour(0xffddd7cf), true);
    drawTrackedText(g, scaledBoundsF(132.0f, 50.0f, 220.0f, 18.0f), "by Echosynthesis", scaledFont(11.0f),
                    juce::Justification::centredLeft, scaledX(1.9f), Theme::labelText.withAlpha(0.86f), true);
    drawTrackedText(g, scaledBoundsF(132.0f, 75.0f, 305.0f, 18.0f), "MULTI-LAYER AMBIENCE PLAYER", scaledFont(10.6f),
                    juce::Justification::centredLeft, scaledX(2.0f), Theme::purple.withAlpha(0.88f), true);

    drawUiLabel(g, scaledBoundsF(728.0f, 13.0f, 70.0f, 17.0f), "SCENE", scaledFont(10.0f));
    drawTopButtonContent(g, scaledBounds(986.0f, 31.0f, 128.0f, 53.0f), "LOAD SCENE", false, loadSceneButton.isMouseOver());
    drawTopButtonContent(g, scaledBounds(1147.0f, 31.0f, 128.0f, 53.0f), "SAVE SCENE", true, saveSceneButton.isMouseOver());

    const auto sceneName = processor.getCurrentSceneName() == "Untitled Scene"
                               ? juce::String("Project State")
                               : processor.getCurrentSceneName();

    g.setColour(Theme::valueText.withAlpha(0.90f));
    g.setFont(juce::Font("Avenir Next", scaledFont(15.0f), juce::Font::plain));
    g.drawFittedText(sceneName, sceneNameLabel.getBounds(), juce::Justification::centred, 1);

    drawUiLabel(g, scaledBoundsF(160.0f, 139.0f, 170.0f, 20.0f), "MASTER OUTPUT", scaledFont(12.0f), juce::Justification::centredLeft);
    drawUiLabel(g, scaledBoundsF(568.0f, 139.0f, 185.0f, 20.0f), "GLOBAL CROSSFADE", scaledFont(12.0f), juce::Justification::centredLeft);
    drawUiLabel(g, scaledBoundsF(972.0f, 139.0f, 175.0f, 20.0f), "MASTER LOW CUT", scaledFont(12.0f), juce::Justification::centredLeft);
    drawUiLabel(g, scaledBoundsF(1390.0f, 139.0f, 185.0f, 20.0f), "MASTER HIGH CUT", scaledFont(12.0f), juce::Justification::centredLeft);

    drawUiLabel(g, scaledBoundsF(31.0f, 246.0f, 65.0f, 20.0f), "LAYER", scaledFont(10.5f), juce::Justification::centredLeft);
    drawUiLabel(g, scaledBoundsF(128.0f, 246.0f, 170.0f, 20.0f), "FILE / LOOP", scaledFont(10.5f), juce::Justification::centredLeft);
    drawUiLabel(g, scaledBoundsF(395.0f, 246.0f, 100.0f, 20.0f), "CONTROL", scaledFont(10.5f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(580.0f, 246.0f, 80.0f, 20.0f), "VOLUME", scaledFont(10.5f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(716.0f, 246.0f, 60.0f, 20.0f), "PAN", scaledFont(10.5f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(815.0f, 238.0f, 90.0f, 15.0f), "AUTO PAN", scaledFont(10.0f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(815.0f, 253.0f, 90.0f, 15.0f), "AMOUNT", scaledFont(10.0f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(918.0f, 238.0f, 85.0f, 15.0f), "AUTO PAN", scaledFont(10.0f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(918.0f, 253.0f, 85.0f, 15.0f), "RATE", scaledFont(10.0f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(1018.0f, 246.0f, 70.0f, 20.0f), "SPEED", scaledFont(10.5f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(1112.0f, 246.0f, 65.0f, 20.0f), "DRIFT", scaledFont(10.5f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(1205.0f, 246.0f, 70.0f, 20.0f), "WIDTH", scaledFont(10.5f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(1297.0f, 246.0f, 110.0f, 20.0f), "START OFFSET", scaledFont(10.5f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(1430.0f, 246.0f, 45.0f, 20.0f), "HP", scaledFont(10.5f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(1510.0f, 246.0f, 45.0f, 20.0f), "LP", scaledFont(10.5f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(1581.0f, 246.0f, 75.0f, 20.0f), "XFADE", scaledFont(10.5f), juce::Justification::centred);

    drawTextValue(g, scaledBounds(151, 167, 157, 43), sliderValueText(masterSlider), scaledFont(15.2f), juce::Justification::centredLeft);
    drawTextValue(g, scaledBounds(563, 167, 157, 43), sliderValueText(globalXFadeSlider), scaledFont(15.2f), juce::Justification::centredLeft);
    drawTextValue(g, scaledBounds(973, 167, 157, 43), sliderValueText(masterLowCutSlider), scaledFont(15.2f), juce::Justification::centredLeft);
    drawTextValue(g, scaledBounds(1378, 167, 157, 43), sliderValueText(masterHighCutSlider), scaledFont(15.2f), juce::Justification::centredLeft);

    const auto randomSlot = scaledBounds(29, 879, 225, 43);
    drawDiceIcon(g, randomSlot.toFloat().withTrimmedLeft(scaledX(25)).withTrimmedRight(scaledX(160)).reduced(0.0f, scaledY(5.0f)));

    drawUiLabel(g, scaledBoundsF(77.0f, 855.0f, 170.0f, 20.0f), "RANDOMIZATION", scaledFont(10.4f), juce::Justification::centredLeft);
    drawUiLabel(g, scaledBoundsF(350.0f, 855.0f, 135.0f, 20.0f), "RANDOM START", scaledFont(10.4f), juce::Justification::centredLeft);
    drawUiLabel(g, scaledBoundsF(596.0f, 855.0f, 150.0f, 20.0f), "MASTER METER", scaledFont(10.4f), juce::Justification::centredLeft);

    drawTextValue(g, scaledBounds(337, 879, 127, 43), sliderValueText(randomStartSlider), scaledFont(13.8f), juce::Justification::centred);

    const auto masterLeft = juce::jlimit(0.0f, 1.0f, processor.getMasterLeftLevel());
    const auto masterRight = juce::jlimit(0.0f, 1.0f, processor.getMasterRightLevel());
    auto ledArea = scaledBoundsF(596.0f, 883.0f, 326.0f, 25.0f);

    constexpr int bars = 45;
    auto drawMeterRow = [&g, &ledArea] (float level, float y, float h)
    {
        for (int i = 0; i < bars; ++i)
        {
            const float t = (float) i / (float) (bars - 1);
            const bool active = t <= level;
            const auto colour = Theme::purple.interpolatedWith(Theme::cyan, t);
            const auto segmentWidth = ledArea.getWidth() / (float) bars;
            const auto x = ledArea.getX() + (float) i * segmentWidth;
            g.setColour(active ? colour.withAlpha(0.88f) : juce::Colour(0xff0b2a30).withAlpha(0.12f));
            g.fillRoundedRectangle(x, y, juce::jmax(2.4f, segmentWidth - 2.0f), h, 0.7f);
        }
    };

    drawMeterRow(masterLeft, ledArea.getY() + scaledY(1.0f), scaledY(7.0f));
    drawMeterRow(masterRight, ledArea.getY() + scaledY(12.0f), scaledY(7.0f));

    drawTextValue(g, scaledBounds(536, 883, 58, 26), levelToDbText(juce::jmax(masterLeft, masterRight)), scaledFont(12.5f), juce::Justification::centredRight);
    drawUiLabel(g, scaledBoundsF(593.0f, 909.0f, 34.0f, 15.0f), "-60", scaledFont(9.2f));
    drawUiLabel(g, scaledBoundsF(644.0f, 909.0f, 34.0f, 15.0f), "-48", scaledFont(9.2f));
    drawUiLabel(g, scaledBoundsF(695.0f, 909.0f, 34.0f, 15.0f), "-36", scaledFont(9.2f));
    drawUiLabel(g, scaledBoundsF(746.0f, 909.0f, 34.0f, 15.0f), "-24", scaledFont(9.2f));
    drawUiLabel(g, scaledBoundsF(797.0f, 909.0f, 34.0f, 15.0f), "-18", scaledFont(9.2f));
    drawUiLabel(g, scaledBoundsF(848.0f, 909.0f, 34.0f, 15.0f), "-12", scaledFont(9.2f));
    drawUiLabel(g, scaledBoundsF(899.0f, 909.0f, 34.0f, 15.0f), "-6", scaledFont(9.2f));
    drawUiLabel(g, scaledBoundsF(915.0f, 887.0f, 70.0f, 25.0f), "dB", scaledFont(12.4f), juce::Justification::centredRight);
}

void SceneLooperAudioProcessorEditor::resized()
{
    titleLabel.setBounds(scaledBounds(132, 21, 240, 27));
    bylineLabel.setBounds(scaledBounds(132, 51, 185, 17));
    taglineLabel.setBounds(scaledBounds(132, 76, 260, 18));
    sceneCaptionLabel.setBounds(scaledBounds(733, 13, 54, 18));
    sceneNameLabel.setBounds(scaledBounds(520, 28, 436, 60));
    loadSceneButton.setBounds(scaledBounds(986, 31, 128, 53));
    saveSceneButton.setBounds(scaledBounds(1147, 31, 128, 53));

    masterLabel.setBounds(scaledBounds(162, 136, 150, 20));
    globalXFadeLabel.setBounds(scaledBounds(545, 136, 174, 20));
    masterLowCutLabel.setBounds(scaledBounds(944, 136, 160, 20));
    masterHighCutLabel.setBounds(scaledBounds(1330, 136, 160, 20));

    masterSlider.setBounds(scaledBounds(64, 120, 84, 84));
    globalXFadeSlider.setBounds(scaledBounds(444, 120, 84, 84));
    masterLowCutSlider.setBounds(scaledBounds(848, 120, 84, 84));
    masterHighCutSlider.setBounds(scaledBounds(1230, 120, 84, 84));

    randomizationLabel.setBounds(scaledBounds(78, 859, 150, 18));
    randomizeButton.setBounds(scaledBounds(29, 879, 225, 43));
    randomStartLabel.setBounds(scaledBounds(349, 859, 130, 18));
    randomStartSlider.setBounds(scaledBounds(272, 856, 68, 68));
    masterMeterLabel.setBounds(scaledBounds(596, 859, 160, 18));

    for (int i = 0; i < (int) rows.size(); ++i)
    {
        if (rows[(size_t) i] != nullptr)
        {
            rows[(size_t) i]->setBounds(scaledBounds(16, 280 + i * 70, 1639, 70));
        }
    }
}

void SceneLooperAudioProcessorEditor::refreshLayerNames()
{
    for (auto& row : rows)
    {
        if (row != nullptr)
            row->refreshFileName();
    }
}

void SceneLooperAudioProcessorEditor::refreshLayerTimes()
{
    for (auto& row : rows)
    {
        if (row != nullptr)
            row->refreshTimeDisplay();
    }
}

void SceneLooperAudioProcessorEditor::timerCallback()
{
    refreshLayerTimes();
    repaint();
}

SceneLooperAudioProcessorEditor::LayerRow::WaveformPreview::WaveformPreview(SceneLooperAudioProcessor& p, int index)
    : processor(p), layerIndex(index)
{
}

void SceneLooperAudioProcessorEditor::LayerRow::WaveformPreview::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f, 0.0f);
    const auto accent = Theme::layerColour(layerIndex);
    const auto highlight = Theme::layerHighlightColour(layerIndex);

    std::array<float, SceneLooperAudioProcessor::waveformPreviewPoints> preview;
    const bool hasPreview = processor.copyWaveformPreview(layerIndex, preview);
    const float centreY = bounds.getCentreY();

    auto drawEdgeMarker = [&g, bounds] (float x, bool right)
    {
        const auto y = bounds.getBottom() - 9.0f;
        const auto h = 8.0f;
        g.setColour(Theme::mutedText.withAlpha(0.54f));
        g.drawLine(x, y, x, y + h, 1.15f);
        g.drawLine(x, y, x + (right ? -3.2f : 3.2f), y, 1.15f);
        g.drawLine(x, y + h, x + (right ? -3.2f : 3.2f), y + h, 1.15f);
    };

    drawEdgeMarker(bounds.getX() + 2.0f, false);
    drawEdgeMarker(bounds.getRight() - 2.0f, true);

    if (! hasPreview)
    {
        g.setColour(accent.withAlpha(0.16f));
        g.drawLine(bounds.getX() + 4.0f, centreY, bounds.getRight() - 4.0f, centreY, 1.0f);
        return;
    }

    g.reduceClipRegion(bounds.toNearestInt());

    const float usableHeight = bounds.getHeight() * 0.90f;
    const float pointWidth = bounds.getWidth() / (float) SceneLooperAudioProcessor::waveformPreviewPoints;
    const float displayGain = processor.getLayerWaveformDisplayGain(layerIndex);

    g.setColour(accent.withAlpha(0.12f));
    g.drawLine(bounds.getX() + 6.0f, centreY, bounds.getRight() - 6.0f, centreY, 3.4f);

    for (int i = 0; i < SceneLooperAudioProcessor::waveformPreviewPoints; ++i)
    {
        const float previous = i > 0 ? preview[(size_t) i - 1] : preview[(size_t) i];
        const float next = i + 1 < SceneLooperAudioProcessor::waveformPreviewPoints ? preview[(size_t) i + 1] : preview[(size_t) i];
        const float peak = juce::jlimit(0.0f, 1.0f, (previous + preview[(size_t) i] * 1.8f + next) * 0.26f * displayGain);
        const float x = bounds.getX() + ((float) i + 0.5f) * pointWidth;
        const float y = juce::jmax(0.8f, peak * usableHeight);
        const auto waveColour = accent.interpolatedWith(Theme::cyan, (float) i / (float) SceneLooperAudioProcessor::waveformPreviewPoints);
        g.setColour(waveColour.withAlpha(0.16f));
        g.fillRoundedRectangle(x - 2.0f, centreY - y - 2.0f, 4.0f, y * 2.0f + 4.0f, 1.6f);
        g.setColour(waveColour.withAlpha(0.92f));
        g.fillRoundedRectangle(x - 1.05f, centreY - y, 2.1f, y * 2.0f, 0.9f);
    }

    const auto cursorFraction = processor.getLayerPlaybackPositionFraction(layerIndex);
    if (cursorFraction >= 0.0)
    {
        const float cursorX = bounds.getX() + bounds.getWidth() * (float) cursorFraction;
        g.setColour(Theme::cyan.withAlpha(0.22f));
        g.drawLine(cursorX, bounds.getY() + 2.0f, cursorX, bounds.getBottom() - 2.0f, 4.0f);
        g.setColour(Theme::text.withAlpha(0.92f));
        g.drawLine(cursorX, bounds.getY() + 2.0f, cursorX, bounds.getBottom() - 2.0f, 1.35f);
        g.setColour(highlight.withAlpha(0.70f));
        g.fillEllipse(cursorX - 1.8f, bounds.getY() + 1.0f, 3.6f, 3.6f);
    }
}

void SceneLooperAudioProcessorEditor::LayerRow::WaveformPreview::mouseDown(const juce::MouseEvent& event)
{
    seekToMousePosition(event.position);
}

void SceneLooperAudioProcessorEditor::LayerRow::WaveformPreview::mouseDrag(const juce::MouseEvent& event)
{
    seekToMousePosition(event.position);
}

void SceneLooperAudioProcessorEditor::LayerRow::WaveformPreview::seekToMousePosition(juce::Point<float> position)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    if (bounds.getWidth() <= 0.0f)
        return;

    const auto fraction = (position.x - bounds.getX()) / bounds.getWidth();
    processor.seekLayerToFraction(layerIndex, juce::jlimit(0.0f, 1.0f, fraction));
    repaint();
}

SceneLooperAudioProcessorEditor::LayerRow::LayerRow(SceneLooperAudioProcessor& p, int index)
    : processor(p), layerIndex(index), waveformPreview(p, index)
{
    const auto accent = Theme::layerColour(layerIndex);

    numberLabel.setText(juce::String(index + 1), juce::dontSendNotification);
    numberLabel.setJustificationType(juce::Justification::centred);
    numberLabel.setColour(juce::Label::textColourId, Theme::layerHighlightColour(layerIndex));
    numberLabel.setFont(juce::Font(26.0f, juce::Font::plain));
    addAndMakeVisible(numberLabel);
    numberLabel.setVisible(false);

    loadButton.setButtonText("Load");
    loadButton.setTooltip("Load WAV: choose an audio file for this layer.");
    fileLabel.setText(processor.getFileNameForLayer(layerIndex), juce::dontSendNotification);
    fileLabel.setColour(juce::Label::textColourId, Theme::valueText.withAlpha(0.86f));
    fileLabel.setJustificationType(juce::Justification::centredLeft);
    fileLabel.setFont(juce::Font("Avenir Next", 9.6f, juce::Font::plain));
    addAndMakeVisible(fileLabel);
    addAndMakeVisible(waveformPreview);

    addAndMakeVisible(loadButton);
    addAndMakeVisible(onButton);
    onButton.setTooltip("Layer On/Off: enable or mute this layer.");
    soloButton.setButtonText("S");
    addAndMakeVisible(soloButton);
    soloButton.setTooltip("Solo: listen to this layer by itself.");
    autoPanButton.setButtonText("AP");
    addAndMakeVisible(autoPanButton);
    autoPanButton.setTooltip("Auto Pan: turn automatic left-right panning on or off for this layer.");

    auto setupTimeLabel = [] (juce::Label& label)
    {
        label.setJustificationType(juce::Justification::centredRight);
        label.setColour(juce::Label::textColourId, Theme::mutedText);
        label.setFont(8.6f);
    };

    setupTimeLabel(lengthLabel);
    setupTimeLabel(remainLabel);
    addAndMakeVisible(lengthLabel);
    addAndMakeVisible(remainLabel);
    lengthLabel.setVisible(false);
    remainLabel.setVisible(false);

    auto setupLabel = [] (juce::Label& label, const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, Theme::mutedText);
        label.setFont(8.4f);
    };

    setupLabel(volumeLabel, "Volume");
    setupLabel(panLabel, "Pan");
    setupLabel(speedLabel, "Speed");
    setupLabel(driftLabel, "Drift");
    setupLabel(widthLabel, "Width");
    setupLabel(autoPanAmountLabel, "Amount");
    setupLabel(autoPanRateLabel, "Rate");
    setupLabel(hpLabel, "HP");
    setupLabel(lpLabel, "LP");
    setupLabel(xfadeLabel, "XFade");
    setupLabel(offsetLabel, "Start Offset");

    addAndMakeVisible(volumeLabel);
    addAndMakeVisible(panLabel);
    addAndMakeVisible(speedLabel);
    addAndMakeVisible(driftLabel);
    addAndMakeVisible(widthLabel);
    addAndMakeVisible(autoPanAmountLabel);
    addAndMakeVisible(autoPanRateLabel);
    addAndMakeVisible(hpLabel);
    addAndMakeVisible(lpLabel);
    addAndMakeVisible(xfadeLabel);
    addAndMakeVisible(offsetLabel);
    for (auto* label : { &volumeLabel, &panLabel, &speedLabel, &driftLabel, &widthLabel,
                         &autoPanAmountLabel, &autoPanRateLabel, &hpLabel, &lpLabel, &xfadeLabel,
                         &offsetLabel })
    {
        label->setVisible(false);
    }

    setupSlider(volumeSlider, " dB");
    setupSlider(panSlider, "");
    setupSlider(speedSlider, "k");
    setupSlider(driftSlider, "%");
    setupSlider(widthSlider, "%");
    setupSlider(autoPanAmountSlider, "");
    setupSlider(autoPanRateSlider, " Hz");
    setupSlider(hpSlider, " Hz");
    setupSlider(lpSlider, " Hz");
    setupSlider(xfadeSlider, " s");
    setupSlider(offsetSlider, " s");

    speedSlider.setNumDecimalPlacesToDisplay(1);
    driftSlider.setNumDecimalPlacesToDisplay(0);
    widthSlider.setNumDecimalPlacesToDisplay(0);
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    panSlider.textFromValueFunction = [] (double value)
    {
        if (std::abs(value) < 0.005)
            return juce::String("C");

        return juce::String(value < 0.0 ? "L " : "R ") + juce::String((int) std::round(std::abs(value) * 100.0));
    };
    autoPanAmountSlider.textFromValueFunction = [] (double value)
    {
        return juce::String((int) std::round(value * 100.0)) + "%";
    };
    autoPanAmountSlider.valueFromTextFunction = [] (const juce::String& text)
    {
        return juce::jlimit(0.0, 1.0, text.retainCharacters("0123456789.").getDoubleValue() * 0.01);
    };
    speedSlider.textFromValueFunction = [] (double value)
    {
        return juce::String(value, 1) + "k";
    };
    hpSlider.textFromValueFunction = [] (double value)
    {
        return value >= 1000.0 ? juce::String(value / 1000.0, 1) + "k" : juce::String((int) std::round(value)) + " Hz";
    };
    lpSlider.textFromValueFunction = hpSlider.textFromValueFunction;

    for (auto* slider : { &volumeSlider, &panSlider, &speedSlider, &driftSlider, &widthSlider,
                          &autoPanAmountSlider, &autoPanRateSlider, &hpSlider, &lpSlider, &xfadeSlider,
                          &offsetSlider })
    {
        slider->setColour(juce::Slider::rotarySliderFillColourId, accent);
        slider->setColour(juce::Slider::trackColourId, accent);
    }

    addAndMakeVisible(volumeSlider);
    addAndMakeVisible(panSlider);
    addAndMakeVisible(speedSlider);
    addAndMakeVisible(driftSlider);
    addAndMakeVisible(widthSlider);
    addAndMakeVisible(autoPanAmountSlider);
    addAndMakeVisible(autoPanRateSlider);
    addAndMakeVisible(hpSlider);
    addAndMakeVisible(lpSlider);
    addAndMakeVisible(xfadeSlider);
    addAndMakeVisible(offsetSlider);

    onAttachment = std::make_unique<ButtonAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "on"), onButton);
    soloAttachment = std::make_unique<ButtonAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "solo"), soloButton);
    autoPanAttachment = std::make_unique<ButtonAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "autoPanOn"), autoPanButton);
    volumeAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "volume"), volumeSlider);
    panAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "pan"), panSlider);
    speedAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "speed"), speedSlider);
    driftAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "drift"), driftSlider);
    widthAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "width"), widthSlider);
    autoPanAmountAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "autoPanAmount"), autoPanAmountSlider);
    autoPanRateAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "autoPanRate"), autoPanRateSlider);
    hpAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "hp"), hpSlider);
    lpAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "lp"), lpSlider);
    xfadeAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "xfade"), xfadeSlider);
    offsetAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "offset"), offsetSlider);

    loadButton.onClick = [this]
    {
        fileChooser = std::make_unique<juce::FileChooser>("Load WAV 48 kHz / 24-bit", juce::File{}, "*.wav");
        fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& chooser)
            {
                const auto file = chooser.getResult();
                if (file.existsAsFile())
                {
                    juce::String error;
                    if (! processor.loadFileForLayer(layerIndex, file, error))
                        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Atmocycle", error);
                    refreshFileName();
                    refreshTimeDisplay();
                }
            });
    };

    refreshTimeDisplay();
}

void SceneLooperAudioProcessorEditor::LayerRow::setupSlider(juce::Slider& slider, const juce::String& suffix)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setTextValueSuffix(suffix);
}

void SceneLooperAudioProcessorEditor::LayerRow::paint(juce::Graphics&)
{
}

void SceneLooperAudioProcessorEditor::LayerRow::paintOverChildren(juce::Graphics& g)
{
    auto drawControlValue = [&g] (juce::Slider& slider, int width = 56)
    {
        auto value = slider.getBounds().withSizeKeepingCentre(scaledX(width), scaledY(17));
        value.setY(slider.getBottom() + scaledY(2));
        g.setColour(Theme::valueText.withAlpha(0.88f));
        g.setFont(juce::Font("Avenir Next", scaledFont(10.2f), juce::Font::plain));
        g.drawFittedText(sliderValueText(slider), value, juce::Justification::centred, 1);
    };

    if (processor.isLayerLoaded(layerIndex))
        drawTinyFileBadge(g, scaledBoundsF(93.0f, 10.5f, 12.0f, 12.0f), Theme::layerColour(layerIndex));

    drawTextValue(g, scaledBounds(640, 21, 43, 30), sliderValueText(volumeSlider), scaledFont(11.2f), juce::Justification::centred);
    drawControlValue(panSlider, 46);
    drawControlValue(autoPanAmountSlider, 46);
    drawControlValue(autoPanRateSlider, 52);
    drawControlValue(speedSlider, 50);
    drawControlValue(driftSlider, 44);
    drawControlValue(widthSlider, 48);
    drawControlValue(offsetSlider, 58);
    drawControlValue(hpSlider, 52);
    drawControlValue(lpSlider, 52);
    drawControlValue(xfadeSlider, 54);

    g.setColour(Theme::valueText.withAlpha(0.82f));
    g.setFont(juce::Font("Avenir Next", scaledFont(9.6f), juce::Font::plain));
    g.drawFittedText(lengthLabel.getText(), scaledBounds(308, 10, 62, 18),
                     juce::Justification::centredRight, 1);

    auto led = scaledBoundsF(106.0f, 63.0f, 318.0f, 5.5f);
    const int segments = 40;
    const auto rawLevel = juce::jlimit(0.0f, 1.0f, processor.getLayerLevel(layerIndex));
    const auto levelDb = rawLevel > 0.000001f ? juce::Decibels::gainToDecibels(rawLevel) : -80.0f;
    const auto level = rawLevel <= 0.000001f
                           ? 0.0f
                           : (levelDb < -40.0f
                                  ? juce::jmap(juce::jlimit(-60.0f, -40.0f, levelDb), -60.0f, -40.0f, 0.16f, 0.92f)
                                  : juce::jmap(juce::jlimit(-40.0f, -6.0f, levelDb), -40.0f, -6.0f, 0.38f, 1.0f));
    displayedLayerMeter = juce::jmax(level, displayedLayerMeter * 0.84f + level * 0.16f);
    for (int i = 0; i < segments; ++i)
    {
        const float t = (float) i / (float) (segments - 1);
        const bool active = t <= displayedLayerMeter;
        const auto colour = Theme::purple.interpolatedWith(Theme::cyan, t);
        const auto segmentWidth = led.getWidth() / (float) segments;
        const auto x = led.getX() + (float) i * segmentWidth;
        g.setColour(active ? colour.withAlpha(0.36f) : juce::Colour(0xff0b2a30).withAlpha(0.08f));
        g.fillRoundedRectangle(x, led.getY(), juce::jmax(1.6f, segmentWidth - 2.0f), led.getHeight(), 0.55f);
    }
}

void SceneLooperAudioProcessorEditor::LayerRow::resized()
{
    numberLabel.setBounds(scaledBounds(0, 0, 60, 70));

    fileLabel.setBounds(scaledBounds(110, 10, 196, 18));
    waveformPreview.setBounds(scaledBounds(92, 25, 328, 36));
    loadButton.setBounds(scaledBounds(374, 10, 64, 26));

    onButton.setBounds(scaledBounds(452, 19, 28, 28));
    soloButton.setBounds(scaledBounds(488, 19, 28, 28));
    autoPanButton.setBounds(scaledBounds(524, 19, 28, 28));

    volumeSlider.setBounds(scaledBounds(578, 22, 78, 26));

    auto placeKnob = [] (juce::Slider& slider, int centreX, int centreY, int size = 42)
    {
        slider.setBounds(scaledX(centreX - size / 2), scaledY(centreY - size / 2),
                         scaledX(size), scaledY(size));
    };

    placeKnob(panSlider, 724, 34);
    placeKnob(autoPanAmountSlider, 838, 34);
    placeKnob(autoPanRateSlider, 940, 34);
    placeKnob(speedSlider, 1026, 34);
    placeKnob(driftSlider, 1126, 34);
    placeKnob(widthSlider, 1218, 34);
    placeKnob(offsetSlider, 1332, 34);
    placeKnob(hpSlider, 1428, 34);
    placeKnob(lpSlider, 1508, 34);
    placeKnob(xfadeSlider, 1592, 34);

    lengthLabel.setBounds(scaledBounds(1502, 17, 116, 18));
    remainLabel.setBounds(scaledBounds(1502, 34, 116, 18));
}

void SceneLooperAudioProcessorEditor::LayerRow::refreshFileName()
{
    fileLabel.setText(processor.getFileNameForLayer(layerIndex), juce::dontSendNotification);
    waveformPreview.repaint();
}

void SceneLooperAudioProcessorEditor::LayerRow::refreshTimeDisplay()
{
    lengthLabel.setText("Len " + formatTime(processor.getLayerLengthSeconds(layerIndex), false),
        juce::dontSendNotification);
    remainLabel.setText("Rem " + formatTime(processor.getLayerRemainingSeconds(layerIndex), true),
        juce::dontSendNotification);
    waveformPreview.repaint();
}
