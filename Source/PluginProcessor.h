#pragma once

#include <array>
#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_dsp/juce_dsp.h>

class SceneLooperAudioProcessor : public juce::AudioProcessor
{
public:
    static constexpr int numLayers = 8;
    static constexpr int waveformPreviewPoints = 128;

    SceneLooperAudioProcessor();
    ~SceneLooperAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    bool loadFileForLayer(int layerIndex, const juce::File& file, juce::String& errorMessage);
    juce::String getFileNameForLayer(int layerIndex) const;
    bool isLayerLoaded(int layerIndex) const;
    double getLayerLengthSeconds(int layerIndex) const;
    double getLayerRemainingSeconds(int layerIndex) const;
    double getLayerPlaybackPositionFraction(int layerIndex) const;
    void seekLayerToFraction(int layerIndex, double fraction);
    bool copyWaveformPreview(int layerIndex, std::array<float, waveformPreviewPoints>& destination) const;
    bool saveSceneToFile(const juce::File& file, juce::String& errorMessage) const;
    bool loadSceneFromFile(const juce::File& file, juce::String& errorMessage);

    static juce::String paramId(int layerIndex, const juce::String& name);

private:
    struct OnePoleFilter
    {
        void reset() { z1 = 0.0f; }

        float processLowPass(float x, float cutoff, double sampleRate)
        {
            const float a = std::exp(-2.0f * juce::MathConstants<float>::pi * cutoff / (float) sampleRate);
            z1 = (1.0f - a) * x + a * z1;
            return z1;
        }

        float processHighPass(float x, float cutoff, double sampleRate)
        {
            const float lp = processLowPass(x, cutoff, sampleRate);
            return x - lp;
        }

        float z1 = 0.0f;
    };

    struct Layer
    {
        juce::File file;
        juce::String displayName = "No file";
        juce::AudioBuffer<float> audio;
        bool loaded = false;
        double position = 0.0;
        double autoPanPhase = 0.0;
        std::atomic<double> lengthSeconds { 0.0 };
        std::atomic<double> displayPositionSamples { 0.0 };
        std::atomic<double> pendingSeekFraction { -1.0 };
        std::array<float, waveformPreviewPoints> waveformPreview {};
        std::atomic<bool> waveformPreviewReady { false };
        OnePoleFilter hp[2];
        OnePoleFilter lp[2];
    };

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    Layer layers[numLayers];
    juce::AudioFormatManager formatManager;
    double currentSampleRate = 48000.0;

    bool anySoloActive() const;
    float getParameterValue(const juce::String& id) const;
    void setParameterValue(const juce::String& id, float value);
    void markLayerMissingFile(int layerIndex, const juce::File& file);
    void clearLayerFile(int layerIndex);
    void buildWaveformPreview(int layerIndex);
    void resetLayerPlayback();
    void renderLayer(Layer& layer, int layerIndex, juce::AudioBuffer<float>& output, int numSamples, bool soloMode);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SceneLooperAudioProcessor)
};
