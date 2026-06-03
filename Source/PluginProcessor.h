#pragma once

#include <array>
#include <atomic>
#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_dsp/juce_dsp.h>

class SceneLooperAudioProcessor : public juce::AudioProcessor
{
public:
    static constexpr int numLayers = 8;
    static constexpr int waveformPreviewPoints = 128;
    static constexpr int maxSkipRanges = 8;
    struct SkipRange
    {
        double start = 0.0;
        double end = 0.0;
    };

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
    std::vector<SkipRange> getLayerSkipRanges(int layerIndex) const;
    void addLayerSkipRange(int layerIndex, double startFraction, double endFraction);
    void clearLayerSkipRanges(int layerIndex);
    bool copyWaveformPreview(int layerIndex, std::array<float, waveformPreviewPoints>& destination) const;
    float getLayerLevel(int layerIndex) const;
    float getLayerWaveformDisplayGain(int layerIndex) const;
    float getMasterLevel() const;
    float getMasterLeftLevel() const;
    float getMasterRightLevel() const;
    bool isLayerAutopilotOn(int layerIndex) const;
    float getLayerAutopilotPhase(int layerIndex) const;
    void randomizeLayerStarts();
    bool saveSceneToFile(const juce::File& file, juce::String& errorMessage) const;
    bool loadSceneFromFile(const juce::File& file, juce::String& errorMessage);
    juce::String getCurrentSceneName() const;
    void setCurrentSceneName(const juce::String& sceneName);

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
        double autopilotPanPhase = 0.0;
        double autopilotPanRateHz = 1.0 / 45.0;
        double autopilotNextJumpSamples = 0.0;
        double autopilotBlendSourcePosition = 0.0;
        double autopilotBlendTargetPosition = 0.0;
        int autopilotBlendSamplesRemaining = 0;
        int autopilotBlendSamplesTotal = 0;
        double driftPhase = 0.0;
        std::atomic<double> lengthSeconds { 0.0 };
        std::atomic<double> displayPositionSamples { 0.0 };
        std::atomic<double> pendingSeekFraction { -1.0 };
        std::atomic<float> outputLevel { 0.0f };
        std::array<float, waveformPreviewPoints> waveformPreview {};
        std::atomic<bool> waveformPreviewReady { false };
        std::array<std::atomic<double>, maxSkipRanges> skipStartFractions {};
        std::array<std::atomic<double>, maxSkipRanges> skipEndFractions {};
        std::atomic<int> skipRangeCount { 0 };
        OnePoleFilter hp[2];
        OnePoleFilter lp[2];
    };

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    Layer layers[numLayers];
    juce::AudioFormatManager formatManager;
    juce::Random random;
    double currentSampleRate = 48000.0;
    std::atomic<float> masterOutputLevel { 0.0f };
    std::atomic<float> masterOutputLevelLeft { 0.0f };
    std::atomic<float> masterOutputLevelRight { 0.0f };
    OnePoleFilter masterHP[2];
    OnePoleFilter masterLP[2];
    juce::String currentSceneName { "Project State" };

    bool anySoloActive() const;
    float getParameterValue(const juce::String& id) const;
    void setParameterValue(const juce::String& id, float value);
    void markLayerMissingFile(int layerIndex, const juce::File& file);
    void clearLayerFile(int layerIndex);
    juce::String encodeLayerSkipRanges(int layerIndex) const;
    void restoreLayerSkipRangesFromString(int layerIndex, const juce::String& encoded);
    void buildWaveformPreview(int layerIndex);
    void resetLayerPlayback();
    void renderLayer(Layer& layer, int layerIndex, juce::AudioBuffer<float>& output, int numSamples, bool soloMode);
    void applyMasterProcessing(juce::AudioBuffer<float>& buffer);
    int chooseAutopilotTargetSample(const Layer& layer, int length);
    void scheduleAutopilotJump(Layer& layer);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SceneLooperAudioProcessor)
};
