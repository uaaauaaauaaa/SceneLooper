#pragma once

#include <array>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class SceneLooperAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        private juce::Timer
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
        void refreshTimeDisplay();

    private:
        class WaveformPreview : public juce::Component
        {
        public:
            WaveformPreview(SceneLooperAudioProcessor& processor, int layerIndex);
            void paint(juce::Graphics& g) override;
            void mouseDown(const juce::MouseEvent& event) override;

        private:
            SceneLooperAudioProcessor& processor;
            int layerIndex = 0;
        };

        SceneLooperAudioProcessor& processor;
        int layerIndex = 0;

        juce::Label numberLabel;
        juce::TextButton loadButton { "Load WAV" };
        juce::Label fileLabel;
        WaveformPreview waveformPreview;
        juce::ToggleButton onButton { "On" };
        juce::ToggleButton soloButton { "Solo" };
        juce::ToggleButton autoPanButton { "AutoPan" };
        juce::Label lengthLabel;
        juce::Label remainLabel;
        juce::Label volumeLabel;
        juce::Label panLabel;
        juce::Label autoPanAmountLabel;
        juce::Label autoPanRateLabel;
        juce::Label hpLabel;
        juce::Label lpLabel;
        juce::Label xfadeLabel;
        juce::Label offsetLabel;
        juce::Slider volumeSlider;
        juce::Slider panSlider;
        juce::Slider autoPanAmountSlider;
        juce::Slider autoPanRateSlider;
        juce::Slider hpSlider;
        juce::Slider lpSlider;
        juce::Slider xfadeSlider;
        juce::Slider offsetSlider;
        std::unique_ptr<juce::FileChooser> fileChooser;

        std::unique_ptr<ButtonAttachment> onAttachment;
        std::unique_ptr<ButtonAttachment> soloAttachment;
        std::unique_ptr<ButtonAttachment> autoPanAttachment;
        std::unique_ptr<SliderAttachment> volumeAttachment;
        std::unique_ptr<SliderAttachment> panAttachment;
        std::unique_ptr<SliderAttachment> autoPanAmountAttachment;
        std::unique_ptr<SliderAttachment> autoPanRateAttachment;
        std::unique_ptr<SliderAttachment> hpAttachment;
        std::unique_ptr<SliderAttachment> lpAttachment;
        std::unique_ptr<SliderAttachment> xfadeAttachment;
        std::unique_ptr<SliderAttachment> offsetAttachment;

        void setupSlider(juce::Slider& slider, const juce::String& suffix);
    };

    SceneLooperAudioProcessor& processor;

    juce::Label titleLabel;
    juce::TextButton saveSceneButton { "Save Scene" };
    juce::TextButton loadSceneButton { "Load Scene" };
    juce::Slider masterSlider;
    juce::Slider globalXFadeSlider;
    std::unique_ptr<juce::FileChooser> sceneFileChooser;
    std::unique_ptr<SliderAttachment> masterAttachment;
    std::unique_ptr<SliderAttachment> globalXFadeAttachment;

    std::array<std::unique_ptr<LayerRow>, SceneLooperAudioProcessor::numLayers> rows;

    void refreshLayerNames();
    void refreshLayerTimes();
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SceneLooperAudioProcessorEditor)
};
