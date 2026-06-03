#include "PluginEditor.h"
#include "BinaryData.h"

#include <algorithm>
#include <array>
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
constexpr int editorWidth = 1360;
constexpr int editorHeight = 765;
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
    return value * Theme::scaleY * 1.16f;
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

    const auto iconSize = juce::jmin(area.getHeight() * 0.34f, area.getWidth() * 0.18f);
    const auto textWidth = scaledX(save ? 74.0f : 80.0f);
    const auto gap = scaledX(8.0f);
    const auto groupWidth = iconSize * 1.22f + gap + textWidth;
    const auto icon = juce::Rectangle<float>(area.getCentreX() - groupWidth * 0.5f + scaledX(3.0f),
                                             area.getCentreY() - iconSize * 0.5f,
                                             iconSize * 1.22f,
                                             iconSize);

    if (save)
        drawDiskIcon(g, icon, accent);
    else
        drawFolderIcon(g, icon, accent);

    g.setColour(Theme::labelText.withAlpha(highlighted ? 0.96f : 0.84f));
    g.setFont(juce::Font("Avenir Next", scaledFont(13.4f), juce::Font::plain));
    g.drawFittedText(text, juce::Rectangle<int>((int) std::round(icon.getRight() + gap),
                                                bounds.getY(),
                                                (int) std::round(textWidth),
                                                bounds.getHeight()),
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

juce::String levelToDbNumberText(float level)
{
    if (level <= 0.00001f)
        return "-inf";

    return juce::String(juce::Decibels::gainToDecibels(level), 1);
}

juce::String decimalText(double value, int places = 2)
{
    return juce::String(value, places);
}

juce::String panText(double value)
{
    if (std::abs(value) > 1.0)
        value *= 0.001;

    value = juce::jlimit(-1.0, 1.0, value);
    if (std::abs(value) < 0.01)
        return "C";

    return juce::String(value < 0.0 ? "L" : "R") + juce::String((int) std::round(std::abs(value) * 100.0));
}

void drawSubtleStructure(juce::Graphics& g)
{
    const auto veil = juce::Colour(0xff001014);
    const auto line = juce::Colour(0xff1d7b86);

    auto drawLine = [&g, line] (float x1, float y1, float x2, float y2, float alpha = 0.095f)
    {
        g.setColour(line.withAlpha(alpha));
        g.drawLine(scaledX(x1), scaledY(y1), scaledX(x2), scaledY(y2), juce::jmax(0.45f, Theme::scaleY * 0.75f));
    };

    g.setColour(veil.withAlpha(0.18f));
    g.fillRoundedRectangle(scaledBoundsF(82.0f, 280.0f, 1572.0f, 560.0f), scaledY(4.0f));

    for (int i = 0; i <= 8; ++i)
        drawLine(82.0f, 280.0f + (float) i * 70.0f, 1654.0f, 280.0f + (float) i * 70.0f, i == 0 ? 0.12f : 0.075f);

    const std::array<float, 16> layerColumns {
        98.0f, 522.0f, 626.0f, 786.0f, 865.8f, 945.6f, 1025.5f,
        1105.3f, 1185.1f, 1264.9f, 1344.7f, 1424.5f, 1504.4f, 1584.2f,
        1648.0f, 1654.0f
    };

    for (auto x : layerColumns)
        drawLine(x, 280.0f, x, 840.0f, 0.070f);

    g.setColour(veil.withAlpha(0.12f));
    g.fillRoundedRectangle(scaledBoundsF(16.0f, 115.0f, 1640.0f, 111.0f), scaledY(7.0f));
    for (float x : { 426.0f, 836.0f, 1246.0f })
        drawLine(x, 142.0f, x, 199.0f, 0.040f);

    g.setColour(veil.withAlpha(0.16f));
    g.fillRoundedRectangle(scaledBoundsF(16.0f, 844.0f, 1640.0f, 92.0f), scaledY(6.0f));
    for (float x : { 300.0f, 535.0f, 980.0f })
        drawLine(x, 855.0f, x, 921.0f, 0.085f);
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
            const auto radius = scaledY(4.5f);
            const auto accent = Theme::purple.interpolatedWith(Theme::cyan, 0.16f);

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
            g.setColour(accent.withAlpha(active ? 0.42f : 0.22f));
            g.drawRoundedRectangle(bounds, radius, active ? 1.05f : 0.8f);
            g.setColour(Theme::text.withAlpha(active ? 0.92f : 0.78f));
            g.setFont(juce::Font("Avenir Next", scaledFont(10.0f), juce::Font::plain));
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
        auto drawButtonShell = [&] (juce::Rectangle<float> box)
        {
            juce::ColourGradient fill(juce::Colour(0xff12282e).withAlpha(highlighted || down ? 0.78f : 0.56f),
                                      box.getX(), box.getY(),
                                      juce::Colour(0xff020608).withAlpha(0.92f),
                                      box.getRight(), box.getBottom(), false);
            g.setGradientFill(fill);
            g.fillRoundedRectangle(box, 5.5f);
            g.setColour(juce::Colours::white.withAlpha(0.035f));
            g.fillRoundedRectangle(box.reduced(1.2f).withTrimmedBottom(box.getHeight() * 0.62f), 4.5f);
            g.setColour(Theme::stroke.withAlpha(on ? 0.28f : 0.20f));
            g.drawRoundedRectangle(box, 5.5f, 0.75f);
        };

        auto drawGlowingPath = [&] (const juce::Path& path, float thickness)
        {
            const auto offColour = Theme::mutedText.withAlpha(0.54f);
            if (on)
            {
                g.setColour(Theme::purple.withAlpha(0.18f));
                g.strokePath(path, juce::PathStrokeType(thickness + 2.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                g.setColour(Theme::purple.withAlpha(0.92f));
                g.strokePath(path, juce::PathStrokeType(thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
            else
            {
                g.setColour(offColour);
                g.strokePath(path, juce::PathStrokeType(thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        };

        if (button.getButtonText() == "On")
        {
            auto box = bounds.withSizeKeepingCentre(24.0f, 24.0f);
            drawButtonShell(box);

            const auto centre = box.getCentre();
            juce::Path powerArc;
            powerArc.addCentredArc(centre.x, centre.y + 1.0f, 7.0f, 7.0f, 0.0f,
                                   -2.35f, 2.35f, true);
            drawGlowingPath(powerArc, 2.1f);

            juce::Path powerLine;
            powerLine.startNewSubPath(centre.x, centre.y - 8.4f);
            powerLine.lineTo(centre.x, centre.y - 1.8f);
            drawGlowingPath(powerLine, 2.1f);
            return;
        }

        if (button.getButtonText() == "S")
        {
            auto box = bounds.withSizeKeepingCentre(24.0f, 24.0f);
            drawButtonShell(box);
            if (on)
            {
                g.setColour(Theme::purple.withAlpha(0.22f));
                g.setFont(juce::Font("Avenir Next", 15.5f, juce::Font::plain));
                g.drawFittedText("S", box.toNearestInt().reduced(3), juce::Justification::centred, 1);
            }

            g.setColour((on ? Theme::purple : Theme::mutedText).withAlpha(on ? 0.94f : 0.54f));
            g.setFont(juce::Font("Avenir Next", 15.5f, juce::Font::plain));
            g.drawFittedText("S", box.toNearestInt().reduced(3), juce::Justification::centred, 1);
            return;
        }

        if (button.getButtonText() == "AP")
        {
            auto box = bounds.withSizeKeepingCentre(26.0f, 26.0f);
            drawButtonShell(box);

            juce::Path wave;
            const auto waveArea = box.reduced(5.0f, 7.0f);
            for (int i = 0; i < 32; ++i)
            {
                const float t = (float) i / 31.0f;
                const float xx = waveArea.getX() + t * waveArea.getWidth();
                const float yy = waveArea.getCentreY() + std::sin((t - 0.08f) * juce::MathConstants<float>::twoPi) * waveArea.getHeight() * 0.46f;
                if (i == 0)
                    wave.startNewSubPath(xx, yy);
                else
                    wave.lineTo(xx, yy);
            }
            if (on)
            {
                g.setColour(Theme::purple.withAlpha(0.22f));
                g.strokePath(wave, juce::PathStrokeType(5.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
            drawGlowingPath(wave, 2.35f);
            return;
        }

        if (button.getButtonText() == "AUTO")
        {
            auto box = bounds.withSizeKeepingCentre(28.0f, 24.0f);
            drawButtonShell(box);

            const auto blink = on ? 0.55f + 0.35f * (float) std::sin(juce::Time::getMillisecondCounterHiRes() * 0.007) : 0.0f;
            if (on)
            {
                const auto red = juce::Colour(0xffd12a55);
                g.setColour(red.withAlpha(0.20f + blink * 0.25f));
                g.fillRoundedRectangle(box.reduced(2.0f), 4.2f);
                g.setColour(red.withAlpha(0.35f + blink * 0.35f));
                g.drawRoundedRectangle(box.expanded(1.4f), 6.0f, 1.0f);
            }

            g.setColour((on ? juce::Colour(0xffff6d83) : Theme::mutedText).withAlpha(on ? 0.98f : 0.52f));
            g.setFont(juce::Font("Avenir Next", 8.0f, juce::Font::bold));
            g.drawFittedText("AUTO", box.toNearestInt().reduced(2), juce::Justification::centred, 1);
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
    tooltipWindow = std::make_unique<juce::TooltipWindow>(this, 1100);

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
    addAndMakeVisible(versionButton);
    makeHitZoneOnly(versionButton);
    loadSceneButton.setTooltip("Load scene");
    saveSceneButton.setTooltip("Save scene");
    versionButton.setTooltip("Show Atmocycle version");
    versionButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    const char* headerTooltips[] = {
        "Load, solo and auto-pan controls for the layer.",
        "Layer volume in decibels.",
        "Stereo position: left, centre or right.",
        "How strongly auto-pan moves the sound. The small wave/S-shaped AP button enables auto-pan.",
        "Auto-pan movement speed. The small wave/S-shaped AP button enables auto-pan.",
        "Playback-rate resampling amount. Lower kHz is faster/higher, higher kHz is slower/lower.",
        "Subtle random pitch/playback movement.",
        "Stereo width.",
        "Pan XFade smoothly crossmixes left and right channels without polarity inversion.",
        "Start position offset in seconds.",
        "High-pass filter cutoff.",
        "Low-pass filter cutoff.",
        "Layer loop and skip crossfade time.",
        "Click-drag seeks; Cmd/Ctrl-drag creates skip zones."
    };
    for (size_t i = 0; i < headerTooltipZones.size(); ++i)
    {
        headerTooltipZones[i].setText({}, juce::dontSendNotification);
        headerTooltipZones[i].setTooltip(headerTooltips[i]);
        headerTooltipZones[i].setAlpha(0.01f);
        addAndMakeVisible(headerTooltipZones[i]);
    }

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
    setupMacro(masterHighCutSlider, " Hz");
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
        return juce::String((int) std::round(value)) + " Hz";
    };
    masterHighCutSlider.valueFromTextFunction = [] (const juce::String& text)
    {
        const auto value = text.retainCharacters("0123456789.").getDoubleValue();
        return text.containsIgnoreCase("k") ? value * 1000.0 : value;
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
    randomizeButton.setTooltip("Randomize starts");

    versionButton.onClick = []
    {
        const auto message = juce::String("Atmocycle v") + JucePlugin_VersionString
                           + "\nBuild: " + __DATE__ + " " + __TIME__;
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Atmocycle", message);
    };

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
        drawSubtleStructure(g);
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
    drawTrackedText(g, scaledBoundsF(132.0f, 18.0f, 390.0f, 48.0f), "Atmocycle", scaledFont(39.0f),
                    juce::Justification::centredLeft, scaledX(1.10f), juce::Colour(0xffddd7cf), true);
    drawTrackedText(g, scaledBoundsF(132.0f, 64.0f, 305.0f, 24.0f), "by Echosynthesis", scaledFont(17.0f),
                    juce::Justification::centredLeft, scaledX(1.30f), Theme::labelText.withAlpha(0.91f), true);

    drawTopButtonContent(g, scaledBounds(774.0f, 31.0f, 126.0f, 52.0f), "LOAD SCENE", false, loadSceneButton.isMouseOver());
    drawTopButtonContent(g, scaledBounds(914.0f, 31.0f, 126.0f, 52.0f), "SAVE SCENE", true, saveSceneButton.isMouseOver());

    const auto legendText = juce::String("AUTO BUTTON = AUTOPILOT: RANDOM STARTS, SKIP-ZONE JUMPS, XFADE BLENDS\n")
                          + "CMD/CTRL+DRAG WAVEFORM = SKIP AREA   S-SHAPED AP BUTTON = AUTO PAN   PAN XFADE = SAFE L/R CROSSMIX";
    g.setColour(Theme::labelText.withAlpha(0.64f));
    g.setFont(juce::Font("Avenir Next", scaledFont(10.4f), juce::Font::plain));
    g.drawFittedText(legendText, scaledBounds(1083.0f, 28.0f, 560.0f, 54.0f),
                     juce::Justification::centredLeft, 2);

    const auto sceneName = processor.getCurrentSceneName() == "Untitled Scene"
                               ? juce::String("Project State")
                               : processor.getCurrentSceneName();
    const auto sceneDisplay = juce::String("Scene Name: ") + sceneName;

    g.setColour(Theme::valueText.withAlpha(0.90f));
    g.setFont(juce::Font("Avenir Next", scaledFont(14.4f), juce::Font::plain));
    const auto sceneField = sceneNameLabel.getBounds().toFloat().reduced(1.0f);
    g.setColour((processor.getCurrentSceneName() == "Untitled Scene" ? Theme::stroke : Theme::purple).withAlpha(0.22f));
    g.drawRoundedRectangle(sceneField, scaledY(6.0f), scaledY(0.85f));
    g.setColour(Theme::valueText.withAlpha(0.90f));
    g.drawFittedText(sceneDisplay, sceneNameLabel.getBounds(), juce::Justification::centred, 1);

    drawUiLabel(g, scaledBoundsF(176.0f, 143.0f, 190.0f, 21.0f), "MASTER OUTPUT", scaledFont(14.0f), juce::Justification::centredLeft);
    drawUiLabel(g, scaledBoundsF(586.0f, 143.0f, 205.0f, 21.0f), "GLOBAL CROSSFADE", scaledFont(14.0f), juce::Justification::centredLeft);
    drawUiLabel(g, scaledBoundsF(996.0f, 143.0f, 198.0f, 21.0f), "MASTER LOW CUT", scaledFont(14.0f), juce::Justification::centredLeft);
    drawUiLabel(g, scaledBoundsF(1406.0f, 143.0f, 210.0f, 21.0f), "MASTER HIGH CUT", scaledFont(14.0f), juce::Justification::centredLeft);

    drawUiLabel(g, scaledBoundsF(31.0f, 245.0f, 70.0f, 22.0f), "LAYER", scaledFont(12.2f), juce::Justification::centredLeft);
    drawUiLabel(g, scaledBoundsF(128.0f, 245.0f, 180.0f, 22.0f), "FILE / LOOP", scaledFont(12.2f), juce::Justification::centredLeft);
    drawUiLabel(g, scaledBoundsF(522.0f, 245.0f, 104.0f, 22.0f), "CONTROL", scaledFont(12.2f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(626.0f, 245.0f, 160.0f, 22.0f), "VOLUME", scaledFont(12.2f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(796.0f, 245.0f, 60.0f, 22.0f), "PAN", scaledFont(12.2f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(865.5f, 237.0f, 80.0f, 16.0f), "AUTO PAN", scaledFont(11.2f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(865.5f, 253.0f, 80.0f, 16.0f), "AMOUNT", scaledFont(11.2f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(945.5f, 237.0f, 80.0f, 16.0f), "AUTO PAN", scaledFont(11.2f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(945.5f, 253.0f, 80.0f, 16.0f), "RATE", scaledFont(11.2f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(1030.0f, 245.0f, 70.0f, 22.0f), "SPEED", scaledFont(12.2f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(1110.0f, 245.0f, 70.0f, 22.0f), "DRIFT", scaledFont(12.2f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(1190.0f, 245.0f, 70.0f, 22.0f), "WIDTH", scaledFont(12.2f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(1269.5f, 237.0f, 70.0f, 16.0f), "PAN", scaledFont(10.2f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(1269.5f, 253.0f, 70.0f, 16.0f), "XFADE", scaledFont(10.2f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(1346.5f, 237.0f, 76.0f, 16.0f), "START", scaledFont(10.8f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(1346.5f, 253.0f, 76.0f, 16.0f), "OFFSET", scaledFont(10.8f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(1434.0f, 245.0f, 60.0f, 22.0f), "HP", scaledFont(12.2f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(1514.0f, 245.0f, 60.0f, 22.0f), "LP", scaledFont(12.2f), juce::Justification::centred);
    drawUiLabel(g, scaledBoundsF(1594.0f, 245.0f, 60.0f, 22.0f), "XFADE", scaledFont(12.2f), juce::Justification::centred);

    drawTextValue(g, scaledBounds(166, 171, 157, 38), sliderValueText(masterSlider), scaledFont(15.2f), juce::Justification::centredLeft);
    drawTextValue(g, scaledBounds(576, 171, 157, 38), sliderValueText(globalXFadeSlider), scaledFont(15.2f), juce::Justification::centredLeft);
    drawTextValue(g, scaledBounds(986, 171, 157, 38), sliderValueText(masterLowCutSlider), scaledFont(15.2f), juce::Justification::centredLeft);
    drawTextValue(g, scaledBounds(1396, 171, 157, 38), sliderValueText(masterHighCutSlider), scaledFont(15.2f), juce::Justification::centredLeft);

    const auto randomShell = scaledBoundsF(42.0f, 858.0f, 218.0f, 70.0f);
    if (randomizeButton.isMouseOver())
    {
        juce::ColourGradient fill(juce::Colour(0xff09242a).withAlpha(0.72f), randomShell.getX(), randomShell.getY(),
                                  juce::Colour(0xff02080a).withAlpha(0.86f), randomShell.getRight(), randomShell.getBottom(), false);
        g.setGradientFill(fill);
        g.fillRoundedRectangle(randomShell, scaledY(7.0f));
        g.setColour(Theme::cyan.withAlpha(0.34f));
        g.drawRoundedRectangle(randomShell, scaledY(7.0f), scaledY(1.0f));
    }

    drawDiceIcon(g, scaledBoundsF(45.0f, 872.0f, 48.0f, 44.0f));

    drawUiLabel(g, scaledBoundsF(99.0f, 873.0f, 185.0f, 39.0f), "RANDOMIZATION", scaledFont(13.8f), juce::Justification::centredLeft);
    drawUiLabel(g, scaledBoundsF(350.0f, 860.0f, 170.0f, 24.0f), "RANDOM START", scaledFont(14.0f), juce::Justification::centredLeft);
    drawTextValue(g, scaledBounds(342, 883, 145, 38), sliderValueText(randomStartSlider), scaledFont(15.2f), juce::Justification::centredLeft);
    drawUiLabel(g, scaledBoundsF(596.0f, 855.0f, 165.0f, 20.0f), "MASTER METER", scaledFont(12.0f), juce::Justification::centredLeft);

    const auto masterLeft = juce::jlimit(0.0f, 1.0f, processor.getMasterLeftLevel());
    const auto masterRight = juce::jlimit(0.0f, 1.0f, processor.getMasterRightLevel());
    auto ledArea = scaledBoundsF(596.0f, 882.0f, 327.0f, 27.0f);
    const auto masterPeak = juce::jmax(masterLeft, masterRight);
    const auto peakDb = masterPeak > 0.000001f ? juce::Decibels::gainToDecibels(masterPeak) : -80.0f;
    const int meterZoom = peakDb < -44.0f ? 0 : (peakDb < -24.0f ? 1 : (peakDb < -12.0f ? 2 : 3));
    const float meterMinDb = meterZoom == 0 ? -60.0f : (meterZoom == 1 ? -48.0f : (meterZoom == 2 ? -36.0f : -24.0f));
    const float meterMaxDb = meterZoom == 0 ? -40.0f : (meterZoom == 1 ? -24.0f : 0.0f);
    const std::array<float, 8> scaleValues = [&]
    {
        if (meterZoom == 0)
            return std::array<float, 8> { -60.0f, -57.0f, -54.0f, -51.0f, -48.0f, -45.0f, -42.0f, -40.0f };
        if (meterZoom == 1)
            return std::array<float, 8> { -48.0f, -44.0f, -40.0f, -36.0f, -32.0f, -28.0f, -26.0f, -24.0f };
        if (meterZoom == 2)
            return std::array<float, 8> { -36.0f, -32.0f, -28.0f, -24.0f, -20.0f, -16.0f, -12.0f, 0.0f };
        return std::array<float, 8> { -24.0f, -21.0f, -18.0f, -15.0f, -12.0f, -9.0f, -6.0f, 0.0f };
    }();

    constexpr int bars = 52;
    auto zoomedMeterLevel = [meterMinDb, meterMaxDb] (float level)
    {
        if (level <= 0.000001f)
            return 0.0f;

        const auto db = juce::Decibels::gainToDecibels(level);
        return juce::jmap(juce::jlimit(meterMinDb, meterMaxDb, db), meterMinDb, meterMaxDb, 0.0f, 1.0f);
    };

    auto drawMeterRow = [&g, &ledArea, &zoomedMeterLevel] (float level, float y, float h)
    {
        const auto shownLevel = zoomedMeterLevel(level);
        for (int i = 0; i < bars; ++i)
        {
            const float t = (float) i / (float) (bars - 1);
            const bool active = t <= shownLevel;
            const auto colour = Theme::purple.interpolatedWith(Theme::cyan, t);
            const auto segmentWidth = ledArea.getWidth() / (float) bars;
            const auto x = ledArea.getX() + (float) i * segmentWidth;
            if (! active)
                continue;

            g.setColour(colour.withAlpha(0.31f));
            g.fillRoundedRectangle(x, y, juce::jmax(1.9f, segmentWidth - 2.3f), h, 0.65f);
            g.setColour(colour.withAlpha(0.13f));
            g.fillRoundedRectangle(x - 0.35f, y - 0.45f, juce::jmax(2.2f, segmentWidth - 1.6f), h + 0.9f, 0.8f);
        }
    };

    drawMeterRow(masterLeft, ledArea.getY() + scaledY(0.0f), scaledY(5.7f));
    drawMeterRow(masterRight, ledArea.getY() + scaledY(10.2f), scaledY(5.7f));

    drawTextValue(g, scaledBounds(908, 881, 86, 24), levelToDbNumberText(masterPeak) + " dB", scaledFont(12.4f), juce::Justification::centredRight);
    for (int i = 0; i < 8; ++i)
    {
        const auto x = 593.0f + (float) i * 46.5f;
        drawUiLabel(g, scaledBoundsF(x, 907.0f, 39.0f, 14.0f), juce::String((int) scaleValues[(size_t) i]), scaledFont(10.2f));
    }
}

void SceneLooperAudioProcessorEditor::resized()
{
    titleLabel.setBounds(scaledBounds(132, 22, 260, 30));
    bylineLabel.setBounds(scaledBounds(132, 62, 210, 20));
    taglineLabel.setBounds(scaledBounds(132, 76, 260, 18));
    versionButton.setBounds(scaledBounds(37, 15, 330, 78));
    sceneCaptionLabel.setBounds(scaledBounds(733, 13, 54, 18));
    sceneNameLabel.setBounds(scaledBounds(500, 31, 260, 52));
    loadSceneButton.setBounds(scaledBounds(774, 31, 126, 52));
    saveSceneButton.setBounds(scaledBounds(914, 31, 126, 52));

    masterLabel.setBounds(scaledBounds(176, 140, 150, 20));
    globalXFadeLabel.setBounds(scaledBounds(586, 140, 174, 20));
    masterLowCutLabel.setBounds(scaledBounds(996, 140, 160, 20));
    masterHighCutLabel.setBounds(scaledBounds(1406, 140, 160, 20));

    masterSlider.setBounds(scaledBounds(79, 128, 84, 84));
    globalXFadeSlider.setBounds(scaledBounds(489, 128, 84, 84));
    masterLowCutSlider.setBounds(scaledBounds(899, 128, 84, 84));
    masterHighCutSlider.setBounds(scaledBounds(1309, 128, 84, 84));

    randomizationLabel.setBounds(scaledBounds(78, 859, 150, 18));
    randomizeButton.setBounds(scaledBounds(42, 858, 218, 70));
    randomStartLabel.setBounds(scaledBounds(350, 860, 170, 24));
    randomStartSlider.setBounds(scaledBounds(272, 864, 68, 68));
    masterMeterLabel.setBounds(scaledBounds(596, 859, 160, 18));

    const std::array<juce::Rectangle<int>, 14> tooltipBounds {
        scaledBounds(522, 236, 104, 35),
        scaledBounds(626, 236, 160, 35),
        scaledBounds(796, 236, 60, 35),
        scaledBounds(866, 232, 80, 41),
        scaledBounds(946, 232, 80, 41),
        scaledBounds(1030, 236, 70, 35),
        scaledBounds(1110, 236, 70, 35),
        scaledBounds(1190, 236, 70, 35),
        scaledBounds(1270, 232, 70, 41),
        scaledBounds(1347, 232, 76, 41),
        scaledBounds(1434, 236, 60, 35),
        scaledBounds(1514, 236, 60, 35),
        scaledBounds(1594, 236, 60, 35),
        scaledBounds(128, 236, 370, 35)
    };
    for (size_t i = 0; i < headerTooltipZones.size(); ++i)
        headerTooltipZones[i].setBounds(tooltipBounds[i]);

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
    const auto accent = Theme::layerColour(layerIndex).interpolatedWith(Theme::purple, 0.76f);
    const auto highlight = Theme::layerHighlightColour(layerIndex);

    std::array<float, SceneLooperAudioProcessor::waveformPreviewPoints> preview;
    const bool hasPreview = processor.copyWaveformPreview(layerIndex, preview);
    const float centreY = bounds.getCentreY();

    if (! hasPreview)
    {
        g.setColour(accent.withAlpha(0.10f));
        g.drawLine(bounds.getX() + 4.0f, centreY, bounds.getRight() - 4.0f, centreY, 1.0f);
        return;
    }

    g.reduceClipRegion(bounds.toNearestInt());

    const float usableHeight = bounds.getHeight() * 0.82f;
    const float pointWidth = bounds.getWidth() / (float) SceneLooperAudioProcessor::waveformPreviewPoints;
    const float displayGain = processor.getLayerWaveformDisplayGain(layerIndex);

    g.setColour(accent.withAlpha(0.08f));
    g.drawLine(bounds.getX() + 6.0f, centreY, bounds.getRight() - 6.0f, centreY, 3.4f);

    for (int i = 0; i < SceneLooperAudioProcessor::waveformPreviewPoints; ++i)
    {
        const float previous = i > 0 ? preview[(size_t) i - 1] : preview[(size_t) i];
        const float next = i + 1 < SceneLooperAudioProcessor::waveformPreviewPoints ? preview[(size_t) i + 1] : preview[(size_t) i];
        const float peak = juce::jlimit(0.0f, 1.0f, (previous + preview[(size_t) i] * 1.8f + next) * 0.26f * displayGain);
        const float x = bounds.getX() + ((float) i + 0.5f) * pointWidth;
        const float y = juce::jmax(0.8f, peak * usableHeight);
        const auto waveColour = accent.interpolatedWith(Theme::purple, 0.30f);
        g.setColour(waveColour.withAlpha(0.040f));
        g.fillRoundedRectangle(x - 2.0f, centreY - y - 2.0f, 4.0f, y * 2.0f + 4.0f, 1.6f);
        g.setColour(waveColour.withAlpha(0.34f));
        g.fillRoundedRectangle(x - 1.05f, centreY - y, 2.1f, y * 2.0f, 0.9f);
    }

    auto drawSkipRegion = [&g, bounds] (double start, double end)
    {
        if (end <= start)
            return;

        const float x1 = bounds.getX() + bounds.getWidth() * (float) juce::jlimit(0.0, 1.0, start);
        const float x2 = bounds.getX() + bounds.getWidth() * (float) juce::jlimit(0.0, 1.0, end);
        auto region = juce::Rectangle<float>(x1, bounds.getY() + 1.0f, juce::jmax(2.0f, x2 - x1), bounds.getHeight() - 2.0f);
        g.setColour(juce::Colours::white.withAlpha(0.125f));
        g.fillRoundedRectangle(region, 2.0f);
        g.setColour(juce::Colours::white.withAlpha(0.20f));
        g.drawRoundedRectangle(region.reduced(0.4f), 2.0f, 0.8f);
    };

    for (const auto& range : processor.getLayerSkipRanges(layerIndex))
        drawSkipRegion(range.start, range.end);

    if (selectingSkipRange)
        drawSkipRegion(std::min(skipDragStart, skipDragEnd), std::max(skipDragStart, skipDragEnd));

    const auto cursorFraction = processor.getLayerPlaybackPositionFraction(layerIndex);
    if (cursorFraction >= 0.0)
    {
        const float cursorX = bounds.getX() + bounds.getWidth() * (float) cursorFraction;
        g.setColour(Theme::purple.withAlpha(0.18f));
        g.drawLine(cursorX, bounds.getY() + 2.0f, cursorX, bounds.getBottom() - 2.0f, 4.0f);
        g.setColour(Theme::valueText.withAlpha(0.76f));
        g.drawLine(cursorX, bounds.getY() + 2.0f, cursorX, bounds.getBottom() - 2.0f, 1.35f);
        g.setColour(highlight.interpolatedWith(Theme::purple, 0.55f).withAlpha(0.50f));
        g.fillEllipse(cursorX - 1.8f, bounds.getY() + 1.0f, 3.6f, 3.6f);
    }
}

void SceneLooperAudioProcessorEditor::LayerRow::WaveformPreview::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isCtrlDown() || event.mods.isCommandDown())
    {
        selectingSkipRange = true;
        skipDragStart = fractionForPosition(event.position);
        skipDragEnd = skipDragStart;
        repaint();
        return;
    }

    seekToMousePosition(event.position);
}

void SceneLooperAudioProcessorEditor::LayerRow::WaveformPreview::mouseDrag(const juce::MouseEvent& event)
{
    if (selectingSkipRange)
    {
        skipDragEnd = fractionForPosition(event.position);
        repaint();
        return;
    }

    seekToMousePosition(event.position);
}

void SceneLooperAudioProcessorEditor::LayerRow::WaveformPreview::mouseUp(const juce::MouseEvent&)
{
    if (! selectingSkipRange)
        return;

    selectingSkipRange = false;
    processor.addLayerSkipRange(layerIndex, skipDragStart, skipDragEnd);
    skipDragStart = -1.0;
    skipDragEnd = -1.0;
    repaint();
}

double SceneLooperAudioProcessorEditor::LayerRow::WaveformPreview::fractionForPosition(juce::Point<float> position) const
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    if (bounds.getWidth() <= 0.0f)
        return 0.0;

    return juce::jlimit(0.0, 1.0, (double) ((position.x - bounds.getX()) / bounds.getWidth()));
}

void SceneLooperAudioProcessorEditor::LayerRow::WaveformPreview::seekToMousePosition(juce::Point<float> position)
{
    const auto fraction = fractionForPosition(position);
    processor.seekLayerToFraction(layerIndex, fraction);
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
    loadButton.setTooltip("Load WAV");
    fileLabel.setText(processor.getFileNameForLayer(layerIndex), juce::dontSendNotification);
    fileLabel.setColour(juce::Label::textColourId, Theme::valueText.withAlpha(0.86f));
    fileLabel.setJustificationType(juce::Justification::centredLeft);
    fileLabel.setFont(juce::Font("Avenir Next", 12.0f, juce::Font::plain));
    addAndMakeVisible(fileLabel);
    addAndMakeVisible(waveformPreview);

    addAndMakeVisible(loadButton);
    addAndMakeVisible(onButton);
    makeHitZoneOnly(onButton);
    onButton.setTooltip("Layer on/off");
    onButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    onButton.onStateChange = [this] { repaint(); };
    soloButton.setButtonText("S");
    addAndMakeVisible(soloButton);
    soloButton.setTooltip("Solo");
    autoPanButton.setButtonText("AP");
    addAndMakeVisible(autoPanButton);
    autoPanButton.setTooltip("Auto Pan");
    autopilotButton.setButtonText("AUTO");
    addAndMakeVisible(autopilotButton);
    autopilotButton.setTooltip("Autopilot: random seamless layer movement using skip zones and XFade");

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
    setupLabel(panXFadeLabel, "Pan XFade");
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
    addAndMakeVisible(panXFadeLabel);
    addAndMakeVisible(autoPanAmountLabel);
    addAndMakeVisible(autoPanRateLabel);
    addAndMakeVisible(hpLabel);
    addAndMakeVisible(lpLabel);
    addAndMakeVisible(xfadeLabel);
    addAndMakeVisible(offsetLabel);
    for (auto* label : { &volumeLabel, &panLabel, &speedLabel, &driftLabel, &widthLabel, &panXFadeLabel,
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
    setupSlider(panXFadeSlider, "%");
    setupSlider(autoPanAmountSlider, "");
    setupSlider(autoPanRateSlider, " Hz");
    setupSlider(hpSlider, " Hz");
    setupSlider(lpSlider, " Hz");
    setupSlider(xfadeSlider, " s");
    setupSlider(offsetSlider, " s");

    speedSlider.setNumDecimalPlacesToDisplay(1);
    driftSlider.setNumDecimalPlacesToDisplay(0);
    widthSlider.setNumDecimalPlacesToDisplay(0);
    panXFadeSlider.setNumDecimalPlacesToDisplay(0);
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    panSlider.textFromValueFunction = [] (double value)
    {
        return panText(value);
    };
    autoPanAmountSlider.textFromValueFunction = [] (double value)
    {
        return decimalText(value * 100.0) + "%";
    };
    autoPanAmountSlider.valueFromTextFunction = [] (const juce::String& text)
    {
        return juce::jlimit(0.0, 1.0, text.retainCharacters("0123456789.").getDoubleValue() * 0.01);
    };
    speedSlider.textFromValueFunction = [] (double value)
    {
        return juce::String(value, 1) + "k";
    };
    autoPanRateSlider.textFromValueFunction = [] (double value)
    {
        return decimalText(value) + " Hz";
    };
    offsetSlider.textFromValueFunction = [] (double value)
    {
        return decimalText(value) + " s";
    };
    xfadeSlider.textFromValueFunction = [] (double value)
    {
        return decimalText(value) + " s";
    };
    hpSlider.textFromValueFunction = [] (double value)
    {
        return value >= 1000.0 ? juce::String(value / 1000.0, 1) + "k" : juce::String((int) std::round(value)) + " Hz";
    };
    lpSlider.textFromValueFunction = hpSlider.textFromValueFunction;

    for (auto* slider : { &volumeSlider, &panSlider, &speedSlider, &driftSlider, &widthSlider, &panXFadeSlider,
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
    addAndMakeVisible(panXFadeSlider);
    addAndMakeVisible(autoPanAmountSlider);
    addAndMakeVisible(autoPanRateSlider);
    addAndMakeVisible(hpSlider);
    addAndMakeVisible(lpSlider);
    addAndMakeVisible(xfadeSlider);
    addAndMakeVisible(offsetSlider);

    onAttachment = std::make_unique<ButtonAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "on"), onButton);
    soloAttachment = std::make_unique<ButtonAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "solo"), soloButton);
    autoPanAttachment = std::make_unique<ButtonAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "autoPanOn"), autoPanButton);
    autopilotAttachment = std::make_unique<ButtonAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "autopilotOn"), autopilotButton);
    volumeAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "volume"), volumeSlider);
    panAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "pan"), panSlider);
    speedAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "speed"), speedSlider);
    driftAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "drift"), driftSlider);
    widthAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "width"), widthSlider);
    panXFadeAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "panXFade"), panXFadeSlider);
    autoPanAmountAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "autoPanAmount"), autoPanAmountSlider);
    autoPanRateAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "autoPanRate"), autoPanRateSlider);
    hpAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "hp"), hpSlider);
    lpAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "lp"), lpSlider);
    xfadeAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "xfade"), xfadeSlider);
    offsetAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "offset"), offsetSlider);

    loadButton.onClick = [this]
    {
        fileChooser = std::make_unique<juce::FileChooser>("Load audio", juce::File{}, "*.wav;*.aif;*.aiff;*.flac");
        fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& chooser)
            {
                const auto file = chooser.getResult();
                if (file.existsAsFile())
                {
                    juce::String error;
                    if (! processor.loadFileForLayer(layerIndex, file, error))
                    {
                        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Atmocycle", error);
                    }
                    else
                    {
                        setLayerOn(true);
                        refreshFileName();
                        refreshTimeDisplay();
                    }
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

void SceneLooperAudioProcessorEditor::LayerRow::setLayerOn(bool shouldBeOn)
{
    if (auto* parameter = processor.apvts.getParameter(SceneLooperAudioProcessor::paramId(layerIndex, "on")))
    {
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(shouldBeOn ? 1.0f : 0.0f));
        parameter->endChangeGesture();
    }

    onButton.setToggleState(shouldBeOn, juce::dontSendNotification);
    repaint();
}

void SceneLooperAudioProcessorEditor::LayerRow::paint(juce::Graphics&)
{
}

void SceneLooperAudioProcessorEditor::LayerRow::paintOverChildren(juce::Graphics& g)
{
    auto drawControlValue = [this, &g] (juce::Slider& slider, int width = 56)
    {
        auto value = slider.getBounds().withSizeKeepingCentre(scaledX(width), scaledY(17));
        value.setY(slider.getBottom() - scaledY(6));
        g.setColour(Theme::valueText.withAlpha(0.92f));
        g.setFont(juce::Font("Avenir Next", scaledFont(13.2f), juce::Font::plain));
        const auto text = (&slider == &panSlider) ? panText(slider.getValue()) : sliderValueText(slider);
        g.drawFittedText(text, value, juce::Justification::centred, 1);
    };

    auto autopilotAmountFor = [this] (float phaseOffset)
    {
        const auto phase = (float) std::fmod(juce::Time::getMillisecondCounterHiRes() * 0.000014
                                             + (double) layerIndex * 0.061 + phaseOffset, 1.0);
        const auto a = std::sin(phase * juce::MathConstants<float>::twoPi);
        const auto b = 0.35f * std::sin(phase * juce::MathConstants<float>::twoPi * 2.0f + phaseOffset * 3.4f);
        const auto c = 0.18f * std::sin(phase * juce::MathConstants<float>::twoPi * 3.0f + phaseOffset * 1.2f);
        const auto shaped = juce::jlimit(-1.0f, 1.0f, (a + b + c) / 1.53f);
        return 0.5f + 0.5f * shaped;
    };

    auto drawAutopilotNeedle = [this, &g, &autopilotAmountFor] (juce::Slider& slider, float phaseOffset, float depth = 0.76f)
    {
        if (! autopilotButton.getToggleState())
            return;

        const auto bounds = slider.getBounds().toFloat().reduced(scaledX(6.0f));
        const auto centre = bounds.getCentre();
        const auto radius = bounds.getWidth() * 0.34f;
        const auto amount = autopilotAmountFor(phaseOffset);
        const auto angle = -2.35f + amount * 4.70f;
        const auto marker = centre + juce::Point<float>(std::cos(angle - juce::MathConstants<float>::halfPi),
                                                        std::sin(angle - juce::MathConstants<float>::halfPi)) * radius * depth;
        g.setColour(juce::Colour(0xffff6178).withAlpha(0.22f));
        g.fillEllipse(marker.x - scaledX(5.0f), marker.y - scaledY(5.0f), scaledX(10.0f), scaledY(10.0f));
        g.setColour(juce::Colour(0xffff8ea0).withAlpha(0.90f));
        g.fillEllipse(marker.x - scaledX(2.0f), marker.y - scaledY(2.0f), scaledX(4.0f), scaledY(4.0f));
    };

    auto drawAutopilotText = [this, &g] (juce::Slider& slider, const juce::String& text, int width = 56)
    {
        if (! autopilotButton.getToggleState())
            return false;

        auto value = slider.getBounds().withSizeKeepingCentre(scaledX(width), scaledY(17));
        value.setY(slider.getBottom() - scaledY(6));
        g.setColour(juce::Colour(0xffff8ea0).withAlpha(0.95f));
        g.setFont(juce::Font("Avenir Next", scaledFont(13.2f), juce::Font::plain));
        g.drawFittedText(text, value, juce::Justification::centred, 1);
        return true;
    };

    if (processor.isLayerLoaded(layerIndex))
        drawTinyFileBadge(g, scaledBoundsF(94.0f, 8.5f, 12.0f, 12.0f), Theme::layerColour(layerIndex));

    drawTextValue(g, scaledBounds(718, 20, 52, 31), sliderValueText(volumeSlider), scaledFont(13.2f), juce::Justification::centred);
    drawControlValue(panSlider, 50);
    if (! drawAutopilotText(autoPanAmountSlider, juce::String((int) std::round(autopilotAmountFor(0.18f) * 18.0f)) + "%", 46))
        drawControlValue(autoPanAmountSlider, 46);
    if (! drawAutopilotText(autoPanRateSlider, juce::String(0.01f + autopilotAmountFor(0.35f) * 0.07f, 2) + " Hz", 52))
        drawControlValue(autoPanRateSlider, 52);
    if (! drawAutopilotText(speedSlider, juce::String(43.0f + autopilotAmountFor(0.44f) * 12.0f, 1) + "k", 50))
        drawControlValue(speedSlider, 50);
    if (! drawAutopilotText(driftSlider, juce::String(0.5f + autopilotAmountFor(0.61f) * 0.8f, 1) + "%", 44))
        drawControlValue(driftSlider, 44);
    drawControlValue(widthSlider, 48);
    if (! drawAutopilotText(panXFadeSlider, juce::String((int) std::round(autopilotAmountFor(0.52f) * 10.0f)) + "%", 54))
        drawControlValue(panXFadeSlider, 54);
    drawControlValue(offsetSlider, 58);
    drawControlValue(hpSlider, 52);
    if (! drawAutopilotText(lpSlider, juce::String(8.0f + autopilotAmountFor(0.79f) * 4.0f, 1) + "k", 52))
        drawControlValue(lpSlider, 52);
    drawControlValue(xfadeSlider, 54);

    drawAutopilotNeedle(panSlider, 0.00f);
    drawAutopilotNeedle(autoPanAmountSlider, 0.18f, 0.58f);
    drawAutopilotNeedle(autoPanRateSlider, 0.35f, 0.58f);
    drawAutopilotNeedle(speedSlider, 0.44f, 0.62f);
    drawAutopilotNeedle(driftSlider, 0.61f, 0.52f);
    drawAutopilotNeedle(panXFadeSlider, 0.52f, 0.70f);
    drawAutopilotNeedle(lpSlider, 0.79f, 0.62f);

    g.setColour(Theme::valueText.withAlpha(0.84f));
    g.setFont(juce::Font("Avenir Next", scaledFont(11.6f), juce::Font::plain));
    g.drawFittedText(lengthLabel.getText(), scaledBounds(330, 8, 88, 18),
                     juce::Justification::centredRight, 1);

    auto drawWaveBracket = [&g] (float x, bool right)
    {
        const auto y = (float) scaledY(56.0f);
        const auto h = (float) scaledY(12.0f);
        const auto arm = (float) scaledX(5.0f);
        g.setColour(Theme::mutedText.withAlpha(0.50f));
        g.drawLine(x, y, x, y + h, 1.15f);
        g.drawLine(x, y, x + (right ? -arm : arm), y, 1.15f);
        g.drawLine(x, y + h, x + (right ? -arm : arm), y + h, 1.15f);
    };
    drawWaveBracket((float) scaledX(92.0f), false);
    drawWaveBracket((float) scaledX(490.0f), true);

    auto led = scaledBoundsF(106.0f, 63.0f, 360.0f, 5.0f);
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
        g.setColour(active ? colour.withAlpha(0.24f) : juce::Colour(0xff0b2a30).withAlpha(0.07f));
        g.fillRoundedRectangle(x, led.getY(), juce::jmax(1.6f, segmentWidth - 2.0f), led.getHeight(), 0.55f);
    }

    const auto autopilotActive = autopilotButton.getToggleState();
    if (! processor.isLayerLoaded(layerIndex) || ! onButton.getToggleState() || autopilotActive)
    {
        auto dimArea = getLocalBounds().toFloat().withTrimmedLeft((float) scaledX(72.0f));
        const auto pulse = 0.45f + 0.55f * (float) std::sin(juce::Time::getMillisecondCounterHiRes() * 0.0055);
        g.setColour(juce::Colours::black.withAlpha(autopilotActive ? 0.10f : 0.34f));
        g.fillRoundedRectangle(dimArea.reduced(1.0f, 1.0f), 3.5f);
        g.setColour((autopilotActive ? juce::Colour(0xffd12a55) : juce::Colour(0xff011115)).withAlpha(autopilotActive ? 0.055f + pulse * 0.045f : 0.20f));
        g.fillRoundedRectangle(dimArea.reduced(3.0f, 5.0f), 3.0f);
        if (autopilotActive)
        {
            g.setColour(juce::Colour(0xffd12a55).withAlpha(0.12f + pulse * 0.10f));
            g.drawRoundedRectangle(dimArea.reduced(1.5f, 2.0f), 4.0f, 0.9f);
        }
    }

    if (onButton.isMouseOver())
    {
        const auto badge = scaledBoundsF(12.0f, 8.0f, 52.0f, 54.0f);
        g.setColour(Theme::layerColour(layerIndex).withAlpha(0.10f));
        g.fillRoundedRectangle(badge, scaledY(6.0f));
        g.setColour(Theme::layerHighlightColour(layerIndex).withAlpha(0.30f));
        g.drawRoundedRectangle(badge, scaledY(6.0f), scaledY(1.0f));
    }
}

