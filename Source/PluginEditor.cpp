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
const juce::Colour text { 0xffeefcff };
const juce::Colour mutedText { 0xff93adb6 };

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
        const auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) width, (float) height).reduced(8.0f, 3.0f);
        const auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
        const auto r = bounds.withSizeKeepingCentre(size, size).reduced(2.0f);
        const auto radius = r.getWidth() * 0.5f;
        const auto centre = r.getCentre();
        const auto accent = slider.findColour(juce::Slider::rotarySliderFillColourId);

        g.setColour(juce::Colour(0xff02090b).withAlpha(0.72f));
        g.fillEllipse(r.translated(0.0f, 2.0f));

        juce::ColourGradient knobGradient(juce::Colour(0xff1a2a30), centre.x - radius * 0.35f, centre.y - radius * 0.45f,
                                          juce::Colour(0xff050b0d), centre.x + radius * 0.45f, centre.y + radius * 0.5f, true);
        g.setGradientFill(knobGradient);
        g.fillEllipse(r);

        g.setColour(Theme::stroke.withAlpha(0.85f));
        g.drawEllipse(r, 1.0f);

        juce::Path track;
        track.addCentredArc(centre.x, centre.y, radius - 4.0f, radius - 4.0f, 0.0f,
                            rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colours::black.withAlpha(0.42f));
        g.strokePath(track, juce::PathStrokeType(3.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        juce::Path value;
        value.addCentredArc(centre.x, centre.y, radius - 4.0f, radius - 4.0f, 0.0f,
                            rotaryStartAngle, angle, true);
        g.setColour(accent.withAlpha(0.32f));
        g.strokePath(value, juce::PathStrokeType(7.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(accent);
        g.strokePath(value, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        const auto indicator = centre + juce::Point<float>(std::cos(angle - juce::MathConstants<float>::halfPi),
                                                           std::sin(angle - juce::MathConstants<float>::halfPi)) * (radius - 9.0f);
        g.setColour(Theme::text.withAlpha(0.86f));
        g.fillEllipse(indicator.x - 2.2f, indicator.y - 2.2f, 4.4f, 4.4f);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          float, float, const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        if (style != juce::Slider::LinearHorizontal)
        {
            juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, 0.0f, 0.0f, style, slider);
            return;
        }

        const auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) width, (float) height).reduced(8.0f, 16.0f);
        const auto cy = bounds.getCentreY();
        const auto accent = slider.findColour(juce::Slider::trackColourId);
        g.setColour(juce::Colour(0xff041012));
        g.fillRoundedRectangle(bounds.withHeight(4.0f).withCentre(juce::Point<float>(bounds.getCentreX(), cy)), 2.0f);
        g.setColour(accent.withAlpha(0.85f));
        g.fillRoundedRectangle(juce::Rectangle<float>(bounds.getX(), cy - 2.0f, juce::jmax(0.0f, sliderPos - bounds.getX()), 4.0f), 2.0f);
        g.setColour(Theme::text);
        g.fillRoundedRectangle(sliderPos - 3.0f, cy - 9.0f, 6.0f, 18.0f, 2.0f);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&, bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
        auto base = shouldDrawButtonAsDown ? Theme::purple.withAlpha(0.45f) : Theme::panel;
        if (shouldDrawButtonAsHighlighted)
            base = base.brighter(0.12f);

        Theme::fillVerticalGradient(g, bounds, base.brighter(0.10f), Theme::panelDeep);
        g.setColour((button.getToggleState() ? Theme::purple : Theme::stroke).withAlpha(0.9f));
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
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
    setResizeLimits(1640, 900, 1640, 900);

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
    sceneNameLabel.setText("Coastal Distant Storm", juce::dontSendNotification);
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
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 82, 19);
        s.setTextValueSuffix(suffix);
        s.setColour(juce::Slider::rotarySliderFillColourId, Theme::purple);
    };

    setupMacro(masterSlider, " dB");
    setupMacro(globalXFadeSlider, " s");
    setupMacro(masterLowCutSlider, " Hz");
    setupMacro(masterHighCutSlider, " kHz");
    setupMacro(randomStartSlider, "%");
    masterLowCutSlider.setRange(20.0, 1000.0, 1.0);
    masterLowCutSlider.setValue(80.0, juce::dontSendNotification);
    masterHighCutSlider.setRange(1.0, 20.0, 0.1);
    masterHighCutSlider.setValue(16.0, juce::dontSendNotification);
    randomStartSlider.setRange(0.0, 100.0, 1.0);
    randomStartSlider.setValue(30.0, juce::dontSendNotification);
    globalXFadeSlider.setColour(juce::Slider::rotarySliderFillColourId, Theme::cyan);
    masterLowCutSlider.setColour(juce::Slider::rotarySliderFillColourId, Theme::blue);
    masterHighCutSlider.setColour(juce::Slider::rotarySliderFillColourId, Theme::purple);
    randomStartSlider.setColour(juce::Slider::rotarySliderFillColourId, Theme::purple);
    addAndMakeVisible(masterSlider);
    addAndMakeVisible(globalXFadeSlider);
    addAndMakeVisible(masterLowCutSlider);
    addAndMakeVisible(masterHighCutSlider);
    addAndMakeVisible(randomStartSlider);

    masterAttachment = std::make_unique<SliderAttachment>(processor.apvts, "masterVolume", masterSlider);
    globalXFadeAttachment = std::make_unique<SliderAttachment>(processor.apvts, "globalXFade", globalXFadeSlider);

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
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Atmocycle", error);
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
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Atmocycle", error);

                refreshLayerNames();
                refreshLayerTimes();
            });
    };

    for (int i = 0; i < SceneLooperAudioProcessor::numLayers; ++i)
    {
        rows[(size_t) i] = std::make_unique<LayerRow>(processor, i);
        addAndMakeVisible(*rows[(size_t) i]);
    }

    setSize(1640, 900);
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

    auto bounds = getLocalBounds().toFloat().reduced(10.0f);
    juce::ColourGradient backgroundGlow(juce::Colour(0xff0b2b2e), bounds.getX(), bounds.getY(),
                                        juce::Colour(0xff050d11), bounds.getRight(), bounds.getBottom(), false);
    backgroundGlow.addColour(0.55, juce::Colour(0xff071b25));
    g.setGradientFill(backgroundGlow);
    g.fillRoundedRectangle(bounds, 14.0f);

    g.setColour(Theme::stroke.withAlpha(0.85f));
    g.drawRoundedRectangle(bounds, 14.0f, 1.2f);

    const auto topPanel = getLocalBounds().reduced(18).removeFromTop(160).toFloat();
    Theme::fillVerticalGradient(g, topPanel, juce::Colour(0xff0d3034).withAlpha(0.92f), Theme::panelDeep);
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

    const auto bottomStrip = getLocalBounds().reduced(18).removeFromBottom(82).toFloat();
    Theme::fillVerticalGradient(g, bottomStrip, juce::Colour(0xff0d3438), juce::Colour(0xff071a1e));
    g.setColour(Theme::stroke.withAlpha(0.88f));
    g.drawRoundedRectangle(bottomStrip, 9.0f, 1.0f);

    auto dice = bottomStrip.withWidth(48.0f).reduced(12.0f, 18.0f);
    g.setColour(Theme::text.withAlpha(0.82f));
    g.drawRoundedRectangle(dice, 4.0f, 1.0f);
    g.drawLine(dice.getX(), dice.getCentreY(), dice.getCentreX(), dice.getBottom(), 1.0f);
    g.drawLine(dice.getRight(), dice.getCentreY(), dice.getCentreX(), dice.getBottom(), 1.0f);
    g.fillEllipse(dice.getX() + 7.0f, dice.getY() + 7.0f, 3.0f, 3.0f);
    g.fillEllipse(dice.getRight() - 10.0f, dice.getY() + 12.0f, 3.0f, 3.0f);

    auto meter = bottomStrip.withX(620.0f).withWidth(300.0f).reduced(8.0f, 34.0f);
    const int bars = 44;
    for (int i = 0; i < bars; ++i)
    {
        const float t = (float) i / (float) (bars - 1);
        const float barHeight = 4.0f + 16.0f * std::sin((t * juce::MathConstants<float>::pi) * 0.9f);
        const auto barColour = Theme::purple.interpolatedWith(Theme::cyan, t);
        g.setColour(barColour.withAlpha(0.72f));
        g.fillRoundedRectangle(meter.getX() + t * meter.getWidth(), meter.getBottom() - barHeight,
                               3.0f, barHeight, 1.5f);
    }

    juce::Path waveA;
    juce::Path waveB;
    auto waveArea = bottomStrip.withTrimmedLeft(940.0f).reduced(18.0f, 16.0f);
    for (int i = 0; i < 180; ++i)
    {
        const float t = (float) i / 179.0f;
        const float x = waveArea.getX() + t * waveArea.getWidth();
        const float yA = waveArea.getCentreY() + std::sin(t * 17.0f + 0.5f) * 13.0f + std::sin(t * 41.0f) * 4.0f;
        const float yB = waveArea.getCentreY() + std::sin(t * 14.0f + 2.1f) * 16.0f;
        if (i == 0)
        {
            waveA.startNewSubPath(x, yA);
            waveB.startNewSubPath(x, yB);
        }
        else
        {
            waveA.lineTo(x, yA);
            waveB.lineTo(x, yB);
        }
    }
    g.setColour(Theme::purple.withAlpha(0.65f));
    g.strokePath(waveA, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(Theme::cyan.withAlpha(0.65f));
    g.strokePath(waveB, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setColour(Theme::purple.withAlpha(0.9f));
    g.drawLine(22.0f, 166.0f, (float) getWidth() - 22.0f, 166.0f, 1.0f);
}

void SceneLooperAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(18);
    auto fullTop = area.removeFromTop(160);
    auto top = fullTop.reduced(14, 10);

    auto brand = top.removeFromLeft(420);
    titleLabel.setBounds(brand.removeFromTop(32).withTrimmedLeft(84));
    bylineLabel.setBounds(brand.removeFromTop(20).withTrimmedLeft(84));
    taglineLabel.setBounds(brand.removeFromTop(20).withTrimmedLeft(84));

    auto scene = top.removeFromLeft(420).reduced(8, 0);
    sceneCaptionLabel.setBounds(scene.removeFromTop(18));
    sceneNameLabel.setBounds(scene.removeFromTop(44).reduced(0, 4));

    auto sceneButtons = top.removeFromLeft(260).reduced(4, 24);
    loadSceneButton.setBounds(sceneButtons.removeFromLeft(124).reduced(4, 0));
    saveSceneButton.setBounds(sceneButtons.removeFromLeft(124).reduced(4, 0));

    auto macroArea = fullTop.withTrimmedTop(82).reduced(24, 4);
    auto placeMacro = [&macroArea] (juce::Label& label, juce::Slider& slider, int width)
    {
        auto column = macroArea.removeFromLeft(width).reduced(10, 0);
        label.setBounds(column.removeFromTop(18));
        slider.setBounds(column);
    };
    placeMacro(masterLabel, masterSlider, 290);
    placeMacro(globalXFadeLabel, globalXFadeSlider, 320);
    placeMacro(masterLowCutLabel, masterLowCutSlider, 300);
    placeMacro(masterHighCutLabel, masterHighCutSlider, 300);

    auto bottom = area.removeFromBottom(82).reduced(16, 8);
    randomizationLabel.setBounds(bottom.removeFromLeft(190).reduced(36, 12));
    randomStartLabel.setBounds(bottom.removeFromLeft(140).removeFromTop(20));
    randomStartSlider.setBounds(bottom.withX(250).withWidth(92).reduced(4, 0));
    masterMeterLabel.setBounds(bottom.withX(600).withWidth(220).removeFromTop(18));

    auto rowArea = area.reduced(0, 8);
    const int rowHeight = 72;
    for (auto& row : rows)
    {
        if (row != nullptr)
            row->setBounds(rowArea.removeFromTop(rowHeight).reduced(0, 3));
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
}

SceneLooperAudioProcessorEditor::LayerRow::WaveformPreview::WaveformPreview(SceneLooperAudioProcessor& p, int index)
    : processor(p), layerIndex(index)
{
}

void SceneLooperAudioProcessorEditor::LayerRow::WaveformPreview::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    const auto accent = Theme::layerColour(layerIndex);

    g.setColour(Theme::panelDeep);
    g.fillRoundedRectangle(bounds, 4.0f);

    g.setColour(accent.withAlpha(0.28f));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    std::array<float, SceneLooperAudioProcessor::waveformPreviewPoints> preview;
    const bool hasPreview = processor.copyWaveformPreview(layerIndex, preview);
    const float centreY = bounds.getCentreY();

    if (! hasPreview)
    {
        g.setColour(Theme::text.withAlpha(0.16f));
        g.drawLine(bounds.getX() + 4.0f, centreY, bounds.getRight() - 4.0f, centreY, 1.0f);
        return;
    }

    const float usableHeight = bounds.getHeight() * 0.42f;
    const float pointWidth = bounds.getWidth() / (float) SceneLooperAudioProcessor::waveformPreviewPoints;

    for (int i = 0; i < SceneLooperAudioProcessor::waveformPreviewPoints; ++i)
    {
        const float peak = juce::jlimit(0.0f, 1.0f, preview[(size_t) i]);
        const float x = bounds.getX() + ((float) i + 0.5f) * pointWidth;
        const float y = peak * usableHeight;
        g.setColour(accent.interpolatedWith(Theme::cyan, (float) i / (float) SceneLooperAudioProcessor::waveformPreviewPoints)
                        .withAlpha(0.88f));
        g.drawVerticalLine((int) std::round(x), centreY - y, centreY + y);
    }

    const auto cursorFraction = processor.getLayerPlaybackPositionFraction(layerIndex);
    if (cursorFraction >= 0.0)
    {
        const float cursorX = bounds.getX() + bounds.getWidth() * (float) cursorFraction;
        g.setColour(Theme::text.withAlpha(0.92f));
        g.drawLine(cursorX, bounds.getY() + 2.0f, cursorX, bounds.getBottom() - 2.0f, 1.5f);
        g.setColour(accent.withAlpha(0.9f));
        g.fillEllipse(cursorX - 2.0f, bounds.getY() + 1.0f, 4.0f, 4.0f);
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
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, Theme::mutedText);
        label.setFont(10.0f);
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
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 54, 16);
    slider.setTextValueSuffix(suffix);
}

