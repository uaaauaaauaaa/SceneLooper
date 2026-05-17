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

constexpr int designWidth = 1600;
constexpr int designHeight = 960;
constexpr int editorWidth = 1208;
constexpr int editorHeight = 725;
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

juce::Image getLoadSceneButtonImage()
{
    return juce::ImageCache::getFromMemory(BinaryData::load_scene_button_png,
                                           BinaryData::load_scene_button_pngSize);
}

juce::Image getSaveSceneButtonImage()
{
    return juce::ImageCache::getFromMemory(BinaryData::save_scene_button_png,
                                           BinaryData::save_scene_button_pngSize);
}

juce::Image getRandomizationButtonImage()
{
    return juce::ImageCache::getFromMemory(BinaryData::randomization_button_png,
                                           BinaryData::randomization_button_pngSize);
}

juce::Image getSmallKnobFramesImage()
{
    return juce::ImageCache::getFromMemory(BinaryData::small_knob_frames_png,
                                           BinaryData::small_knob_frames_pngSize);
}

juce::Image getMasterKnobFramesImage()
{
    return juce::ImageCache::getFromMemory(BinaryData::master_knob_frames_png,
                                           BinaryData::master_knob_frames_pngSize);
}

juce::Image getLayerLoadNormalImage()
{
    return juce::ImageCache::getFromMemory(BinaryData::asset_layer_load_normal_png,
                                           BinaryData::asset_layer_load_normal_pngSize);
}

juce::Image getLayerLoadHoverImage()
{
    return juce::ImageCache::getFromMemory(BinaryData::asset_layer_load_hover_png,
                                           BinaryData::asset_layer_load_hover_pngSize);
}

juce::Image getLayerLoadDownImage()
{
    return juce::ImageCache::getFromMemory(BinaryData::asset_layer_load_down_png,
                                           BinaryData::asset_layer_load_down_pngSize);
}

juce::Image getPowerOffImage()
{
    return juce::ImageCache::getFromMemory(BinaryData::asset_power_off_png,
                                           BinaryData::asset_power_off_pngSize);
}

juce::Image getPowerOnImage()
{
    return juce::ImageCache::getFromMemory(BinaryData::asset_power_on_png,
                                           BinaryData::asset_power_on_pngSize);
}

juce::Image getSoloOffImage()
{
    return juce::ImageCache::getFromMemory(BinaryData::asset_solo_off_png,
                                           BinaryData::asset_solo_off_pngSize);
}

juce::Image getSoloOnImage()
{
    return juce::ImageCache::getFromMemory(BinaryData::asset_solo_on_png,
                                           BinaryData::asset_solo_on_pngSize);
}

juce::Image getApOffImage()
{
    return juce::ImageCache::getFromMemory(BinaryData::asset_ap_off_png,
                                           BinaryData::asset_ap_off_pngSize);
}