void SceneLooperAudioProcessorEditor::LayerRow::resized()
{
    numberLabel.setBounds(scaledBounds(0, 0, 60, 70));

    fileLabel.setBounds(scaledBounds(110, 7, 250, 19));
    waveformPreview.setBounds(scaledBounds(92, 22, 398, 36));
    loadButton.setBounds(scaledBounds(440, 7, 50, 21));

    onButton.setBounds(scaledBounds(0, 0, 72, 70));
    soloButton.setBounds(scaledBounds(500, 19, 28, 28));
    autoPanButton.setBounds(scaledBounds(535, 19, 28, 28));
    autopilotButton.setBounds(scaledBounds(570, 19, 34, 28));

    volumeSlider.setBounds(scaledBounds(618, 21, 114, 28));

    auto placeKnob = [] (juce::Slider& slider, int centreX, int centreY, int size = 42)
    {
        slider.setBounds(scaledX(centreX - size / 2), scaledY(centreY - size / 2),
                         scaledX(size), scaledY(size));
    };

    constexpr int firstKnobCentre = 810;
    constexpr int knobStep = 80;
    placeKnob(panSlider, firstKnobCentre + knobStep * 0, 33, 48);
    placeKnob(autoPanAmountSlider, firstKnobCentre + knobStep * 1, 33, 48);
    placeKnob(autoPanRateSlider, firstKnobCentre + knobStep * 2, 33, 48);
    placeKnob(speedSlider, firstKnobCentre + knobStep * 3, 33, 48);
    placeKnob(driftSlider, firstKnobCentre + knobStep * 4, 33, 48);
    placeKnob(widthSlider, firstKnobCentre + knobStep * 5, 33, 48);
    placeKnob(panXFadeSlider, firstKnobCentre + knobStep * 6, 33, 46);
    placeKnob(offsetSlider, firstKnobCentre + knobStep * 7, 33, 48);
    placeKnob(hpSlider, firstKnobCentre + knobStep * 8, 33, 46);
    placeKnob(lpSlider, firstKnobCentre + knobStep * 9, 33, 46);
    placeKnob(xfadeSlider, firstKnobCentre + knobStep * 10, 33, 46);

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
