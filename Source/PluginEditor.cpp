#include "PluginEditor.h"

SceneLooperAudioProcessorEditor::SceneLooperAudioProcessorEditor(SceneLooperAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setResizable(false, false);
    setResizeLimits(1180, 860, 1180, 860);

    titleLabel.setText("SceneLooper v0.1 UI fix 2", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(28.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

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

    for (int i = 0; i < SceneLooperAudioProcessor::numLayers; ++i)
    {
        rows[(size_t) i] = std::make_unique<LayerRow>(processor, i);
        addAndMakeVisible(*rows[(size_t) i]);
    }

    setSize(1180, 860);
    resized();
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

SceneLooperAudioProcessorEditor::LayerRow::LayerRow(SceneLooperAudioProcessor& p, int index)
    : processor(p), layerIndex(index)
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

    addAndMakeVisible(loadButton);
    addAndMakeVisible(onButton);
    addAndMakeVisible(soloButton);

    auto setupLabel = [] (juce::Label& label, const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.72f));
        label.setFont(11.0f);
    };

    setupLabel(volumeLabel, "Volume");
    setupLabel(panLabel, "Pan");
    setupLabel(hpLabel, "HP");
    setupLabel(lpLabel, "LP");
    setupLabel(xfadeLabel, "XFade");
    setupLabel(offsetLabel, "Start Offset");

    addAndMakeVisible(volumeLabel);
    addAndMakeVisible(panLabel);
    addAndMakeVisible(hpLabel);
    addAndMakeVisible(lpLabel);
    addAndMakeVisible(xfadeLabel);
    addAndMakeVisible(offsetLabel);

    setupSlider(volumeSlider, " dB");
    setupSlider(panSlider, "");
    setupSlider(hpSlider, " Hz");
    setupSlider(lpSlider, " Hz");
    setupSlider(xfadeSlider, " s");
    setupSlider(offsetSlider, " s");

    addAndMakeVisible(volumeSlider);
    addAndMakeVisible(panSlider);
    addAndMakeVisible(hpSlider);
    addAndMakeVisible(lpSlider);
    addAndMakeVisible(xfadeSlider);
    addAndMakeVisible(offsetSlider);

    onAttachment = std::make_unique<ButtonAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "on"), onButton);
    soloAttachment = std::make_unique<ButtonAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "solo"), soloButton);
    volumeAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "volume"), volumeSlider);
    panAttachment = std::make_unique<SliderAttachment>(processor.apvts, SceneLooperAudioProcessor::paramId(layerIndex, "pan"), panSlider);
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
                }
            });
    };
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
    fileLabel.setBounds(area.removeFromLeft(170));
    onButton.setBounds(area.removeFromLeft(58));
    soloButton.setBounds(area.removeFromLeft(64));

    auto placeControl = [&area] (juce::Label& label, juce::Slider& slider, int width)
    {
        auto column = area.removeFromLeft(width);
        label.setBounds(column.removeFromTop(16));
        slider.setBounds(column);
    };

    placeControl(volumeLabel, volumeSlider, 92);
    placeControl(panLabel, panSlider, 82);
    placeControl(hpLabel, hpSlider, 92);
    placeControl(lpLabel, lpSlider, 92);
    placeControl(xfadeLabel, xfadeSlider, 92);
    placeControl(offsetLabel, offsetSlider, 112);
}

void SceneLooperAudioProcessorEditor::LayerRow::refreshFileName()
{
    fileLabel.setText(processor.getFileNameForLayer(layerIndex), juce::dontSendNotification);
}
