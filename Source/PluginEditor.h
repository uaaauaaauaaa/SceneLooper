#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class SceneLooperAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit SceneLooperAudioProcessorEditor(SceneLooperAudioProcessor&);
    ~SceneLooperAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    class LayerRow : public juce::Component
    {
    public:
        LayerRow(SceneLooperAudioProcessor& processor, int layerIndex);
        void resized() override;
        void paint(juce::Graphics& g) override;
        void refreshFileName();

    private:
        SceneLooperAudioProcessor& processor;
        int layerIndex = 0;

        juce::Label numberLabel;
        juce::TextButton loadButton { "Load WAV" };
        juce::Label fileLabel;
        juce::ToggleButton onButton { "On" };
        juce::ToggleButton soloButton { "S" };
        juce::Slider volumeSlider;
        juce::Slider panSlider;
        juce::Slider hpSlider;
        juce::Slider lpSlider;
        juce::Slider xfadeSlider;
        juce::Slider offsetSlider;
        std::unique_ptr<juce::FileChooser> fileChooser;

        std::unique_ptr<ButtonAttachment> onAttachment;
        std::unique_ptr<ButtonAttachment> soloAttachment;
        std::unique_ptr<SliderAttachment> volumeAttachment;
        std::unique_ptr<SliderAttachment> panAttachment;
        std::unique_ptr<SliderAttachment> hpAttachment;
        std::unique_ptr<SliderAttachment> lpAttachment;
        std::unique_ptr<SliderAttachment> xfadeAttachment;
        std::unique_ptr<SliderAttachment> offsetAttachment;

        void setupSlider(juce::Slider& slider, const juce::String& suffix);
    };

    SceneLooperAudioProcessor& processor;

    juce::Label titleLabel;
    juce::Slider masterSlider;
    juce::Slider globalXFadeSlider;
    std::unique_ptr<SliderAttachment> masterAttachment;
    std::unique_ptr<SliderAttachment> globalXFadeAttachment;

    juce::OwnedArray<LayerRow> rows;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SceneLooperAudioProcessorEditor)
};