void SceneLooperAudioProcessorEditor::LayerRow::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    const auto accent = Theme::layerColour(layerIndex);
    Theme::fillVerticalGradient(g, r, juce::Colour(0xff0b2528).withAlpha(0.92f), juce::Colour(0xff061316));

    g.setColour(accent.withAlpha(0.95f));
    g.fillRoundedRectangle(r.withWidth(5.0f), 7.0f);
    g.setColour(accent.withAlpha(0.15f));
    g.fillRoundedRectangle(r.withWidth(64.0f), 7.0f);

    g.setColour(Theme::stroke.withAlpha(0.72f));
    g.drawRoundedRectangle(r, 7.0f, 1.0f);
}

void SceneLooperAudioProcessorEditor::LayerRow::resized()
{
    auto area = getLocalBounds().reduced(8, 5);
    numberLabel.setBounds(area.removeFromLeft(44));

    auto fileArea = area.removeFromLeft(360).reduced(2, 0);
    auto fileTop = fileArea.removeFromTop(19);
    loadButton.setBounds(fileTop.removeFromRight(58).reduced(3, 1));
    fileLabel.setBounds(fileTop);
    waveformPreview.setBounds(fileArea.reduced(0, 3));

    onButton.setBounds(area.removeFromLeft(34).reduced(2, 16));
    soloButton.setBounds(area.removeFromLeft(34).reduced(2, 16));

    auto placeControl = [&area] (juce::Label& label, juce::Slider& slider, int width)
    {
        auto column = area.removeFromLeft(width).reduced(1, 0);
        label.setBounds(column.removeFromTop(13));
        slider.setBounds(column);
    };

    placeControl(volumeLabel, volumeSlider, 76);
    placeControl(panLabel, panSlider, 58);
    autoPanButton.setBounds(area.removeFromLeft(38).reduced(2, 16));
    placeControl(autoPanAmountLabel, autoPanAmountSlider, 58);
    placeControl(autoPanRateLabel, autoPanRateSlider, 62);
    placeControl(speedLabel, speedSlider, 58);
    placeControl(driftLabel, driftSlider, 56);
    placeControl(widthLabel, widthSlider, 56);
    placeControl(offsetLabel, offsetSlider, 66);
    placeControl(hpLabel, hpSlider, 56);
    placeControl(lpLabel, lpSlider, 56);
    placeControl(xfadeLabel, xfadeSlider, 58);

    auto timeArea = area.removeFromLeft(130).reduced(2, 5);
    lengthLabel.setBounds(timeArea.removeFromTop(25));
    remainLabel.setBounds(timeArea.removeFromTop(25));
}

void SceneLooperAudioProcessorEditor::LayerRow::refreshFileName()
{
    fileLabel.setText(processor.getFileNameForLayer(layerIndex), juce::dontSendNotification);
    waveformPreview.repaint();
}

void SceneLooperAudioProcessorEditor::LayerRow::refreshTimeDisplay()
{
    lengthLabel.setText("Length / " + formatTime(processor.getLayerLengthSeconds(layerIndex), false),
        juce::dontSendNotification);
    remainLabel.setText("Remain / " + formatTime(processor.getLayerRemainingSeconds(layerIndex), true),
        juce::dontSendNotification);
    waveformPreview.repaint();
}
