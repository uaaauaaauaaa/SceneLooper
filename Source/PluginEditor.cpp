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

constexpr int editorWidth = 1280;
constexpr int editorHeight = 720;
constexpr int outerMargin = 12;
constexpr int topPanelHeight = 164;
constexpr int bottomStripHeight = 72;
constexpr int figmaLayerRowX = 14;
constexpr int figmaLayerRowY = 202;
constexpr int figmaLayerRowWidth = 1252;
constexpr int figmaLayerRowHeight = 52;
constexpr int figmaLayerRowPitch = 55;

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

void drawValuePill(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& text,
                   juce::Colour accent, float fontSize = 11.0f)
{
    g.setColour(accent.withAlpha(0.14f));
    g.setFont(juce::Font(fontSize + 0.4f));
    g.drawFittedText(text, bounds.translated(0, 1), juce::Justification::centred, 1);
    g.setColour(Theme::text.withAlpha(0.88f));
    g.setFont(juce::Font(fontSize));
    g.drawFittedText(text, bounds, juce::Justification::centred, 1);
}

void drawSmallCaption(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& text)
{
    g.setColour(Theme::mutedText.withAlpha(0.66f));
    g.setFont(juce::Font(7.3f));
    g.drawFittedText(text.toUpperCase(), bounds, juce::Justification::centred, 1);
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
        const auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) width, (float) height).reduced(5.0f);
        const auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
        const auto r = bounds.withSizeKeepingCentre(size, size).reduced(4.5f);
        const auto radius = r.getWidth() * 0.5f;
        const auto centre = r.getCentre();
        const auto accent = slider.findColour(juce::Slider::rotarySliderFillColourId);

        g.setColour(accent.withAlpha(0.16f));
        g.fillEllipse(r.expanded(4.0f));
        g.setColour(accent.withAlpha(0.055f));
        g.fillEllipse(r.expanded(9.0f));

        g.setColour(juce::Colours::black.withAlpha(0.58f));
        g.fillEllipse(r.translated(0.0f, 3.0f));

        juce::ColourGradient knobGradient(juce::Colour(0xff14282d), centre.x - radius * 0.38f, centre.y - radius * 0.44f,
                                          juce::Colour(0xff000203), centre.x + radius * 0.42f, centre.y + radius * 0.52f, true);
        knobGradient.addColour(0.55, juce::Colour(0xff061115));
        g.setGradientFill(knobGradient);
        g.fillEllipse(r);

        g.setColour(juce::Colours::white.withAlpha(0.10f));
        g.fillEllipse(r.reduced(radius * 0.24f).withTrimmedBottom(radius * 0.55f).translated(-radius * 0.10f, -radius * 0.10f));

        g.setColour(accent.withAlpha(0.58f));
        g.drawEllipse(r.expanded(1.4f), 1.0f);
        g.setColour(Theme::stroke.withAlpha(0.48f));
        g.drawEllipse(r, 0.8f);

        juce::Path track;
        track.addCentredArc(centre.x, centre.y, radius - 4.0f, radius - 4.0f, 0.0f,
                            rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colours::black.withAlpha(0.42f));
        g.strokePath(track, juce::PathStrokeType(3.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        juce::Path value;
        value.addCentredArc(centre.x, centre.y, radius - 4.0f, radius - 4.0f, 0.0f,
                            rotaryStartAngle, angle, true);
        g.setColour(accent.withAlpha(0.16f));
        g.strokePath(value, juce::PathStrokeType(7.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(accent.withAlpha(0.88f));
        g.strokePath(value, juce::PathStrokeType(2.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

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
        g.setColour(Theme::text.withAlpha(button.isEnabled() ? 0.84f : 0.35f));
        g.setFont(juce::Font(9.5f));
        g.drawFittedText(button.getButtonText(), button.getLocalBounds().reduced(5, 2),
                         juce::Justification::centred, 1);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool highlighted, bool down) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
        const auto on = button.getToggleState();
        if (button.getButtonText() == "AP")
        {
            auto base = on ? Theme::panel.brighter(0.16f) : Theme::panelDeep;
            if (highlighted || down)
                base = base.brighter(0.14f);

            juce::ColourGradient gradient(base.brighter(0.16f), bounds.getX(), bounds.getY(),
                                          Theme::panelDeep, bounds.getRight(), bounds.getBottom(), false);
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(bounds, 8.0f);
            g.setColour((on ? Theme::cyan : Theme::stroke).withAlpha(on ? 0.95f : 0.72f));
            g.drawRoundedRectangle(bounds, 8.0f, 1.0f);

            if (on)
            {
                juce::Path wave;
                const auto waveArea = bounds.reduced(6.0f, 8.0f);
                for (int i = 0; i < 24; ++i)
                {
                    const float t = (float) i / 23.0f;
                    const float x = waveArea.getX() + t * waveArea.getWidth();
                    const float y = waveArea.getCentreY() + std::sin(t * juce::MathConstants<float>::twoPi) * waveArea.getHeight() * 0.32f;
                    if (i == 0)
                        wave.startNewSubPath(x, y);
                    else
                        wave.lineTo(x, y);
                }
                g.setColour(Theme::purple.withAlpha(0.35f));
                g.strokePath(wave, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                g.setColour(Theme::cyan.withAlpha(0.95f));
                g.strokePath(wave, juce::PathStrokeType(1.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }

            g.setColour(Theme::text.withAlpha(on ? 0.96f : 0.72f));
            g.setFont(juce::Font(9.6f, juce::Font::bold));
            g.drawFittedText("AP", button.getLocalBounds().reduced(3), juce::Justification::centred, 1);
            return;
        }

        if (button.getButtonText() == "On")
        {
            auto circle = bounds.withSizeKeepingCentre(20.0f, 20.0f);
            juce::ColourGradient gradient(juce::Colour(0xff102329), circle.getX(), circle.getY(),
                                          juce::Colour(0xff010405), circle.getRight(), circle.getBottom(), true);
            g.setGradientFill(gradient);
            g.fillEllipse(circle);
            g.setColour((on ? Theme::purple : Theme::stroke).withAlpha(on ? 0.78f : 0.42f));
            g.drawEllipse(circle, 1.0f);
            g.setColour((on ? Theme::purple : Theme::mutedText).withAlpha(on ? 0.36f : 0.20f));
            g.drawEllipse(circle.expanded(3.0f), 1.0f);
            g.setColour(Theme::text.withAlpha(on ? 0.86f : 0.50f));
            g.setFont(juce::Font(11.0f));
            g.drawFittedText(juce::String::fromUTF8("\xe2\x8f\xbb"), circle.toNearestInt().reduced(2),
                             juce::Justification::centred, 1);
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
        makeHitZoneOnly(*rows[(size_t) i]);
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
                             static_cast<juce::Component*>(&randomizeButton),
                             static_cast<juce::Component*>(&masterSlider),
                             static_cast<juce::Component*>(&globalXFadeSlider),
                             static_cast<juce::Component*>(&masterLowCutSlider),
                             static_cast<juce::Component*>(&masterHighCutSlider),
                             static_cast<juce::Component*>(&randomStartSlider) })
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
    const auto sceneName = processor.getCurrentSceneName() == "Untitled Scene"
                               ? juce::String("Project State")
                               : processor.getCurrentSceneName();

    auto sceneBounds = sceneNameLabel.getBounds().toFloat().expanded(10.0f, 5.0f);
    g.setColour(juce::Colour(0xff06191d).withAlpha(0.82f));
    g.fillRoundedRectangle(sceneBounds, 5.5f);
    g.setColour(Theme::text.withAlpha(0.92f));
    g.setFont(juce::Font(13.0f, juce::Font::plain));
    g.drawFittedText(sceneName, sceneNameLabel.getBounds(), juce::Justification::centred, 1);

    const auto masterLevel = juce::jlimit(0.0f, 1.0f, processor.getMasterLevel());
    auto meter = juce::Rectangle<float>(486.0f, 674.0f, 171.0f, 18.0f);
    g.setColour(juce::Colour(0xff021014).withAlpha(0.76f));
    g.fillRoundedRectangle(meter.expanded(3.0f, 2.0f), 3.0f);

    constexpr int bars = 34;
    for (int i = 0; i < bars; ++i)
    {
        const float t = (float) i / (float) (bars - 1);
        const bool active = t <= masterLevel;
        const auto colour = Theme::purple.interpolatedWith(Theme::cyan, t);
        const auto x = meter.getX() + (float) i * 5.0f;
        const auto h = juce::jmap(t, 7.0f, 18.0f);
        g.setColour(active ? colour.withAlpha(0.92f) : juce::Colour(0xff0b2a30).withAlpha(0.36f));
        g.fillRoundedRectangle(x, meter.getBottom() - h, 3.0f, h, 0.7f);
    }

    g.setColour(Theme::text.withAlpha(0.72f));
    g.setFont(juce::Font(10.0f, juce::Font::plain));
    g.drawFittedText(levelToDbText(masterLevel), juce::Rectangle<int>(713, 670, 72, 16),
                     juce::Justification::centredLeft, 1);
}

void SceneLooperAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(Theme::outerMargin);
    auto fullTop = area.removeFromTop(Theme::topPanelHeight);
    auto top = fullTop.reduced(16, 12);

    auto brand = top.removeFromLeft(360);
    titleLabel.setBounds(brand.removeFromTop(31).withTrimmedLeft(84));
    bylineLabel.setBounds(brand.removeFromTop(18).withTrimmedLeft(84));
    taglineLabel.setBounds(brand.removeFromTop(18).withTrimmedLeft(84));

    auto scene = top.removeFromLeft(335).reduced(8, 0);
    sceneCaptionLabel.setBounds(scene.removeFromTop(16));
    sceneNameLabel.setBounds(scene.removeFromTop(38).reduced(0, 3));

    auto sceneButtons = top.removeFromLeft(265).reduced(4, 0);
    sceneButtons.removeFromTop(19);
    auto sceneButtonRow = sceneButtons.removeFromTop(40);
    loadSceneButton.setBounds(sceneButtonRow.removeFromLeft(126).reduced(5, 0));
    saveSceneButton.setBounds(sceneButtonRow.removeFromLeft(126).reduced(5, 0));

    auto macroArea = fullTop.withTrimmedTop(84).reduced(28, 0);
    auto placeMacro = [&macroArea] (juce::Label& label, juce::Slider& slider, int width)
    {
        auto column = macroArea.removeFromLeft(width).reduced(8, 0);
        label.setBounds(column.removeFromTop(14));
        auto knobArea = column.removeFromTop(62);
        slider.setBounds(knobArea.withSizeKeepingCentre(58, 58));
    };
    placeMacro(masterLabel, masterSlider, 305);
    placeMacro(globalXFadeLabel, globalXFadeSlider, 315);
    placeMacro(masterLowCutLabel, masterLowCutSlider, 315);
    placeMacro(masterHighCutLabel, masterHighCutSlider, 305);

    auto bottom = area.removeFromBottom(Theme::bottomStripHeight).reduced(18, 6);
    auto randomBlock = bottom.removeFromLeft(330);
    randomizationLabel.setBounds(randomBlock.removeFromLeft(128).reduced(4, 16));
    randomizeButton.setBounds(randomBlock.removeFromLeft(100).reduced(8, 16));
    randomStartLabel.setBounds(randomBlock.removeFromTop(16));
    randomStartSlider.setBounds(randomBlock.withSizeKeepingCentre(44, 44).translated(-16, 1));
    masterMeterLabel.setBounds(bottom.withX(462).withWidth(220).removeFromTop(18));

    for (int i = 0; i < (int) rows.size(); ++i)
    {
        if (rows[(size_t) i] != nullptr)
        {
            rows[(size_t) i]->setBounds(Theme::figmaLayerRowX,
                                        Theme::figmaLayerRowY + i * Theme::figmaLayerRowPitch,
                                        Theme::figmaLayerRowWidth,
                                        Theme::figmaLayerRowHeight);
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
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    const auto accent = Theme::layerColour(layerIndex);
    const auto highlight = Theme::layerHighlightColour(layerIndex);

    juce::ColourGradient bg(juce::Colour(0xff020c0e), bounds.getX(), bounds.getY(),
                            juce::Colour(0xff061c20), bounds.getRight(), bounds.getBottom(), false);
    bg.addColour(0.50, accent.withAlpha(0.10f));
    g.setGradientFill(bg);
    g.fillRoundedRectangle(bounds, 5.0f);

    g.setColour(accent.withAlpha(0.16f));
    g.drawRoundedRectangle(bounds, 5.0f, 0.8f);

    std::array<float, SceneLooperAudioProcessor::waveformPreviewPoints> preview;
    const bool hasPreview = processor.copyWaveformPreview(layerIndex, preview);
    const float centreY = bounds.getCentreY();

    if (! hasPreview)
    {
        g.setColour(accent.withAlpha(0.20f));
        g.drawLine(bounds.getX() + 5.0f, centreY, bounds.getRight() - 5.0f, centreY, 1.4f);
        return;
    }

    const float usableHeight = bounds.getHeight() * 0.56f;
    const float pointWidth = bounds.getWidth() / (float) SceneLooperAudioProcessor::waveformPreviewPoints;
    const float displayGain = processor.getLayerWaveformDisplayGain(layerIndex);

    for (int i = 0; i < SceneLooperAudioProcessor::waveformPreviewPoints; ++i)
    {
        const float peak = juce::jlimit(0.0f, 1.0f, preview[(size_t) i] * displayGain);
        const float x = bounds.getX() + ((float) i + 0.5f) * pointWidth;
        const float y = peak * usableHeight;
        const auto waveColour = accent.interpolatedWith(Theme::cyan, (float) i / (float) SceneLooperAudioProcessor::waveformPreviewPoints);
        g.setColour(waveColour.withAlpha(0.13f));
        g.fillRoundedRectangle(x - 1.45f, centreY - y - 1.4f, 2.9f, y * 2.0f + 2.8f, 1.2f);
        g.setColour(waveColour.withAlpha(0.82f));
        g.fillRoundedRectangle(x - 0.75f, centreY - y, 1.5f, y * 2.0f, 0.7f);
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

    const auto layerLevel = juce::jlimit(0.0f, 1.0f, processor.getLayerLevel(layerIndex));
    const auto meterBounds = bounds.removeFromBottom(7.0f).reduced(4.0f, 1.0f);
    const int segments = 26;
    for (int i = 0; i < segments; ++i)
    {
        const float t = (float) i / (float) (segments - 1);
        const bool active = t <= layerLevel;
        const auto colour = accent.interpolatedWith(Theme::cyan, t);
        const float x = meterBounds.getX() + t * meterBounds.getWidth();
        g.setColour(active ? colour.withAlpha(0.72f) : juce::Colour(0xff10282d).withAlpha(0.36f));
        g.fillRoundedRectangle(x, meterBounds.getY() + 1.0f, 2.0f, meterBounds.getHeight() - 2.0f, 0.4f);
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

void SceneLooperAudioProcessorEditor::LayerRow::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    const auto accent = Theme::layerColour(layerIndex);
    const auto highlight = Theme::layerHighlightColour(layerIndex);
    juce::ColourGradient rowGradient(juce::Colour(0xff06181b).withAlpha(0.95f), r.getX(), r.getY(),
                                     juce::Colour(0xff010709).withAlpha(0.99f), r.getRight(), r.getBottom(), false);
    rowGradient.addColour(0.52, juce::Colour(0xff030f12).withAlpha(0.99f));
    g.setGradientFill(rowGradient);
    g.fillRoundedRectangle(r.reduced(0.0f, 1.5f), 7.0f);

    g.setColour(accent.withAlpha(0.035f));
    g.fillRoundedRectangle(r.reduced(7.0f, 6.0f), 7.0f);

    juce::ColourGradient stripGradient(accent.withAlpha(0.84f), r.getX(), r.getY(),
                                       highlight.withAlpha(0.76f), r.getX(), r.getBottom(), false);
    stripGradient.addColour(0.48, accent.brighter(0.18f).withAlpha(0.96f));
    g.setGradientFill(stripGradient);
    g.fillRoundedRectangle(r.withWidth(4.0f).reduced(0.0f, 4.0f), 3.0f);
    g.setColour(highlight.withAlpha(0.18f));
    g.fillRoundedRectangle(r.withWidth(18.0f).reduced(0.0f, 4.0f), 7.0f);
    juce::ColourGradient numberGradient(accent.withAlpha(0.16f), r.getX(), r.getY(),
                                        Theme::panelDeep.withAlpha(0.03f), r.getX() + 74.0f, r.getBottom(), false);
    g.setGradientFill(numberGradient);
    g.fillRoundedRectangle(r.withWidth(62.0f).reduced(0.0f, 3.0f), 7.0f);

    auto fileBlock = fileLabel.getBounds()
                         .getUnion(waveformPreview.getBounds())
                         .getUnion(loadButton.getBounds())
                         .toFloat()
                         .expanded(4.0f, 4.0f);
    juce::ColourGradient fileGradient(juce::Colour(0xff061a1e).withAlpha(0.56f), fileBlock.getX(), fileBlock.getY(),
                                      juce::Colour(0xff02090b).withAlpha(0.76f), fileBlock.getRight(), fileBlock.getBottom(), false);
    fileGradient.addColour(0.50, accent.withAlpha(0.055f));
    g.setGradientFill(fileGradient);
    g.fillRoundedRectangle(fileBlock, 6.0f);
    g.setColour(accent.withAlpha(0.14f));
    g.drawRoundedRectangle(fileBlock, 6.0f, 0.65f);

    auto switchBlock = onButton.getBounds().getUnion(soloButton.getBounds()).toFloat().expanded(5.0f, 10.0f);
    g.setColour(juce::Colour(0xff061719).withAlpha(0.42f));
    g.fillRoundedRectangle(switchBlock, 6.0f);
    g.setColour(Theme::stroke.withAlpha(0.16f));
    g.drawRoundedRectangle(switchBlock, 6.0f, 0.65f);

    const float dividerAlpha = 0.10f;
    for (auto x : { fileBlock.getRight() + 8.0f, switchBlock.getRight() + 7.0f,
                    autoPanButton.getX() - 5.0f, speedSlider.getX() - 6.0f,
                    hpSlider.getX() - 6.0f, lengthLabel.getX() - 8.0f })
    {
        juce::ColourGradient divider(Theme::cyan.withAlpha(0.0f), x, r.getY() + 7.0f,
                                     Theme::cyan.withAlpha(dividerAlpha), x, r.getCentreY(), false);
        divider.addColour(1.0, Theme::cyan.withAlpha(0.0f));
        g.setGradientFill(divider);
        g.drawLine(x, r.getY() + 7.0f, x, r.getBottom() - 7.0f, 0.55f);
    }

    auto status = lengthLabel.getBounds().getUnion(remainLabel.getBounds()).toFloat().expanded(2.0f, 2.0f);
    auto led = status.removeFromBottom(11.0f).reduced(3.0f, 2.0f);
    const int segments = 24;
    const auto level = juce::jlimit(0.0f, 1.0f, processor.getLayerLevel(layerIndex));
    for (int i = 0; i < segments; ++i)
    {
        const float t = (float) i / (float) (segments - 1);
        const bool active = t <= level;
        const auto colour = Theme::purple.interpolatedWith(Theme::cyan, t);
        const float x = led.getX() + t * led.getWidth();
        g.setColour(active ? colour.withAlpha(0.74f) : juce::Colour(0xff10282d).withAlpha(0.35f));
        g.fillRoundedRectangle(x, led.getY(), 1.7f, led.getHeight(), 0.35f);
    }

    g.setColour(Theme::stroke.withAlpha(0.22f));
    g.drawRoundedRectangle(r.reduced(0.5f, 1.5f), 7.0f, 0.65f);
}

void SceneLooperAudioProcessorEditor::LayerRow::paintOverChildren(juce::Graphics& g)
{
    const auto accent = Theme::layerColour(layerIndex);

    auto drawControl = [&g, accent] (juce::Slider& slider, const juce::String& caption, int width = 52)
    {
        drawSmallCaption(g, slider.getBounds().withSizeKeepingCentre(width + 10, 9).translated(0, -5), caption);
        auto value = slider.getBounds().withSizeKeepingCentre(width, 15);
        value.setY(slider.getBottom() - 1);
        drawValuePill(g, value, sliderValueText(slider), accent, 9.8f);
    };

    drawControl(volumeSlider, "Volume", 64);
    drawControl(panSlider, "Pan", 48);
    drawSmallCaption(g, autoPanButton.getBounds().getUnion(autoPanRateSlider.getBounds()).withSizeKeepingCentre(132, 9).translated(0, -8), "Auto Pan");
    drawSmallCaption(g, autoPanButton.getBounds().withSizeKeepingCentre(30, 9).translated(0, -5), "On");
    drawControl(autoPanAmountSlider, "Amount", 48);
    drawControl(autoPanRateSlider, "Rate", 54);
    drawControl(speedSlider, "Speed", 50);
    drawControl(driftSlider, "Drift", 48);
    drawControl(widthSlider, "Width", 48);
    drawControl(offsetSlider, "Start", 64);
    drawControl(hpSlider, "HP", 52);
    drawControl(lpSlider, "LP", 52);
    drawControl(xfadeSlider, "XFade", 54);
}

void SceneLooperAudioProcessorEditor::LayerRow::resized()
{
    auto area = getLocalBounds().reduced(8, 4);
    numberLabel.setBounds(area.removeFromLeft(48));

    auto fileArea = area.removeFromLeft(282).reduced(2, 0);
    auto fileTop = fileArea.removeFromTop(17);
    loadButton.setBounds(fileTop.removeFromRight(52).reduced(3, 1));
    fileLabel.setBounds(fileTop);
    waveformPreview.setBounds(fileArea.reduced(0, 1));

    auto switchArea = area.removeFromLeft(58).reduced(2, 9);
    onButton.setBounds(switchArea.removeFromLeft(27).reduced(1, 7));
    soloButton.setBounds(switchArea.removeFromLeft(27).reduced(1, 7));

    auto placeControl = [] (juce::Rectangle<int>& group, juce::Slider& slider, int width, int knobSize = 43)
    {
        auto column = group.removeFromLeft(width).reduced(2, 0);
        slider.setBounds(column.withTrimmedTop(14).withTrimmedBottom(2).withSizeKeepingCentre(knobSize, knobSize));
    };

    auto statusArea = area.removeFromRight(104).reduced(2, 0);
    auto statusTop = statusArea.removeFromTop(20);
    lengthLabel.setBounds(statusTop);
    remainLabel.setBounds(statusArea.withTrimmedTop(15));

    auto volumeColumn = area.removeFromLeft(88).reduced(3, 0);
    volumeSlider.setBounds(volumeColumn.withTrimmedTop(19).withTrimmedBottom(8));
    placeControl(area, panSlider, 50, 36);

    auto autoPan = area.removeFromLeft(142).reduced(2, 0);
    autoPanButton.setBounds(autoPan.removeFromLeft(31).reduced(2, 22));
    placeControl(autoPan, autoPanAmountSlider, 52, 36);
    placeControl(autoPan, autoPanRateSlider, 55, 36);

    placeControl(area, speedSlider, 52, 36);
    placeControl(area, driftSlider, 48, 36);
    placeControl(area, widthSlider, 48, 36);
    placeControl(area, offsetSlider, 60, 36);
    placeControl(area, hpSlider, 48, 36);
    placeControl(area, lpSlider, 48, 36);
    placeControl(area, xfadeSlider, 54, 36);
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
