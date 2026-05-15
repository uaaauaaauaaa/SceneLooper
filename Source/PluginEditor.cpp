#include "PluginEditor.h"

#include <cmath>

namespace
{
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
}

SceneLooperAudioProcessorEditor::SceneLooperAudioProcessorEditor(SceneLooperAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setResizable(false, false);
    setResizeLimits(1480, 860, 1480, 860);

    titleLabel.setText("SceneLooper v0.6", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(28.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);
    addAndMakeVisible(saveSceneButton);
    addAndMakeVisible(loadSceneButton);

    auto setupMacro = [] (juce::Slider& s, const juce::String& suffix)
    {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        s.setTextValueSuffix(suffix);
    };

    setupMacro(masterSlider, " dB");
    setupMacro(globalXFadeSlider, " s");
    addAndMakeVisible(masterSlider);
    addAndMakeVisible(globalXFadeSlider);

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
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "SceneLooper", error);
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
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "SceneLooper", error);

                refreshLayerNames();
                refreshLayerTimes();
            });
    };

    for (int i = 0; i < SceneLooperAudioProcessor::numLayers; ++i)
    {
        rows[(size_t) i] = std::make_unique<LayerRow>(processor, i);
        addAndMakeVisible(*rows[(size_t) i]);
    }

    setSize(1480, 860);
    resized();
    startTimerHz(20);
}

SceneLooperAudioProcessorEditor::~SceneLooperAudioProcessorEditor() = default;

void SceneLooperAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0b1118));

    auto bounds = getLocalBounds().toFloat().reduced(12.0f);
    g.setColour(juce::Colour(0xff111b25));
    g.fillRoundedRectangle(bounds, 14.0f);

    g.setColour(juce::Colour(0xff263545));
    g.drawRoundedRectangle(bounds, 14.0f, 1.0f);

    g.setColour(juce::Colour(0xff7d5cff));
    g.drawLine(24.0f, 76.0f, (float) getWidth() - 24.0f, 76.0f, 1.0f);

    g.setColour(juce::Colours::white.withAlpha(0.75f));
    g.setFont(12.0f);
    g.drawText("MASTER", 384, 22, 120, 20, juce::Justification::centred);
    g.drawText("GLOBAL XFADE", 504, 22, 140, 20, juce::Justification::centred);
}

void SceneLooperAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(24);
    auto top = area.removeFromTop(92);
    auto sceneButtons = top.removeFromRight(240);
    saveSceneButton.setBounds(sceneButtons.removeFromLeft(112).reduced(4, 28));
    loadSceneButton.setBounds(sceneButtons.removeFromLeft(112).reduced(4, 28));
    titleLabel.setBounds(top.removeFromLeft(360));
    masterSlider.setBounds(top.removeFromLeft(120).reduced(12));
    globalXFadeSlider.setBounds(top.removeFromLeft(140).reduced(12));

    auto rowArea = area.reduced(0, 6);
    const int rowHeight = 84;
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
    g.setColour(juce::Colour(0xff0b1118));
    g.fillRoundedRectangle(bounds, 4.0f);

    g.setColour(juce::Colour(0xff2b3b4d));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    std::array<float, SceneLooperAudioProcessor::waveformPreviewPoints> preview;
    const bool hasPreview = processor.copyWaveformPreview(layerIndex, preview);
    const float centreY = bounds.getCentreY();

    if (! hasPreview)
    {
        g.setColour(juce::Colours::white.withAlpha(0.16f));
        g.drawLine(bounds.getX() + 4.0f, centreY, bounds.getRight() - 4.0f, centreY, 1.0f);
        return;
    }

    g.setColour(layerIndex % 2 == 0 ? juce::Colour(0xff8b73ff) : juce::Colour(0xff57cfff));
    const float usableHeight = bounds.getHeight() * 0.42f;
    const float pointWidth = bounds.getWidth() / (float) SceneLooperAudioProcessor::waveformPreviewPoints;

    for (int i = 0; i < SceneLooperAudioProcessor::waveformPreviewPoints; ++i)
    {
        const float peak = juce::jlimit(0.0f, 1.0f, preview[(size_t) i]);
        const float x = bounds.getX() + ((float) i + 0.5f) * pointWidth;
        const float y = peak * usableHeight;
        g.drawVerticalLine((int) std::round(x), centreY - y, centreY + y);
    }

    const auto cursorFraction = processor.getLayerPlaybackPositionFraction(layerIndex);
    if (cursorFraction >= 0.0)
    {
        const float cursorX = bounds.getX() + bounds.getWidth() * (float) cursorFraction;
        g.setColour(juce::Colours::white.withAlpha(0.88f));
        g.drawLine(cursorX, bounds.getY() + 2.0f, cursorX, bounds.getBottom() - 2.0f, 1.5f);
    }
}

void SceneLooperAudioProcessorEditor::LayerRow::WaveformPreview::mouseDown(const juce::MouseEvent& event)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    if (bounds.getWidth() <= 0.0f)
        return;

    const auto fraction = (event.position.x - bounds.getX()) / bounds.getWidth();
    processor.seekLayerToFraction(layerIndex, juce::jlimit(0.0f, 1.0f, fraction));
    repaint();
}

