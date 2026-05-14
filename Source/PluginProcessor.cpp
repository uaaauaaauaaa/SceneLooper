#include "PluginProcessor.h"
#include "PluginEditor.h"

SceneLooperAudioProcessor::SceneLooperAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    formatManager.registerBasicFormats();
}

SceneLooperAudioProcessor::~SceneLooperAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout SceneLooperAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("masterVolume", "Master Volume",
        juce::NormalisableRange<float>(-60.0f, 6.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("globalXFade", "Global XFade",
        juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 3.0f));

    for (int i = 0; i < numLayers; ++i)
    {
        const auto prefix = juce::String("layer") + juce::String(i + 1) + "_";
        const auto nice = juce::String("Layer ") + juce::String(i + 1) + " ";

        params.push_back(std::make_unique<juce::AudioParameterBool>(prefix + "on", nice + "On", true));
        params.push_back(std::make_unique<juce::AudioParameterBool>(prefix + "solo", nice + "Solo", false));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "volume", nice + "Volume",
            juce::NormalisableRange<float>(-60.0f, 6.0f, 0.1f), 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "pan", nice + "Pan",
            juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "hp", nice + "HP",
            juce::NormalisableRange<float>(20.0f, 1000.0f, 1.0f, 0.35f), 20.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "lp", nice + "LP",
            juce::NormalisableRange<float>(1000.0f, 20000.0f, 1.0f, 0.35f), 20000.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "xfade", nice + "XFade",
            juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 3.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "offset", nice + "Start Offset",
            juce::NormalisableRange<float>(0.0f, 600.0f, 0.01f), 0.0f));
    }

    return { params.begin(), params.end() };
}

juce::String SceneLooperAudioProcessor::paramId(int layerIndex, const juce::String& name)
{
    return juce::String("layer") + juce::String(layerIndex + 1) + "_" + name;
}

const juce::String SceneLooperAudioProcessor::getName() const { return JucePlugin_Name; }
bool SceneLooperAudioProcessor::acceptsMidi() const { return false; }
bool SceneLooperAudioProcessor::producesMidi() const { return false; }
bool SceneLooperAudioProcessor::isMidiEffect() const { return false; }
double SceneLooperAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int SceneLooperAudioProcessor::getNumPrograms() { return 1; }
int SceneLooperAudioProcessor::getCurrentProgram() { return 0; }
void SceneLooperAudioProcessor::setCurrentProgram(int) {}
const juce::String SceneLooperAudioProcessor::getProgramName(int) { return {}; }
void SceneLooperAudioProcessor::changeProgramName(int, const juce::String&) {}
bool SceneLooperAudioProcessor::hasEditor() const { return true; }

void SceneLooperAudioProcessor::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    resetLayerPlayback();
}

void SceneLooperAudioProcessor::releaseResources() {}

bool SceneLooperAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void SceneLooperAudioProcessor::resetLayerPlayback()
{
    for (int i = 0; i < numLayers; ++i)
    {
        const auto offsetSeconds = apvts.getRawParameterValue(paramId(i, "offset"))->load();
        layers[i].position = std::max(0.0, (double) offsetSeconds * currentSampleRate);
        for (auto& f : layers[i].hp) f.reset();
        for (auto& f : layers[i].lp) f.reset();
    }
}

bool SceneLooperAudioProcessor::anySoloActive() const
{
    for (int i = 0; i < numLayers; ++i)
        if (apvts.getRawParameterValue(paramId(i, "solo"))->load() > 0.5f)
            return true;
    return false;
}

bool SceneLooperAudioProcessor::loadFileForLayer(int layerIndex, const juce::File& file, juce::String& errorMessage)
{
    if (! juce::isPositiveAndBelow(layerIndex, numLayers))
        return false;

    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (reader == nullptr)
    {
        errorMessage = "Unsupported file";
        return false;
    }

    if (std::abs(reader->sampleRate - 48000.0) > 0.1 || reader->bitsPerSample != 24)
    {
        errorMessage = "Use WAV 48 kHz / 24-bit";
        return false;
    }

    if (reader->numChannels < 1 || reader->numChannels > 2)
    {
        errorMessage = "Use mono or stereo WAV";
        return false;
    }

    juce::AudioBuffer<float> newBuffer((int) reader->numChannels, (int) reader->lengthInSamples);
    reader->read(&newBuffer, 0, (int) reader->lengthInSamples, 0, true, true);

    auto& layer = layers[layerIndex];
    layer.audio = std::move(newBuffer);
    layer.file = file;
    layer.displayName = file.getFileName();
    layer.loaded = true;
    layer.position = apvts.getRawParameterValue(paramId(layerIndex, "offset"))->load() * currentSampleRate;

    errorMessage.clear();
    return true;
}

juce::String SceneLooperAudioProcessor::getFileNameForLayer(int layerIndex) const
{
    if (! juce::isPositiveAndBelow(layerIndex, numLayers))
        return {};
    return layers[layerIndex].displayName;
}

bool SceneLooperAudioProcessor::isLayerLoaded(int layerIndex) const
{
    return juce::isPositiveAndBelow(layerIndex, numLayers) && layers[layerIndex].loaded;
}