juce::Image getApOnImage()
{
    return juce::ImageCache::getFromMemory(BinaryData::asset_ap_on_png,
                                           BinaryData::asset_ap_on_pngSize);
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

void drawAssetButton(juce::Graphics& g, const juce::Image& image, juce::Rectangle<float> bounds)
{
    if (! image.isValid())
        return;

    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    g.drawImage(image, bounds, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, false);
}

void drawAssetStretch(juce::Graphics& g, const juce::Image& image, juce::Rectangle<float> bounds)
{
    if (! image.isValid())
        return;

    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    g.drawImage(image, bounds, juce::RectanglePlacement::stretchToFit, false);
}

int smallKnobRowForAccent(juce::Colour accent)
{
    const auto hue = accent.getHue();
    const auto saturation = accent.getSaturation();
    if (saturation < 0.25f)
        return 5;
    if (hue > 0.70f)
        return 1;
    if (hue > 0.64f)
        return 0;
    if (hue > 0.55f)
        return 3;
    if (hue > 0.49f)
        return 2;
    return 4;
}

int masterKnobRowForSlider(const juce::Slider& slider)
{
    const auto name = slider.getName();
    if (name.containsIgnoreCase("Crossfade") || name.containsIgnoreCase("Random"))
        return 1;
    if (name.containsIgnoreCase("LowCut"))
        return 2;
    if (name.containsIgnoreCase("HighCut"))
        return 3;
    return 0;
}

bool drawKnobFrame(juce::Graphics& g, const juce::Image& frames, int columns, int rows, int row,
                   float sliderPos, juce::Rectangle<int> target)
{
    if (! frames.isValid() || columns <= 0 || rows <= 0)
        return false;

    const int frameWidth = frames.getWidth() / columns;
    const int frameHeight = frames.getHeight() / rows;
    const int frame = juce::jlimit(0, columns - 1, (int) std::round(sliderPos * (float) (columns - 1)));
    row = juce::jlimit(0, rows - 1, row);
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    g.drawImage(frames, target.getX(), target.getY(), target.getWidth(), target.getHeight(),
                frame * frameWidth, row * frameHeight, frameWidth, frameHeight, false);
    return true;
}

void drawPlainValue(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& text,
                    float fontSize = 12.0f, juce::Justification justification = juce::Justification::centred)
{
    g.setColour(juce::Colour(0xff051418).withAlpha(0.38f));
    g.fillRoundedRectangle(bounds.toFloat().reduced(1.0f), 4.0f);
    g.setColour(Theme::text.withAlpha(0.86f));
    g.setFont(juce::Font(fontSize, juce::Font::plain));
    g.drawFittedText(text, bounds.reduced(4, 0), justification, 1);
}

void drawFramedValue(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& text,
                     float fontSize = 14.0f, juce::Justification justification = juce::Justification::centredLeft)
{
    const auto area = bounds.toFloat().reduced(0.5f);
    g.setColour(juce::Colour(0xff020b0e).withAlpha(0.56f));
    g.fillRoundedRectangle(area, 4.0f);
    g.setColour(Theme::cyan.withAlpha(0.16f));
    g.drawRoundedRectangle(area, 4.0f, 0.9f);
    g.setColour(Theme::text.withAlpha(0.90f));
    g.setFont(juce::Font(fontSize, juce::Font::plain));
    g.drawFittedText(text, bounds.reduced(6, 0), justification, 1);
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
        const auto glow = juce::jlimit(0.0f, 1.0f, sliderPos);
        const bool useMasterSprite = width >= 48 || height >= 48;
        const auto target = juce::Rectangle<int>(x, y, width, height).withSizeKeepingCentre(
            juce::jmin(width, height), juce::jmin(width, height));

        if (useMasterSprite
                ? drawKnobFrame(g, getMasterKnobFramesImage(), 11, 4, masterKnobRowForSlider(slider), sliderPos, target)
                : drawKnobFrame(g, getSmallKnobFramesImage(), 12, 6, smallKnobRowForAccent(accent), sliderPos, target))
            return;

        const auto innerGlow = 1.6f + glow * 1.2f;
        const auto outerGlow = 2.4f + glow * 0.8f;
        g.setColour(accent.withAlpha(0.12f + glow * 0.20f));
        g.fillEllipse(r.expanded(innerGlow));
        g.setColour(accent.withAlpha(0.04f + glow * 0.09f));
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

        g.setColour(accent.withAlpha(0.38f + glow * 0.34f));
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
                            rotaryStartAngle, angle, true);
        g.setColour(accent.withAlpha(0.11f + glow * 0.14f));
        g.strokePath(value, juce::PathStrokeType(7.0f + glow * 2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(accent.withAlpha(0.68f + glow * 0.24f));
        g.strokePath(value, juce::PathStrokeType(2.4f + glow * 0.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

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

        const auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) width, (float) height).reduced(6.0f, 11.0f);
        const auto cy = bounds.getCentreY();
        const auto accent = slider.findColour(juce::Slider::trackColourId);
        g.setColour(juce::Colour(0xff02090b));
        g.fillRoundedRectangle(bounds.withHeight(5.0f).withCentre(juce::Point<float>(bounds.getCentreX(), cy)), 2.5f);
        g.setColour(accent.withAlpha(0.16f));
        g.fillRoundedRectangle(juce::Rectangle<float>(bounds.getX(), cy - 4.0f, juce::jmax(0.0f, sliderPos - bounds.getX()), 8.0f), 4.0f);
        g.setColour(accent.withAlpha(0.86f));
        g.fillRoundedRectangle(juce::Rectangle<float>(bounds.getX(), cy - 1.6f, juce::jmax(0.0f, sliderPos - bounds.getX()), 3.2f), 1.6f);
        g.setColour(Theme::text.withAlpha(0.88f));
        g.fillRoundedRectangle(sliderPos - 3.0f, cy - 8.5f, 6.0f, 17.0f, 2.0f);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&, bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
        if (button.getButtonText().equalsIgnoreCase("Load"))
        {
            drawAssetStretch(g,
                             shouldDrawButtonAsDown ? getLayerLoadDownImage()
                                                    : (shouldDrawButtonAsHighlighted ? getLayerLoadHoverImage()
                                                                                     : getLayerLoadNormalImage()),
                             bounds);
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
        if (button.getButtonText().equalsIgnoreCase("Load"))
            return;

        g.setColour(Theme::text.withAlpha(button.isEnabled() ? 0.84f : 0.35f));
        g.setFont(juce::Font(9.5f));
        g.drawFittedText(button.getButtonText(), button.getLocalBounds().reduced(5, 2),
                         juce::Justification::centred, 1);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool highlighted, bool down) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
        const auto on = button.getToggleState();
        if (button.getButtonText() == "On")
        {
            drawAssetButton(g, on ? getPowerOnImage() : getPowerOffImage(), bounds.expanded(2.0f));
            return;
        }

        if (button.getButtonText() == "S")
        {
            drawAssetButton(g, on ? getSoloOnImage() : getSoloOffImage(), bounds.expanded(1.0f));
            return;
        }

        if (button.getButtonText() == "AP")
        {
            drawAssetButton(g, on ? getApOnImage() : getApOffImage(), bounds.expanded(1.5f));
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

    for (auto* component : { static_cast<juce::Component*>(&titleLabel),
                             static_cast<juce::Component*>(&bylineLabel),
                             static_cast<juce::Component*>(&taglineLabel),
                             static_cast<juce::Component*>(&sceneCaptionLabel),
                             static_cast<juce::Component*>(&sceneNameLabel),
                             static_cast<juce::Component*>(&masterLabel),
                             static_cast<juce::Component*>(&globalXFadeLabel),
                             static_cast<juce::Component*>(&masterLowCutLabel),
                             static_cast<juce::Component*>(&masterHighCutLabel),
                             static_cast<juce::Component*>(&randomizationLabel),
                             static_cast<juce::Component*>(&randomStartLabel),
                             static_cast<juce::Component*>(&masterMeterLabel),
                             static_cast<juce::Component*>(&saveSceneButton),
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
    drawAssetButton(g, getLoadSceneButtonImage(), scaledBoundsF(932.0f, 27.0f, 137.0f, 57.0f));
    drawAssetButton(g, getSaveSceneButtonImage(), scaledBoundsF(1087.0f, 27.0f, 136.0f, 57.0f));
    drawAssetButton(g, getRandomizationButtonImage(), scaledBoundsF(26.0f, 836.0f, 270.0f, 74.0f));

    const auto sceneName = processor.getCurrentSceneName() == "Untitled Scene"
                               ? juce::String("Project State")
                               : processor.getCurrentSceneName();

    g.setColour(Theme::text.withAlpha(0.92f));
    g.setFont(juce::Font(scaledFont(16.0f), juce::Font::plain));
    g.drawFittedText(sceneName, sceneNameLabel.getBounds(), juce::Justification::centred, 1);

    drawFramedValue(g, scaledBounds(162, 157, 86, 38), sliderValueText(masterSlider), scaledFont(14.5f), juce::Justification::centredLeft);
    drawFramedValue(g, scaledBounds(545, 157, 108, 38), sliderValueText(globalXFadeSlider), scaledFont(14.5f), juce::Justification::centredLeft);
    drawFramedValue(g, scaledBounds(944, 157, 91, 38), sliderValueText(masterLowCutSlider), scaledFont(14.5f), juce::Justification::centredLeft);
    drawFramedValue(g, scaledBounds(1330, 157, 94, 38), sliderValueText(masterHighCutSlider), scaledFont(14.5f), juce::Justification::centredLeft);
    drawPlainValue(g, scaledBounds(370, 872, 82, 38), sliderValueText(randomStartSlider), scaledFont(13.0f), juce::Justification::centredLeft);

    const auto masterLevel = juce::jlimit(0.0f, 1.0f, processor.getMasterLevel());
    auto meter = scaledBoundsF(528.0f, 859.0f, 420.0f, 75.0f);
    g.setColour(juce::Colour(0xff021014).withAlpha(0.48f));
    g.fillRoundedRectangle(meter.reduced(2.0f), 6.0f);
    auto ledArea = scaledBoundsF(610.0f, 887.0f, 280.0f, 22.0f);
    g.setColour(juce::Colour(0xff021014).withAlpha(0.76f));
    g.fillRoundedRectangle(ledArea.expanded(3.0f, 2.0f), 3.0f);

    constexpr int bars = 45;
    for (int i = 0; i < bars; ++i)
    {
        const float t = (float) i / (float) (bars - 1);
        const bool active = t <= masterLevel;
        const auto colour = Theme::purple.interpolatedWith(Theme::cyan, t);
        const auto segmentWidth = ledArea.getWidth() / (float) bars;
        const auto x = ledArea.getX() + (float) i * segmentWidth;
        const auto h = juce::jmap(t, 10.0f, ledArea.getHeight());
        g.setColour(active ? colour.withAlpha(0.92f) : juce::Colour(0xff0b2a30).withAlpha(0.36f));
        g.fillRoundedRectangle(x, ledArea.getBottom() - h, juce::jmax(2.4f, segmentWidth - 2.0f), h, 0.8f);
    }

    drawPlainValue(g, scaledBounds(570, 887, 58, 26), levelToDbText(masterLevel), scaledFont(12.0f), juce::Justification::centredRight);
}

void SceneLooperAudioProcessorEditor::resized()
{
    titleLabel.setBounds(scaledBounds(132, 21, 240, 27));
    bylineLabel.setBounds(scaledBounds(132, 51, 185, 17));
    taglineLabel.setBounds(scaledBounds(132, 76, 260, 18));
    sceneCaptionLabel.setBounds(scaledBounds(733, 13, 54, 18));
    sceneNameLabel.setBounds(scaledBounds(498, 27, 418, 57));
    loadSceneButton.setBounds(scaledBounds(932, 27, 137, 57));
    saveSceneButton.setBounds(scaledBounds(1087, 27, 136, 57));

    masterLabel.setBounds(scaledBounds(162, 136, 150, 20));
    globalXFadeLabel.setBounds(scaledBounds(545, 136, 174, 20));
    masterLowCutLabel.setBounds(scaledBounds(944, 136, 160, 20));
    masterHighCutLabel.setBounds(scaledBounds(1330, 136, 160, 20));

    masterSlider.setBounds(scaledBounds(64, 120, 84, 84));
    globalXFadeSlider.setBounds(scaledBounds(444, 120, 84, 84));
    masterLowCutSlider.setBounds(scaledBounds(848, 120, 84, 84));
    masterHighCutSlider.setBounds(scaledBounds(1230, 120, 84, 84));

    randomizationLabel.setBounds(scaledBounds(96, 869, 150, 18));
    randomizeButton.setBounds(scaledBounds(28, 846, 228, 58));
    randomStartLabel.setBounds(scaledBounds(370, 849, 130, 18));
    randomStartSlider.setBounds(scaledBounds(288, 846, 68, 68));
    masterMeterLabel.setBounds(scaledBounds(600, 849, 160, 18));

    for (int i = 0; i < (int) rows.size(); ++i)
    {
        if (rows[(size_t) i] != nullptr)
        {
            rows[(size_t) i]->setBounds(scaledBounds(16, 262 + i * 69, 1568, 68));
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

    if (! hasPreview)
    {
        g.setColour(accent.withAlpha(0.16f));
        g.drawLine(bounds.getX() + 4.0f, centreY, bounds.getRight() - 4.0f, centreY, 1.0f);
        return;
    }

    g.reduceClipRegion(bounds.toNearestInt());

    const float usableHeight = bounds.getHeight() * 0.78f;
    const float pointWidth = bounds.getWidth() / (float) SceneLooperAudioProcessor::waveformPreviewPoints;
    const float displayGain = processor.getLayerWaveformDisplayGain(layerIndex);

    for (int i = 0; i < SceneLooperAudioProcessor::waveformPreviewPoints; ++i)
    {
        const float peak = juce::jlimit(0.0f, 1.0f, preview[(size_t) i] * displayGain);
        const float x = bounds.getX() + ((float) i + 0.5f) * pointWidth;
        const float y = peak * usableHeight;
        const auto waveColour = accent.interpolatedWith(Theme::cyan, (float) i / (float) SceneLooperAudioProcessor::waveformPreviewPoints);
        g.setColour(waveColour.withAlpha(0.18f));
        g.fillRoundedRectangle(x - 1.7f, centreY - y - 1.8f, 3.4f, y * 2.0f + 3.6f, 1.4f);
        g.setColour(waveColour.withAlpha(0.92f));
        g.fillRoundedRectangle(x - 0.9f, centreY - y, 1.8f, y * 2.0f, 0.8f);
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
    fileLabel.setText(processor.getFileNameForLayer(layerIndex), juce::dontSendNotification);
    fileLabel.setColour(juce::Label::textColourId, Theme::text.withAlpha(0.82f));
    fileLabel.setJustificationType(juce::Justification::centredLeft);
    fileLabel.setFont(juce::Font(9.6f, juce::Font::plain));
    addAndMakeVisible(fileLabel);
    addAndMakeVisible(waveformPreview);

    addAndMakeVisible(loadButton);
    addAndMakeVisible(onButton);
    soloButton.setButtonText("S");
    addAndMakeVisible(soloButton);
    autoPanButton.setButtonText("AP");
    addAndMakeVisible(autoPanButton);

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
        auto value = slider.getBounds().withSizeKeepingCentre(scaledX(width), scaledY(15));
        value.setY(slider.getBottom() + scaledY(1));
        g.setColour(Theme::text.withAlpha(0.78f));
        g.setFont(juce::Font(scaledFont(9.2f), juce::Font::plain));
        g.drawFittedText(sliderValueText(slider), value, juce::Justification::centred, 1);
    };

    drawPlainValue(g, scaledBounds(548, 22, 106, 29), sliderValueText(volumeSlider), scaledFont(10.5f), juce::Justification::centredRight);
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

    g.setColour(Theme::text.withAlpha(0.80f));
    g.setFont(juce::Font(scaledFont(10.0f), juce::Font::plain));
    g.drawFittedText(lengthLabel.getText(), scaledBounds(1438, 17, 104, 18),
                     juce::Justification::centredRight, 1);
    g.drawFittedText(remainLabel.getText(), scaledBounds(1438, 34, 104, 18),
                     juce::Justification::centredRight, 1);

    auto led = scaledBoundsF(1438.0f, 54.0f, 104.0f, 9.0f);
    const int segments = 28;
    const auto level = juce::jlimit(0.0f, 1.0f, processor.getLayerLevel(layerIndex));
    for (int i = 0; i < segments; ++i)
    {
        const float t = (float) i / (float) (segments - 1);
        const bool active = t <= level;
        const auto colour = Theme::purple.interpolatedWith(Theme::cyan, t);
        const auto segmentWidth = led.getWidth() / (float) segments;
        const auto x = led.getX() + (float) i * segmentWidth;
        g.setColour(active ? colour.withAlpha(0.82f) : juce::Colour(0xff0b2a30).withAlpha(0.34f));
        g.fillRoundedRectangle(x, led.getY(), juce::jmax(1.8f, segmentWidth - 1.6f), led.getHeight(), 0.55f);
    }
}

void SceneLooperAudioProcessorEditor::LayerRow::resized()
{
    numberLabel.setBounds(scaledBounds(0, 0, 60, 68));

    fileLabel.setBounds(scaledBounds(92, 8, 258, 18));
    waveformPreview.setBounds(scaledBounds(86, 24, 326, 34));
    loadButton.setBounds(scaledBounds(354, 9, 66, 24));

    onButton.setBounds(scaledBounds(426, 18, 34, 34));
    soloButton.setBounds(scaledBounds(462, 18, 34, 34));
    autoPanButton.setBounds(scaledBounds(498, 18, 34, 34));

    volumeSlider.setBounds(scaledBounds(548, 23, 106, 26));

    auto placeKnob = [] (juce::Slider& slider, int centreX, int centreY, int size = 42)
    {
        slider.setBounds(scaledX(centreX - size / 2), scaledY(centreY - size / 2),
                         scaledX(size), scaledY(size));
    };

    placeKnob(panSlider, 667, 34);
    placeKnob(autoPanAmountSlider, 747, 34);
    placeKnob(autoPanRateSlider, 824, 34);
    placeKnob(speedSlider, 903, 34);
    placeKnob(driftSlider, 980, 34);
    placeKnob(widthSlider, 1055, 34);
    placeKnob(offsetSlider, 1135, 34);
    placeKnob(hpSlider, 1214, 34);
    placeKnob(lpSlider, 1291, 34);
    placeKnob(xfadeSlider, 1370, 34);

    lengthLabel.setBounds(scaledBounds(1438, 17, 104, 18));
    remainLabel.setBounds(scaledBounds(1438, 34, 104, 18));
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