SceneLooperAudioProcessorEditor::LayerRow::LayerRow(SceneLooperAudioProcessor& p, int index)
    : processor(p), layerIndex(index), waveformPreview(p, index)
{
    numberLabel.setText(juce::String(index + 1), juce::dontSendNotification);
    numberLabel.setJustificationType(juce::Justification::centred);
    numberLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    numberLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    addAndMakeVisible(numberLabel);

    fileLabel.setText(processor.getFileNameForLayer(layerIndex), juce::dontSendNotification);
    fileLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.82f));
    fileLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(fileLabel);
    addAndMakeVisible(waveformPreview);

    addAndMakeVisible(loadButton);
    addAndMakeVisible(onButton);
    addAndMakeVisible(soloButton);
    addAndMakeVisible(autoPanButton);

    auto setupTimeLabel = [] (juce::Label& label)
    {
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.78f));
        label.setFont(11.0f);
    };

    setupTimeLabel(lengthLabel);
    setupTimeLabel(remainLabel);
    addAndMakeVisible(lengthLabel);
    addAndMakeVisible(remainLabel);

    auto setupLabel = [] (juce::Label& label, const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.72f));
        label.setFont(11.0f);
    };

    setupLabel(volumeLabel, "Volume");
    setupLabel(panLabel, "Pan");
    setupLabel(autoPanAmountLabel, "AP Amt");
    setupLabel(autoPanRateLabel, "AP Hz");
    setupLabel(hpLabel, "HP");
    setupLabel(lpLabel, "LP");
    setupLabel(xfadeLabel, "XFade");
    setupLabel(offsetLabel, "Start Offset");

    addAndMakeVisible(volumeLabel);
    addAndMakeVisible(panLabel);
    addAndMakeVisible(autoPanAmountLabel);
    addAndMakeVisible(autoPanRateLabel);
    addAndMakeVisible(hpLabel);
    addAndMakeVisible(lpLabel);
    addAndMakeVisible(xfadeLabel);
    addAndMakeVisible(offsetLabel);

    setupSlider(volumeSlider, " dB");
    setupSlider(panSlider, "");
    setupSlider(autoPanAmountSlider, "");
    setupSlider(autoPanRateSlider, " Hz");
    setupSlider(hpSlider, " Hz");
    setupSlider(lpSlider, " Hz");
    setupSlider(xfadeSlider, " s");
    setupSlider(offsetSlider, " s");

    addAndMakeVisible(volumeSlider);
    addAndMakeVisible(panSlider);
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
                        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "SceneLooper", error);
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
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 18);
    slider.setTextValueSuffix(suffix);
}

void SceneLooperAudioProcessorEditor::LayerRow::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff111923));
    g.fillRoundedRectangle(r, 8.0f);

    g.setColour(layerIndex % 2 == 0 ? juce::Colour(0xff7d5cff) : juce::Colour(0xff1dd7ff));
    g.fillRoundedRectangle(r.withWidth(4.0f), 8.0f);

    g.setColour(juce::Colour(0xff263545));
    g.drawRoundedRectangle(r, 8.0f, 1.0f);
}

void SceneLooperAudioProcessorEditor::LayerRow::resized()
{
    auto area = getLocalBounds().reduced(8, 6);
    numberLabel.setBounds(area.removeFromLeft(42));
    loadButton.setBounds(area.removeFromLeft(82).reduced(4, 6));
    auto fileArea = area.removeFromLeft(228);
    fileLabel.setBounds(fileArea.removeFromTop(20));
    waveformPreview.setBounds(fileArea.reduced(0, 3));
    lengthLabel.setBounds(area.removeFromLeft(82));
    remainLabel.setBounds(area.removeFromLeft(88));
    onButton.setBounds(area.removeFromLeft(58));
    soloButton.setBounds(area.removeFromLeft(64));
    autoPanButton.setBounds(area.removeFromLeft(80));

    auto placeControl = [&area] (juce::Label& label, juce::Slider& slider, int width)
    {
        auto column = area.removeFromLeft(width);
        label.setBounds(column.removeFromTop(16));
        slider.setBounds(column);
    };

    placeControl(volumeLabel, volumeSlider, 84);
    placeControl(panLabel, panSlider, 74);
    placeControl(autoPanAmountLabel, autoPanAmountSlider, 76);
    placeControl(autoPanRateLabel, autoPanRateSlider, 78);
    placeControl(hpLabel, hpSlider, 82);
    placeControl(lpLabel, lpSlider, 82);
    placeControl(xfadeLabel, xfadeSlider, 82);
    placeControl(offsetLabel, offsetSlider, 94);
}

void SceneLooperAudioProcessorEditor::LayerRow::refreshFileName()
{
    fileLabel.setText(processor.getFileNameForLayer(layerIndex), juce::dontSendNotification);
    waveformPreview.repaint();
}

void SceneLooperAudioProcessorEditor::LayerRow::refreshTimeDisplay()
{
    lengthLabel.setText("Length " + formatTime(processor.getLayerLengthSeconds(layerIndex), false),
        juce::dontSendNotification);
    remainLabel.setText("Remain " + formatTime(processor.getLayerRemainingSeconds(layerIndex), true),
        juce::dontSendNotification);
    waveformPreview.repaint();
}