void SceneLooperAudioProcessor::renderLayer(Layer& layer, int layerIndex, juce::AudioBuffer<float>& output, int numSamples, bool soloMode)
{
    if (! layer.loaded || layer.audio.getNumSamples() <= 0)
        return;

    const bool on = apvts.getRawParameterValue(paramId(layerIndex, "on"))->load() > 0.5f;
    const bool solo = apvts.getRawParameterValue(paramId(layerIndex, "solo"))->load() > 0.5f;
    if (! on || (soloMode && ! solo))
        return;

    const float layerVolumeDb = apvts.getRawParameterValue(paramId(layerIndex, "volume"))->load();
    const float masterVolumeDb = apvts.getRawParameterValue("masterVolume")->load();
    const float gain = juce::Decibels::decibelsToGain(layerVolumeDb + masterVolumeDb, -90.0f);
    const float pan = apvts.getRawParameterValue(paramId(layerIndex, "pan"))->load();
    const float hp = apvts.getRawParameterValue(paramId(layerIndex, "hp"))->load();
    const float lp = apvts.getRawParameterValue(paramId(layerIndex, "lp"))->load();
    const float xfadeSeconds = apvts.getRawParameterValue(paramId(layerIndex, "xfade"))->load();

    const int srcChannels = layer.audio.getNumChannels();
    const int length = layer.audio.getNumSamples();
    int xfadeSamples = (int) std::round(xfadeSeconds * currentSampleRate);
    xfadeSamples = juce::jlimit(0, std::max(0, length / 2 - 1), xfadeSamples);

    auto* outL = output.getWritePointer(0);
    auto* outR = output.getWritePointer(1);
    const auto* srcL = layer.audio.getReadPointer(0);
    const auto* srcR = srcChannels > 1 ? layer.audio.getReadPointer(1) : nullptr;

    for (int n = 0; n < numSamples; ++n)
    {
        int p = (int) layer.position;
        while (p >= length) p -= length;
        while (p < 0) p += length;

        float left = srcL[p];
        float right = srcR != nullptr ? srcR[p] : left;

        if (xfadeSamples > 0 && p >= length - xfadeSamples)
        {
            const int crossIndex = p - (length - xfadeSamples);
            const float t = (float) crossIndex / (float) xfadeSamples;
            const float fadeOut = std::cos(t * juce::MathConstants<float>::halfPi);
            const float fadeIn = std::sin(t * juce::MathConstants<float>::halfPi);
            const int q = juce::jlimit(0, length - 1, crossIndex);

            left = left * fadeOut + srcL[q] * fadeIn;
            const float startRight = srcR != nullptr ? srcR[q] : srcL[q];
            right = right * fadeOut + startRight * fadeIn;
        }

        if (hp > 20.5f)
        {
            left = layer.hp[0].processHighPass(left, hp, currentSampleRate);
            right = layer.hp[1].processHighPass(right, hp, currentSampleRate);
        }

        if (lp < 19999.0f)
        {
            left = layer.lp[0].processLowPass(left, lp, currentSampleRate);
            right = layer.lp[1].processLowPass(right, lp, currentSampleRate);
        }

        if (srcChannels == 1)
        {
            const float angle = (pan + 1.0f) * juce::MathConstants<float>::quarterPi;
            const float lg = std::cos(angle);
            const float rg = std::sin(angle);
            outL[n] += left * lg * gain;
            outR[n] += left * rg * gain;
        }
        else
        {
            const float lg = pan <= 0.0f ? 1.0f : 1.0f - pan;
            const float rg = pan >= 0.0f ? 1.0f : 1.0f + pan;
            outL[n] += left * lg * gain;
            outR[n] += right * rg * gain;
        }

        layer.position += 1.0;
        if (layer.position >= length)
            layer.position = (double) xfadeSamples;
    }
}

void SceneLooperAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    const bool soloMode = anySoloActive();
    for (int i = 0; i < numLayers; ++i)
        renderLayer(layers[i], i, buffer, buffer.getNumSamples(), soloMode);
}

juce::AudioProcessorEditor* SceneLooperAudioProcessor::createEditor()
{
    return new SceneLooperAudioProcessorEditor(*this);
}

void SceneLooperAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    auto files = juce::ValueTree("FILES");
    for (int i = 0; i < numLayers; ++i)
    {
        auto fileNode = juce::ValueTree("LAYER");
        fileNode.setProperty("index", i, nullptr);
        fileNode.setProperty("path", layers[i].file.getFullPathName(), nullptr);
        files.addChild(fileNode, -1, nullptr);
    }
    state.addChild(files, -1, nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SceneLooperAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml == nullptr)
        return;

    auto state = juce::ValueTree::fromXml(*xml);
    if (! state.isValid())
        return;

    auto files = state.getChildWithName("FILES");
    state.removeChild(files, nullptr);
    apvts.replaceState(state);

    for (int c = 0; c < files.getNumChildren(); ++c)
    {
        auto node = files.getChild(c);
        const int index = (int) node.getProperty("index", -1);
        const juce::File file(node.getProperty("path", {}).toString());
        if (juce::isPositiveAndBelow(index, numLayers) && file.existsAsFile())
        {
            juce::String error;
            loadFileForLayer(index, file, error);
        }
        else if (juce::isPositiveAndBelow(index, numLayers) && file.getFullPathName().isNotEmpty())
        {
            layers[index].displayName = "Missing file";
            layers[index].loaded = false;
        }
    }
}
