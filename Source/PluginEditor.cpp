#include "PluginEditor.h"

#include <cmath>

namespace
{
namespace Theme
{
const juce::Colour background { 0xff061619 };
const juce::Colour panel { 0xff0b2427 };
const juce::Colour panelDeep { 0xff071417 };
const juce::Colour stroke { 0xff173d45 };
const juce::Colour cyan { 0xff28e6ff };
const juce::Colour blue { 0xff347dff };
const juce::Colour purple { 0xffa64bff };
const juce::Colour text { 0xfff1efe0 };
const juce::Colour mutedText { 0xffa8bab3 };

constexpr int editorWidth = 1480;
constexpr int editorHeight = 840;
constexpr int outerMargin = 12;
constexpr int topPanelHeight = 142;
constexpr int headerHeight = 24;
constexpr int bottomStripHeight = 74;
constexpr int rowHeight = 71;

juce::Colour layerColour(int layerIndex)
{
    constexpr float hues[] = { 0.78f, 0.74f, 0.70f, 0.65f, 0.60f, 0.56f, 0.52f, 0.49f };
    return juce::Colour::fromHSV(hues[(size_t) juce::jlimit(0, 7, layerIndex)], 0.78f, 0.98f, 1.0f);
}

void fillVerticalGradient(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour top, juce::Colour bottom)
{
    juce::ColourGradient gradient(top, bounds.getCentreX(), bounds.getY(), bottom, bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, 10.0f);
}
}

void drawValuePill(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& text,
                   juce::Colour accent, float fontSize = 11.0f)
{
    g.setColour(accent.withAlpha(0.24f));
    g.setFont(juce::Font(fontSize + 0.6f, juce::Font::bold));
    g.drawFittedText(text, bounds.translated(0, 1), juce::Justification::centred, 1);
    g.setColour(Theme::text.withAlpha(0.94f));
    g.setFont(juce::Font(fontSize, juce::Font::bold));
    g.drawFittedText(text, bounds, juce::Justification::centred, 1);
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
        const auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) width, (float) height).reduced(3.0f);
        const auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
        const auto r = bounds.withSizeKeepingCentre(size, size).reduced(2.5f);
        const auto radius = r.getWidth() * 0.5f;
        const auto centre = r.getCentre();
        const auto accent = slider.findColour(juce::Slider::rotarySliderFillColourId);

        g.setColour(accent.withAlpha(0.20f));
        g.fillEllipse(r.expanded(8.0f));
        g.setColour(accent.withAlpha(0.10f));
        g.fillEllipse(r.expanded(13.0f));

        g.setColour(juce::Colour(0xff02090b).withAlpha(0.82f));
        g.fillEllipse(r.translated(0.0f, 3.0f));

        juce::ColourGradient knobGradient(juce::Colour(0xff1a2a30), centre.x - radius * 0.35f, centre.y - radius * 0.45f,
                                          juce::Colour(0xff050b0d), centre.x + radius * 0.45f, centre.y + radius * 0.5f, true);
        g.setGradientFill(knobGradient);
        g.fillEllipse(r);

        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.fillEllipse(r.reduced(radius * 0.16f).withTrimmedBottom(radius * 0.42f));

        g.setColour(accent.withAlpha(0.45f));
        g.drawEllipse(r.expanded(1.5f), 1.1f);
        g.setColour(Theme::stroke.withAlpha(0.72f));
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
        g.setColour(accent.withAlpha(0.28f));
        g.strokePath(value, juce::PathStrokeType(10.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(accent);
        g.strokePath(value, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        const auto indicator = centre + juce::Point<float>(std::cos(angle - juce::MathConstants<float>::halfPi),
                                                           std::sin(angle - juce::MathConstants<float>::halfPi)) * (radius - 9.0f);
        g.setColour(accent.withAlpha(0.45f));
        g.fillEllipse(indicator.x - 4.0f, indicator.y - 4.0f, 8.0f, 8.0f);
        g.setColour(Theme::text.withAlpha(0.92f));
        g.fillEllipse(indicator.x - 2.3f, indicator.y - 2.3f, 4.6f, 4.6f);
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
        g.setColour(juce::Colour(0xff041012));
        g.fillRoundedRectangle(bounds.withHeight(5.0f).withCentre(juce::Point<float>(bounds.getCentreX(), cy)), 2.5f);
        g.setColour(accent.withAlpha(0.85f));
        g.fillRoundedRectangle(juce::Rectangle<float>(bounds.getX(), cy - 2.5f, juce::jmax(0.0f, sliderPos - bounds.getX()), 5.0f), 2.5f);
        g.setColour(Theme::text);
        g.fillRoundedRectangle(sliderPos - 3.5f, cy - 11.0f, 7.0f, 22.0f, 2.5f);
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
        g.setColour(Theme::text.withAlpha(button.isEnabled() ? 0.92f : 0.35f));
        g.setFont(juce::Font(11.0f, juce::Font::bold));
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

        auto base = on ? Theme::purple.withAlpha(0.55f) : Theme::panelDeep;
        if (highlighted || down)
            base = base.brighter(0.12f);

        g.setColour(base);
        g.fillRoundedRectangle(bounds, 5.0f);
        g.setColour((on ? Theme::cyan : Theme::stroke).withAlpha(0.9f));
        g.drawRoundedRectangle(bounds, 5.0f, 1.0f);
        g.setColour(Theme::text.withAlpha(on ? 0.98f : 0.72f));
        g.setFont(juce::Font(10.0f, juce::Font::bold));
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
    titleLabel.setFont(juce::Font(25.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, Theme::text);
    bylineLabel.setText("by Echosynthesis", juce::dontSendNotification);
    bylineLabel.setFont(juce::Font(12.0f, juce::Font::plain));
    bylineLabel.setColour(juce::Label::textColourId, Theme::text.withAlpha(0.76f));
    taglineLabel.setText("Multi-Layer Ambience Engine", juce::dontSendNotification);
    taglineLabel.setFont(juce::Font(11.0f, juce::Font::plain));
    taglineLabel.setColour(juce::Label::textColourId, Theme::purple);
    sceneCaptionLabel.setText("SCENE", juce::dontSendNotification);
    sceneCaptionLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    sceneCaptionLabel.setJustificationType(juce::Justification::centred);
    sceneCaptionLabel.setColour(juce::Label::textColourId, Theme::mutedText);
    sceneNameLabel.setText(processor.getCurrentSceneName(), juce::dontSendNotification);
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
        label->setFont(juce::Font(11.0f, juce::Font::bold));
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
    g.fillAll(Theme::background);

    auto bounds = getLocalBounds().toFloat().reduced(8.0f);
    juce::ColourGradient backgroundGlow(juce::Colour(0xff0b2b2e), bounds.getX(), bounds.getY(),
                                        juce::Colour(0xff050d11), bounds.getRight(), bounds.getBottom(), false);
    backgroundGlow.addColour(0.55, juce::Colour(0xff071b25));
    g.setGradientFill(backgroundGlow);
    g.fillRoundedRectangle(bounds, 14.0f);

    g.setColour(Theme::stroke.withAlpha(0.85f));
    g.drawRoundedRectangle(bounds, 14.0f, 1.2f);

    const auto topPanel = getLocalBounds().reduced(Theme::outerMargin).removeFromTop(Theme::topPanelHeight).toFloat();
    Theme::fillVerticalGradient(g, topPanel, juce::Colour(0xff0d3034).withAlpha(0.92f), Theme::panelDeep);
    g.setColour(Theme::cyan.withAlpha(0.06f));
    g.fillRoundedRectangle(topPanel.reduced(8.0f).withTrimmedBottom(topPanel.getHeight() * 0.42f), 8.0f);
    g.setColour(Theme::stroke.withAlpha(0.85f));
    g.drawRoundedRectangle(topPanel, 9.0f, 1.0f);

    g.setColour(Theme::cyan.withAlpha(0.14f));
    g.fillEllipse(34.0f, 22.0f, 62.0f, 62.0f);
    g.setColour(Theme::purple.withAlpha(0.9f));
    g.drawEllipse(35.0f, 23.0f, 60.0f, 60.0f, 1.5f);
    g.setColour(Theme::cyan.withAlpha(0.85f));
    g.drawLine(55.0f, 54.0f, 76.0f, 43.0f, 1.5f);
    g.drawLine(76.0f, 43.0f, 85.0f, 62.0f, 1.5f);
    g.drawLine(55.0f, 54.0f, 85.0f, 62.0f, 0.8f);

    auto masterPanel = topPanel.withTrimmedTop(62.0f).reduced(12.0f, 6.0f);
    juce::ColourGradient masterGradient(juce::Colour(0xff10383c).withAlpha(0.72f), masterPanel.getX(), masterPanel.getY(),
                                        juce::Colour(0xff061417).withAlpha(0.84f), masterPanel.getRight(), masterPanel.getBottom(), false);
    g.setGradientFill(masterGradient);
    g.fillRoundedRectangle(masterPanel, 7.0f);
    g.setColour(Theme::cyan.withAlpha(0.08f));
    g.fillRoundedRectangle(masterPanel.reduced(2.0f).withTrimmedBottom(masterPanel.getHeight() * 0.50f), 6.0f);
    g.setColour(Theme::stroke.withAlpha(0.58f));
    g.drawRoundedRectangle(masterPanel, 7.0f, 0.9f);

    auto sceneGroup = sceneNameLabel.getBounds().getUnion(loadSceneButton.getBounds()).getUnion(saveSceneButton.getBounds()).toFloat().expanded(16.0f, 10.0f);
    juce::ColourGradient sceneGroupGradient(juce::Colour(0xff0b2e34), sceneGroup.getX(), sceneGroup.getY(),
                                            juce::Colour(0xff061316), sceneGroup.getRight(), sceneGroup.getBottom(), false);
    g.setGradientFill(sceneGroupGradient);
    g.fillRoundedRectangle(sceneGroup, 8.0f);
    g.setColour(Theme::cyan.withAlpha(0.07f));
    g.fillRoundedRectangle(sceneGroup.reduced(2.0f).withTrimmedBottom(sceneGroup.getHeight() * 0.55f), 7.0f);
    g.setColour(Theme::stroke.withAlpha(0.76f));
    g.drawRoundedRectangle(sceneGroup, 8.0f, 1.0f);

    auto sceneBox = sceneNameLabel.getBounds().toFloat().expanded(22.0f, 9.0f);
    juce::ColourGradient sceneGradient(juce::Colour(0xff0d3338), sceneBox.getX(), sceneBox.getY(),
                                       juce::Colour(0xff071417), sceneBox.getRight(), sceneBox.getBottom(), false);
    g.setGradientFill(sceneGradient);
    g.fillRoundedRectangle(sceneBox, 7.0f);
    g.setColour(Theme::cyan.withAlpha(0.09f));
    g.fillRoundedRectangle(sceneBox.reduced(2.0f).withTrimmedBottom(sceneBox.getHeight() * 0.58f), 6.0f);
    g.setColour(Theme::stroke.withAlpha(0.9f));
    g.drawRoundedRectangle(sceneBox, 7.0f, 1.0f);
    g.setColour(Theme::mutedText.withAlpha(0.55f));
    g.drawLine(sceneBox.getRight() - 42.0f, sceneBox.getY() + 6.0f, sceneBox.getRight() - 42.0f, sceneBox.getBottom() - 6.0f, 1.0f);
    juce::Path chevron;
    chevron.startNewSubPath(sceneBox.getRight() - 28.0f, sceneBox.getCentreY() - 3.0f);
    chevron.lineTo(sceneBox.getRight() - 22.0f, sceneBox.getCentreY() + 3.0f);
    chevron.lineTo(sceneBox.getRight() - 16.0f, sceneBox.getCentreY() - 3.0f);
    g.setColour(Theme::text.withAlpha(0.72f));
    g.strokePath(chevron, juce::PathStrokeType(1.3f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    juce::Path leftArrow;
    leftArrow.startNewSubPath(sceneBox.getX() + 24.0f, sceneBox.getCentreY() - 6.0f);
    leftArrow.lineTo(sceneBox.getX() + 18.0f, sceneBox.getCentreY());
    leftArrow.lineTo(sceneBox.getX() + 24.0f, sceneBox.getCentreY() + 6.0f);
    g.setColour(Theme::cyan.withAlpha(0.64f));
    g.strokePath(leftArrow, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const auto bottomStrip = getLocalBounds().reduced(Theme::outerMargin).removeFromBottom(Theme::bottomStripHeight).toFloat();
    Theme::fillVerticalGradient(g, bottomStrip, juce::Colour(0xff0d3438), juce::Colour(0xff071a1e));
    g.setColour(Theme::cyan.withAlpha(0.07f));
    g.fillRoundedRectangle(bottomStrip.reduced(8.0f).withTrimmedBottom(bottomStrip.getHeight() * 0.55f), 8.0f);
    g.setColour(Theme::stroke.withAlpha(0.88f));
    g.drawRoundedRectangle(bottomStrip, 9.0f, 1.0f);

    auto randomPanel = bottomStrip.withWidth(355.0f).reduced(10.0f, 10.0f);
    juce::ColourGradient randomGradient(juce::Colour(0xff113d42), randomPanel.getX(), randomPanel.getY(),
                                        juce::Colour(0xff07171b), randomPanel.getRight(), randomPanel.getBottom(), false);
    g.setGradientFill(randomGradient);
    g.fillRoundedRectangle(randomPanel, 7.0f);
    g.setColour(Theme::cyan.withAlpha(0.08f));
    g.fillRoundedRectangle(randomPanel.reduced(2.0f).withTrimmedBottom(randomPanel.getHeight() * 0.54f), 6.0f);
    g.setColour(Theme::stroke.withAlpha(0.62f));
    g.drawRoundedRectangle(randomPanel, 7.0f, 0.9f);

    auto dice = bottomStrip.withWidth(58.0f).reduced(15.0f, 18.0f);
    g.setColour(Theme::cyan.withAlpha(0.12f));
    g.fillRoundedRectangle(dice.expanded(7.0f, 5.0f), 6.0f);
    g.setColour(Theme::text.withAlpha(0.82f));
    g.drawRoundedRectangle(dice, 4.0f, 1.0f);
    g.drawLine(dice.getX(), dice.getCentreY(), dice.getCentreX(), dice.getBottom(), 1.0f);
    g.drawLine(dice.getRight(), dice.getCentreY(), dice.getCentreX(), dice.getBottom(), 1.0f);
    g.fillEllipse(dice.getX() + 7.0f, dice.getY() + 7.0f, 3.0f, 3.0f);
    g.fillEllipse(dice.getRight() - 10.0f, dice.getY() + 12.0f, 3.0f, 3.0f);

    auto meterPanel = bottomStrip.withX(500.0f).withWidth(430.0f).reduced(8.0f, 8.0f);
    auto meter = meterPanel.withTrimmedTop(23.0f).withTrimmedBottom(15.0f);
    const int bars = 64;
    const auto masterLevel = juce::jlimit(0.0f, 1.0f, processor.getMasterLevel());
    for (int i = 0; i < bars; ++i)
    {
        const float t = (float) i / (float) (bars - 1);
        const bool active = t <= masterLevel;
        const float barHeight = 5.0f + 20.0f * std::sin((t * juce::MathConstants<float>::pi) * 0.9f);
        const auto barColour = Theme::purple.interpolatedWith(Theme::cyan, t);
        g.setColour(barColour.withAlpha(active ? 0.92f : 0.18f));
        g.fillRoundedRectangle(meter.getX() + t * meter.getWidth(), meter.getBottom() - barHeight,
                               3.4f, barHeight, 1.7f);
    }

    g.setColour(Theme::mutedText.withAlpha(0.76f));
    g.setFont(juce::Font(7.8f, juce::Font::bold));
    const char* ticks[] = { "-60", "-50", "-40", "-30", "-20", "-10", "0" };
    for (int i = 0; i < 7; ++i)
    {
        const float t = (float) i / 6.0f;
        const float x = meter.getX() + t * meter.getWidth();
        g.drawFittedText(ticks[i], juce::Rectangle<int>((int) x - 13, (int) meter.getBottom() + 1, 26, 9),
                         juce::Justification::centred, 1);
    }

    g.setColour(Theme::text.withAlpha(0.82f));
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawFittedText(levelToDbText(masterLevel), bottomStrip.withX(930.0f).withWidth(82.0f).toNearestInt().reduced(0, 30),
                     juce::Justification::centredLeft, 1);

    juce::Path waveA;
    juce::Path waveB;
    juce::Path waveC;
    auto waveArea = bottomStrip.withTrimmedLeft(1015.0f).reduced(16.0f, 12.0f);
    for (int i = 0; i < 220; ++i)
    {
        const float t = (float) i / 219.0f;
        const float x = waveArea.getX() + t * waveArea.getWidth();
        const float yA = waveArea.getCentreY() + std::sin(t * 17.0f + 0.5f) * 15.0f + std::sin(t * 41.0f) * 5.0f;
        const float yB = waveArea.getCentreY() + std::sin(t * 14.0f + 2.1f) * 18.0f;
        const float yC = waveArea.getCentreY() + std::sin(t * 22.0f + 1.4f) * 10.0f + std::sin(t * 9.0f) * 12.0f;
        if (i == 0)
        {
            waveA.startNewSubPath(x, yA);
            waveB.startNewSubPath(x, yB);
            waveC.startNewSubPath(x, yC);
        }
        else
        {
            waveA.lineTo(x, yA);
            waveB.lineTo(x, yB);
            waveC.lineTo(x, yC);
        }
    }
    g.setColour(Theme::purple.withAlpha(0.18f));
    g.strokePath(waveA, juce::PathStrokeType(5.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(Theme::cyan.withAlpha(0.18f));
    g.strokePath(waveB, juce::PathStrokeType(5.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(Theme::blue.withAlpha(0.14f));
    g.strokePath(waveC, juce::PathStrokeType(4.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(Theme::purple.withAlpha(0.78f));
    g.strokePath(waveA, juce::PathStrokeType(1.9f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(Theme::cyan.withAlpha(0.78f));
    g.strokePath(waveB, juce::PathStrokeType(1.9f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(Theme::blue.withAlpha(0.50f));
    g.strokePath(waveC, juce::PathStrokeType(1.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    auto bodyArea = getLocalBounds().reduced(Theme::outerMargin);
    bodyArea.removeFromTop(Theme::topPanelHeight);
    bodyArea.removeFromBottom(Theme::bottomStripHeight);
    auto header = bodyArea.removeFromTop(Theme::headerHeight).toFloat();
    Theme::fillVerticalGradient(g, header, juce::Colour(0xff0a2326), juce::Colour(0xff061316));
    g.setColour(Theme::stroke.withAlpha(0.45f));
    g.drawRoundedRectangle(header, 4.0f, 0.8f);

    auto headerCells = header.toNearestInt().reduced(8, 0);
    auto drawHeader = [&g] (juce::Rectangle<int>& r, int width, const juce::String& text)
    {
        auto cell = r.removeFromLeft(width);
        g.setColour(Theme::mutedText.withAlpha(0.90f));
        g.setFont(juce::Font(9.5f, juce::Font::bold));
        g.drawFittedText(text, cell.reduced(2, 0), juce::Justification::centred, 1);
    };
    drawHeader(headerCells, 54, "LAYER");
    drawHeader(headerCells, 540, "FILE / LOOP / TIME");
    drawHeader(headerCells, 74, "ON");
    drawHeader(headerCells, 90, "VOLUME");
    drawHeader(headerCells, 62, "PAN");
    drawHeader(headerCells, 164, "AUTO PAN");
    drawHeader(headerCells, 60, "SPEED");
    drawHeader(headerCells, 58, "DRIFT");
    drawHeader(headerCells, 58, "WIDTH");
    drawHeader(headerCells, 78, "START OFFSET");
    drawHeader(headerCells, 60, "HP");
    drawHeader(headerCells, 60, "LP");
    drawHeader(headerCells, 62, "XFADE");

    g.setColour(Theme::purple.withAlpha(0.9f));
    g.drawLine((float) Theme::outerMargin + 10.0f, (float) Theme::topPanelHeight + Theme::outerMargin,
               (float) getWidth() - (float) Theme::outerMargin - 10.0f,
               (float) Theme::topPanelHeight + Theme::outerMargin, 1.0f);
}

void SceneLooperAudioProcessorEditor::paintOverChildren(juce::Graphics& g)
{
    auto drawMacroValue = [&g] (juce::Slider& slider, juce::Colour accent)
    {
        auto value = slider.getBounds().withSizeKeepingCentre(86, 18);
        value.setY(slider.getBottom() - 22);
        drawValuePill(g, value, sliderValueText(slider), accent, 11.0f);
    };

    drawMacroValue(masterSlider, Theme::purple);
    drawMacroValue(globalXFadeSlider, Theme::cyan);
    drawMacroValue(masterLowCutSlider, Theme::blue);
    drawMacroValue(masterHighCutSlider, Theme::purple);

    auto randomValue = randomStartSlider.getBounds().translated(48, 15).withSizeKeepingCentre(72, 18);
    drawValuePill(g, randomValue, sliderValueText(randomStartSlider), Theme::purple, 11.0f);
}

void SceneLooperAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(Theme::outerMargin);
    auto fullTop = area.removeFromTop(Theme::topPanelHeight);
    auto top = fullTop.reduced(16, 10);

    auto brand = top.removeFromLeft(350);
    titleLabel.setBounds(brand.removeFromTop(32).withTrimmedLeft(84));
    bylineLabel.setBounds(brand.removeFromTop(20).withTrimmedLeft(84));
    taglineLabel.setBounds(brand.removeFromTop(20).withTrimmedLeft(84));

    auto scene = top.removeFromLeft(390).reduced(8, 0);
    sceneCaptionLabel.setBounds(scene.removeFromTop(18));
    sceneNameLabel.setBounds(scene.removeFromTop(44).reduced(0, 4));

    auto sceneButtons = top.removeFromLeft(290).reduced(4, 0);
    sceneButtons.removeFromTop(28);
    auto sceneButtonRow = sceneButtons.removeFromTop(46);
    loadSceneButton.setBounds(sceneButtonRow.removeFromLeft(140).reduced(5, 0));
    saveSceneButton.setBounds(sceneButtonRow.removeFromLeft(140).reduced(5, 0));

    auto macroArea = fullTop.withTrimmedTop(60).reduced(32, 2);
    auto placeMacro = [&macroArea] (juce::Label& label, juce::Slider& slider, int width)
    {
        auto column = macroArea.removeFromLeft(width).reduced(10, 0);
        label.setBounds(column.removeFromTop(16));
        auto knobArea = column.removeFromTop(68);
        slider.setBounds(knobArea.withSizeKeepingCentre(70, 70));
    };
    placeMacro(masterLabel, masterSlider, 346);
    placeMacro(globalXFadeLabel, globalXFadeSlider, 356);
    placeMacro(masterLowCutLabel, masterLowCutSlider, 356);
    placeMacro(masterHighCutLabel, masterHighCutSlider, 346);

    auto bottom = area.removeFromBottom(Theme::bottomStripHeight).reduced(18, 6);
    auto randomBlock = bottom.removeFromLeft(350);
    randomizationLabel.setBounds(randomBlock.removeFromLeft(142).reduced(4, 17));
    randomizeButton.setBounds(randomBlock.removeFromLeft(102).reduced(8, 16));
    randomStartLabel.setBounds(randomBlock.removeFromTop(16));
    randomStartSlider.setBounds(randomBlock.withSizeKeepingCentre(48, 48).translated(-22, 1));
    masterMeterLabel.setBounds(bottom.withX(520).withWidth(220).removeFromTop(18));

    area.removeFromTop(Theme::headerHeight);
    auto rowArea = area.reduced(0, 4);
    for (auto& row : rows)
    {
        if (row != nullptr)
            row->setBounds(rowArea.removeFromTop(Theme::rowHeight));
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

    juce::ColourGradient bg(juce::Colour(0xff020c0e), bounds.getX(), bounds.getY(),
                            juce::Colour(0xff0a2a2f), bounds.getRight(), bounds.getBottom(), false);
    bg.addColour(0.48, accent.withAlpha(0.18f));
    g.setGradientFill(bg);
    g.fillRoundedRectangle(bounds, 5.0f);

    g.setColour(accent.withAlpha(0.30f));
    g.drawRoundedRectangle(bounds, 5.0f, 1.0f);

    std::array<float, SceneLooperAudioProcessor::waveformPreviewPoints> preview;
    const bool hasPreview = processor.copyWaveformPreview(layerIndex, preview);
    const float centreY = bounds.getCentreY();

    if (! hasPreview)
    {
        g.setColour(accent.withAlpha(0.24f));
        g.drawLine(bounds.getX() + 5.0f, centreY, bounds.getRight() - 5.0f, centreY, 1.8f);
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
        g.setColour(waveColour.withAlpha(0.22f));
        g.fillRoundedRectangle(x - 1.75f, centreY - y - 1.8f, 3.5f, y * 2.0f + 3.6f, 1.8f);
        g.setColour(waveColour.withAlpha(0.96f));
        g.fillRoundedRectangle(x - 0.9f, centreY - y, 1.8f, y * 2.0f, 0.9f);
    }

    const auto cursorFraction = processor.getLayerPlaybackPositionFraction(layerIndex);
    if (cursorFraction >= 0.0)
    {
        const float cursorX = bounds.getX() + bounds.getWidth() * (float) cursorFraction;
        g.setColour(Theme::cyan.withAlpha(0.32f));
        g.drawLine(cursorX, bounds.getY() + 2.0f, cursorX, bounds.getBottom() - 2.0f, 5.0f);
        g.setColour(Theme::text.withAlpha(0.98f));
        g.drawLine(cursorX, bounds.getY() + 2.0f, cursorX, bounds.getBottom() - 2.0f, 1.8f);
        g.setColour(accent.withAlpha(0.9f));
        g.fillEllipse(cursorX - 2.0f, bounds.getY() + 1.0f, 4.0f, 4.0f);
    }

    const auto layerLevel = juce::jlimit(0.0f, 1.0f, processor.getLayerLevel(layerIndex));
    const auto meterBounds = bounds.removeFromBottom(7.0f).reduced(4.0f, 1.0f);
    g.setColour(accent.withAlpha(0.18f));
    g.fillRoundedRectangle(meterBounds, 2.0f);
    g.setColour(accent.interpolatedWith(Theme::cyan, 0.55f).withAlpha(0.88f));
    g.fillRoundedRectangle(meterBounds.withWidth(meterBounds.getWidth() * layerLevel), 2.0f);
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
    numberLabel.setColour(juce::Label::textColourId, accent.brighter(0.25f));
    numberLabel.setFont(juce::Font(28.0f, juce::Font::plain));
    addAndMakeVisible(numberLabel);

    loadButton.setButtonText("Load");
    fileLabel.setText(processor.getFileNameForLayer(layerIndex), juce::dontSendNotification);
    fileLabel.setColour(juce::Label::textColourId, Theme::text.withAlpha(0.84f));
    fileLabel.setJustificationType(juce::Justification::centredLeft);
    fileLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    addAndMakeVisible(fileLabel);
    addAndMakeVisible(waveformPreview);

    addAndMakeVisible(loadButton);
    addAndMakeVisible(onButton);
    addAndMakeVisible(soloButton);
    autoPanButton.setButtonText("AP");
    addAndMakeVisible(autoPanButton);

    auto setupTimeLabel = [] (juce::Label& label)
    {
        label.setJustificationType(juce::Justification::centredRight);
        label.setColour(juce::Label::textColourId, Theme::mutedText);
        label.setFont(9.4f);
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
        label.setFont(9.5f);
    };

    setupLabel(volumeLabel, "Volume");
    setupLabel(panLabel, "Pan");
    setupLabel(speedLabel, "Speed");
    setupLabel(driftLabel, "Drift");
    setupLabel(widthLabel, "Width");
    setupLabel(autoPanAmountLabel, "AP Amt");
    setupLabel(autoPanRateLabel, "AP Hz");
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
    Theme::fillVerticalGradient(g, r.reduced(0.0f, 1.0f), juce::Colour(0xff0c282b).withAlpha(0.92f), juce::Colour(0xff061316));

    g.setColour(accent.withAlpha(0.055f));
    g.fillRoundedRectangle(r.reduced(7.0f, 5.0f), 7.0f);

    g.setColour(accent.withAlpha(0.95f));
    g.fillRoundedRectangle(r.withWidth(5.0f).reduced(0.0f, 2.0f), 7.0f);
    juce::ColourGradient numberGradient(accent.withAlpha(0.22f), r.getX(), r.getY(),
                                        Theme::panelDeep.withAlpha(0.18f), r.getX() + 72.0f, r.getBottom(), false);
    g.setGradientFill(numberGradient);
    g.fillRoundedRectangle(r.withWidth(72.0f).reduced(0.0f, 2.0f), 7.0f);

    auto fileBlock = juce::Rectangle<float>(54.0f, 6.0f, 540.0f, r.getHeight() - 13.0f);
    juce::ColourGradient fileGradient(juce::Colour(0xff07181b), fileBlock.getX(), fileBlock.getY(),
                                      juce::Colour(0xff041012), fileBlock.getRight(), fileBlock.getBottom(), false);
    fileGradient.addColour(0.50, accent.withAlpha(0.065f));
    g.setGradientFill(fileGradient);
    g.fillRoundedRectangle(fileBlock, 6.0f);
    g.setColour(accent.withAlpha(0.22f));
    g.drawRoundedRectangle(fileBlock, 6.0f, 0.8f);

    auto controlBlock = juce::Rectangle<float>(604.0f, 6.0f, r.getWidth() - 612.0f, r.getHeight() - 13.0f);
    g.setColour(juce::Colour(0xff051417).withAlpha(0.38f));
    g.fillRoundedRectangle(controlBlock, 6.0f);

    g.setColour(Theme::stroke.withAlpha(0.38f));
    g.drawRoundedRectangle(r.reduced(0.5f, 1.0f), 7.0f, 0.8f);
}

void SceneLooperAudioProcessorEditor::LayerRow::paintOverChildren(juce::Graphics& g)
{
    const auto accent = Theme::layerColour(layerIndex);

    auto drawSliderValue = [&g, accent] (juce::Slider& slider, int width = 52)
    {
        auto value = slider.getBounds().withSizeKeepingCentre(width, 15);
        value.setY(slider.getBottom() + 1);
        drawValuePill(g, value, sliderValueText(slider), accent, 9.8f);
    };

    drawSliderValue(volumeSlider, 64);
    drawSliderValue(panSlider, 48);
    drawSliderValue(autoPanAmountSlider, 48);
    drawSliderValue(autoPanRateSlider, 54);
    drawSliderValue(speedSlider, 50);
    drawSliderValue(driftSlider, 48);
    drawSliderValue(widthSlider, 48);
    drawSliderValue(offsetSlider, 64);
    drawSliderValue(hpSlider, 52);
    drawSliderValue(lpSlider, 52);
    drawSliderValue(xfadeSlider, 54);
}

void SceneLooperAudioProcessorEditor::LayerRow::resized()
{
    auto area = getLocalBounds().reduced(8, 6);
    numberLabel.setBounds(area.removeFromLeft(46));

    auto fileArea = area.removeFromLeft(540).reduced(2, 0);
    auto fileTop = fileArea.removeFromTop(18);
    loadButton.setBounds(fileTop.removeFromRight(62).reduced(3, 1));
    auto timeArea = fileTop.removeFromRight(142).reduced(2, 0);
    lengthLabel.setBounds(timeArea.removeFromLeft(68));
    remainLabel.setBounds(timeArea);
    fileLabel.setBounds(fileTop);
    waveformPreview.setBounds(fileArea.reduced(0, 2));

    onButton.setBounds(area.removeFromLeft(34).reduced(2, 18));
    soloButton.setBounds(area.removeFromLeft(34).reduced(2, 18));

    auto placeControl = [&area] (juce::Slider& slider, int width)
    {
        auto column = area.removeFromLeft(width).reduced(1, 0);
        auto knobArea = column.removeFromTop(43);
        slider.setBounds(knobArea.withSizeKeepingCentre(42, 42));
    };

    auto volumeColumn = area.removeFromLeft(90).reduced(2, 0);
    volumeSlider.setBounds(volumeColumn.removeFromTop(43).reduced(0, 2));
    placeControl(panSlider, 62);
    autoPanButton.setBounds(area.removeFromLeft(40).reduced(2, 18));
    placeControl(autoPanAmountSlider, 60);
    placeControl(autoPanRateSlider, 64);
    placeControl(speedSlider, 60);
    placeControl(driftSlider, 58);
    placeControl(widthSlider, 58);
    placeControl(offsetSlider, 78);
    placeControl(hpSlider, 60);
    placeControl(lpSlider, 60);
    placeControl(xfadeSlider, 62);
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
