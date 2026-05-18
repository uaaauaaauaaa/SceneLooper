#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>
#include <cstdint>

namespace
{
float getFloatPropertyOrDefault(juce::DynamicObject& object, const char* name, float defaultValue)
{
    const auto value = object.getProperty(juce::Identifier(name));
    return value.isVoid() ? defaultValue : (float) value;
}
}

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
    params.push_back(std::make_unique<juce::AudioParameterFloat>("masterLowCut", "Master Low Cut",
        juce::NormalisableRange<float>(20.0f, 1000.0f, 1.0f, 0.35f), 20.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("masterHighCut", "Master High Cut",
        juce::NormalisableRange<float>(1000.0f, 20000.0f, 1.0f, 0.35f), 20000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("randomStart", "Random Start",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 30.0f));

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
        params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "speed", nice + "Speed",
            juce::NormalisableRange<float>(38.0f, 55.0f, 0.1f), 48.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "drift", nice + "Drift",
            juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "width", nice + "Width",
            juce::NormalisableRange<float>(0.0f, 120.0f, 1.0f), 100.0f));
        params.push_back(std::make_unique<juce::AudioParameterBool>(prefix + "autoPanOn", nice + "AutoPan On", false));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "autoPanAmount", nice + "AutoPan Amount",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(prefix + "autoPanRate", nice + "AutoPan Rate",
            juce::NormalisableRange<float>(0.01f, 1.0f, 0.001f, 0.35f), 0.25f));
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

float SceneLooperAudioProcessor::getParameterValue(const juce::String& id) const
{
    if (auto* value = apvts.getRawParameterValue(id))
        return value->load();

    return 0.0f;
}

void SceneLooperAudioProcessor::setParameterValue(const juce::String& id, float value)
{
    if (auto* parameter = apvts.getParameter(id))
    {
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
        parameter->endChangeGesture();
    }
}

void SceneLooperAudioProcessor::markLayerMissingFile(int layerIndex, const juce::File& file)
{
    if (! juce::isPositiveAndBelow(layerIndex, numLayers))
        return;

    auto& layer = layers[layerIndex];
    layer.file = file;
    layer.displayName = "Missing file";
    layer.loaded = false;
    layer.audio.setSize(0, 0);
    layer.lengthSeconds.store(0.0, std::memory_order_relaxed);
    layer.displayPositionSamples.store(0.0, std::memory_order_relaxed);
    layer.pendingSeekFraction.store(-1.0, std::memory_order_relaxed);
    layer.outputLevel.store(0.0f, std::memory_order_relaxed);
    layer.waveformPreview.fill(0.0f);
    layer.waveformPreviewReady.store(false, std::memory_order_relaxed);
}

void SceneLooperAudioProcessor::clearLayerFile(int layerIndex)
{
    if (! juce::isPositiveAndBelow(layerIndex, numLayers))
        return;

    auto& layer = layers[layerIndex];
    layer.file = juce::File();
    layer.displayName = "No file";
    layer.loaded = false;
    layer.audio.setSize(0, 0);
    layer.lengthSeconds.store(0.0, std::memory_order_relaxed);
    layer.displayPositionSamples.store(0.0, std::memory_order_relaxed);
    layer.pendingSeekFraction.store(-1.0, std::memory_order_relaxed);
    layer.outputLevel.store(0.0f, std::memory_order_relaxed);
    layer.waveformPreview.fill(0.0f);
    layer.waveformPreviewReady.store(false, std::memory_order_relaxed);
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
    const auto randomStartAmount = apvts.getRawParameterValue("randomStart")->load() * 0.01;

    for (int i = 0; i < numLayers; ++i)
    {
        const auto offsetSeconds = apvts.getRawParameterValue(paramId(i, "offset"))->load();
        layers[i].position = std::max(0.0, (double) offsetSeconds * currentSampleRate);
        if (layers[i].audio.getNumSamples() > 0)
        {
            if (randomStartAmount > 0.0f)
                layers[i].position += random.nextDouble() * (double) layers[i].audio.getNumSamples() * (double) randomStartAmount;

            layers[i].position = std::fmod(layers[i].position, (double) layers[i].audio.getNumSamples());
            layers[i].displayPositionSamples.store(std::fmod(layers[i].position, (double) layers[i].audio.getNumSamples()),
                std::memory_order_relaxed);
        }
        else
        {
            layers[i].displayPositionSamples.store(0.0, std::memory_order_relaxed);
        }
        layers[i].autoPanPhase = 0.0;
        layers[i].driftPhase = 0.0;
        layers[i].outputLevel.store(0.0f, std::memory_order_relaxed);
        for (auto& f : layers[i].hp) f.reset();
        for (auto& f : layers[i].lp) f.reset();
    }

    masterOutputLevel.store(0.0f, std::memory_order_relaxed);
    masterOutputLevelLeft.store(0.0f, std::memory_order_relaxed);
    masterOutputLevelRight.store(0.0f, std::memory_order_relaxed);
    for (auto& f : masterHP) f.reset();
    for (auto& f : masterLP) f.reset();
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

    if (reader->lengthInSamples <= 0)
    {
        errorMessage = "Empty WAV";
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
    layer.lengthSeconds.store((double) reader->lengthInSamples / reader->sampleRate, std::memory_order_relaxed);
    layer.displayPositionSamples.store(std::fmod(layer.position, (double) layer.audio.getNumSamples()), std::memory_order_relaxed);
    layer.pendingSeekFraction.store(-1.0, std::memory_order_relaxed);
    buildWaveformPreview(layerIndex);

    errorMessage.clear();
    return true;
}

void SceneLooperAudioProcessor::buildWaveformPreview(int layerIndex)
{
    if (! juce::isPositiveAndBelow(layerIndex, numLayers))
        return;

    auto& layer = layers[layerIndex];
    layer.waveformPreview.fill(0.0f);

    const int numSamples = layer.audio.getNumSamples();
    const int numChannels = layer.audio.getNumChannels();
    if (numSamples <= 0 || numChannels <= 0)
    {
        layer.waveformPreviewReady.store(false, std::memory_order_relaxed);
        return;
    }

    for (int point = 0; point < waveformPreviewPoints; ++point)
    {
        const int start = (int) (((int64_t) point * numSamples) / waveformPreviewPoints);
        const int end = juce::jmax(start + 1, (int) (((int64_t) (point + 1) * numSamples) / waveformPreviewPoints));
        float peak = 0.0f;

        for (int sample = start; sample < juce::jmin(end, numSamples); ++sample)
        {
            float summed = 0.0f;
            for (int channel = 0; channel < numChannels; ++channel)
                summed += std::abs(layer.audio.getSample(channel, sample));

            peak = juce::jmax(peak, summed / (float) numChannels);
        }

        layer.waveformPreview[(size_t) point] = juce::jlimit(0.0f, 1.0f, peak);
    }

    float maxPeak = 0.0f;
    for (const auto peak : layer.waveformPreview)
        maxPeak = juce::jmax(maxPeak, peak);

    if (maxPeak > 0.0f)
    {
        constexpr float targetPeak = 0.82f;
        constexpr float maxVisualGain = 48.0f;
        const float visualGain = juce::jlimit(0.0f, maxVisualGain, targetPeak / maxPeak);

        for (auto& peak : layer.waveformPreview)
            peak = juce::jlimit(0.0f, targetPeak, peak * visualGain);
    }

    layer.waveformPreviewReady.store(true, std::memory_order_relaxed);
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

double SceneLooperAudioProcessor::getLayerLengthSeconds(int layerIndex) const
{
    if (! isLayerLoaded(layerIndex))
        return -1.0;

    return layers[layerIndex].lengthSeconds.load(std::memory_order_relaxed);
}

double SceneLooperAudioProcessor::getLayerRemainingSeconds(int layerIndex) const
{
    if (! isLayerLoaded(layerIndex))
        return -1.0;

    const auto lengthSeconds = layers[layerIndex].lengthSeconds.load(std::memory_order_relaxed);
    if (lengthSeconds <= 0.0)
        return -1.0;

    const auto positionSeconds = layers[layerIndex].displayPositionSamples.load(std::memory_order_relaxed) / currentSampleRate;
    return juce::jlimit(0.0, lengthSeconds, lengthSeconds - std::fmod(positionSeconds, lengthSeconds));
}

double SceneLooperAudioProcessor::getLayerPlaybackPositionFraction(int layerIndex) const
{
    if (! isLayerLoaded(layerIndex))
        return -1.0;

    const int numSamples = layers[layerIndex].audio.getNumSamples();
    if (numSamples <= 0)
        return -1.0;

    const auto position = layers[layerIndex].displayPositionSamples.load(std::memory_order_relaxed);
    return juce::jlimit(0.0, 1.0, position / (double) juce::jmax(1, numSamples - 1));
}

void SceneLooperAudioProcessor::seekLayerToFraction(int layerIndex, double fraction)
{
    if (! isLayerLoaded(layerIndex))
        return;

    const int numSamples = layers[layerIndex].audio.getNumSamples();
    if (numSamples <= 0)
        return;

    const auto clampedFraction = juce::jlimit(0.0, 1.0, fraction);
    const auto targetSample = juce::jlimit(0.0, (double) juce::jmax(0, numSamples - 1),
        clampedFraction * (double) juce::jmax(0, numSamples - 1));
    layers[layerIndex].displayPositionSamples.store(targetSample, std::memory_order_relaxed);
    layers[layerIndex].pendingSeekFraction.store(clampedFraction, std::memory_order_relaxed);
}

bool SceneLooperAudioProcessor::copyWaveformPreview(int layerIndex, std::array<float, waveformPreviewPoints>& destination) const
{
    if (! juce::isPositiveAndBelow(layerIndex, numLayers)
        || ! layers[layerIndex].loaded
        || ! layers[layerIndex].waveformPreviewReady.load(std::memory_order_relaxed))
    {
        destination.fill(0.0f);
        return false;
    }

    destination = layers[layerIndex].waveformPreview;
    return true;
}

float SceneLooperAudioProcessor::getLayerLevel(int layerIndex) const
{
    if (! juce::isPositiveAndBelow(layerIndex, numLayers))
        return 0.0f;

    return layers[layerIndex].outputLevel.load(std::memory_order_relaxed);
}

float SceneLooperAudioProcessor::getLayerWaveformDisplayGain(int layerIndex) const
{
    if (! juce::isPositiveAndBelow(layerIndex, numLayers))
        return 1.0f;

    const auto volumeDb = getParameterValue(paramId(layerIndex, "volume"));
    const auto gain = juce::Decibels::decibelsToGain(volumeDb, -60.0f);
    return juce::jlimit(0.08f, 1.0f, std::pow(gain, 0.35f));
}

float SceneLooperAudioProcessor::getMasterLevel() const
{
    return masterOutputLevel.load(std::memory_order_relaxed);
}

float SceneLooperAudioProcessor::getMasterLeftLevel() const
{
    return masterOutputLevelLeft.load(std::memory_order_relaxed);
}

float SceneLooperAudioProcessor::getMasterRightLevel() const
{
    return masterOutputLevelRight.load(std::memory_order_relaxed);
}

void SceneLooperAudioProcessor::randomizeLayerStarts()
{
    resetLayerPlayback();
}

juce::String SceneLooperAudioProcessor::getCurrentSceneName() const
{
    return currentSceneName;
}

void SceneLooperAudioProcessor::setCurrentSceneName(const juce::String& sceneName)
{
    currentSceneName = sceneName.isNotEmpty() ? sceneName : "Project State";
}

bool SceneLooperAudioProcessor::saveSceneToFile(const juce::File& file, juce::String& errorMessage) const
{
    juce::var rootVar(new juce::DynamicObject());
    auto* root = rootVar.getDynamicObject();

    juce::var globalVar(new juce::DynamicObject());
    auto* global = globalVar.getDynamicObject();
    global->setProperty("masterVolume", getParameterValue("masterVolume"));
    global->setProperty("globalXFade", getParameterValue("globalXFade"));
    global->setProperty("masterLowCut", getParameterValue("masterLowCut"));
    global->setProperty("masterHighCut", getParameterValue("masterHighCut"));
    global->setProperty("randomStart", getParameterValue("randomStart"));
    root->setProperty("global", globalVar);

    juce::Array<juce::var> layerArray;
    for (int i = 0; i < numLayers; ++i)
    {
        juce::var layerVar(new juce::DynamicObject());
        auto* layer = layerVar.getDynamicObject();

        layer->setProperty("filePath", layers[i].file.getFullPathName());
        layer->setProperty("enabled", getParameterValue(paramId(i, "on")) > 0.5f);
        layer->setProperty("solo", getParameterValue(paramId(i, "solo")) > 0.5f);
        layer->setProperty("volume", getParameterValue(paramId(i, "volume")));
        layer->setProperty("pan", getParameterValue(paramId(i, "pan")));
        layer->setProperty("speed", getParameterValue(paramId(i, "speed")));
        layer->setProperty("drift", getParameterValue(paramId(i, "drift")));
        layer->setProperty("width", getParameterValue(paramId(i, "width")));
        layer->setProperty("autoPanEnabled", getParameterValue(paramId(i, "autoPanOn")) > 0.5f);
        layer->setProperty("autoPanAmount", getParameterValue(paramId(i, "autoPanAmount")));
        layer->setProperty("autoPanRateHz", getParameterValue(paramId(i, "autoPanRate")));
        layer->setProperty("hp", getParameterValue(paramId(i, "hp")));
        layer->setProperty("lp", getParameterValue(paramId(i, "lp")));
        layer->setProperty("xfade", getParameterValue(paramId(i, "xfade")));
        layer->setProperty("startOffset", getParameterValue(paramId(i, "offset")));

        layerArray.add(layerVar);
    }

    root->setProperty("layers", layerArray);

    auto target = file;
    if (target.getFileExtension() != ".scene")
        target = target.withFileExtension(".scene");

    if (! target.replaceWithText(juce::JSON::toString(rootVar, true)))
    {
        errorMessage = "Could not write scene file";
        return false;
    }

    errorMessage.clear();
    return true;
}

bool SceneLooperAudioProcessor::loadSceneFromFile(const juce::File& file, juce::String& errorMessage)
{
    if (! file.existsAsFile())
    {
        errorMessage = "Scene file not found";
        return false;
    }

    juce::var scene;
    const auto parseResult = juce::JSON::parse(file.loadFileAsString(), scene);
    if (parseResult.failed() || ! scene.isObject())
    {
        errorMessage = "Invalid scene file";
        return false;
    }

    auto* root = scene.getDynamicObject();
    auto globalVar = root->getProperty("global");
    if (globalVar.isObject())
    {
        auto* global = globalVar.getDynamicObject();
        setParameterValue("masterVolume", (float) global->getProperty("masterVolume"));
        setParameterValue("globalXFade", (float) global->getProperty("globalXFade"));
        setParameterValue("masterLowCut", getFloatPropertyOrDefault(*global, "masterLowCut", 20.0f));
        setParameterValue("masterHighCut", getFloatPropertyOrDefault(*global, "masterHighCut", 20000.0f));
        setParameterValue("randomStart", getFloatPropertyOrDefault(*global, "randomStart", 30.0f));
    }

    auto layersVar = root->getProperty("layers");
    if (auto* layerArray = layersVar.getArray())
    {
        const int count = juce::jmin(numLayers, layerArray->size());
        for (int i = 0; i < count; ++i)
        {
            if (! layerArray->getReference(i).isObject())
                continue;

            auto* layer = layerArray->getReference(i).getDynamicObject();
            setParameterValue(paramId(i, "on"), (bool) layer->getProperty("enabled") ? 1.0f : 0.0f);
            setParameterValue(paramId(i, "solo"), (bool) layer->getProperty("solo") ? 1.0f : 0.0f);
            setParameterValue(paramId(i, "volume"), (float) layer->getProperty("volume"));
            setParameterValue(paramId(i, "pan"), (float) layer->getProperty("pan"));
            setParameterValue(paramId(i, "speed"), getFloatPropertyOrDefault(*layer, "speed", 48.0f));
            setParameterValue(paramId(i, "drift"), getFloatPropertyOrDefault(*layer, "drift", 0.0f));
            setParameterValue(paramId(i, "width"), getFloatPropertyOrDefault(*layer, "width", 100.0f));
            setParameterValue(paramId(i, "autoPanOn"), (bool) layer->getProperty("autoPanEnabled") ? 1.0f : 0.0f);
            setParameterValue(paramId(i, "autoPanAmount"), (float) layer->getProperty("autoPanAmount"));
            setParameterValue(paramId(i, "autoPanRate"), (float) layer->getProperty("autoPanRateHz"));
            setParameterValue(paramId(i, "hp"), (float) layer->getProperty("hp"));
            setParameterValue(paramId(i, "lp"), (float) layer->getProperty("lp"));
            setParameterValue(paramId(i, "xfade"), (float) layer->getProperty("xfade"));
            setParameterValue(paramId(i, "offset"), (float) layer->getProperty("startOffset"));

            const auto path = layer->getProperty("filePath").toString();
            if (path.isEmpty())
            {
                clearLayerFile(i);
                continue;
            }

            const juce::File wavFile(path);
            if (wavFile.existsAsFile())
            {
                juce::String loadError;
                if (! loadFileForLayer(i, wavFile, loadError))
                    markLayerMissingFile(i, wavFile);
            }
            else
            {
                markLayerMissingFile(i, wavFile);
            }
        }
    }

    setCurrentSceneName(file.getFileNameWithoutExtension());
    resetLayerPlayback();
    errorMessage.clear();
    return true;
}

void SceneLooperAudioProcessor::renderLayer(Layer& layer, int layerIndex, juce::AudioBuffer<float>& output, int numSamples, bool soloMode)
{
    if (! layer.loaded || layer.audio.getNumSamples() <= 0)
    {
        layer.outputLevel.store(0.0f, std::memory_order_relaxed);
        return;
    }

    const bool on = apvts.getRawParameterValue(paramId(layerIndex, "on"))->load() > 0.5f;
    const bool solo = apvts.getRawParameterValue(paramId(layerIndex, "solo"))->load() > 0.5f;
    if (! on || (soloMode && ! solo))
    {
        layer.outputLevel.store(0.0f, std::memory_order_relaxed);
        return;
    }

    const float layerVolumeDb = apvts.getRawParameterValue(paramId(layerIndex, "volume"))->load();
    const float masterVolumeDb = apvts.getRawParameterValue("masterVolume")->load();
    const float gain = juce::Decibels::decibelsToGain(layerVolumeDb + masterVolumeDb, -90.0f);
    const float pan = apvts.getRawParameterValue(paramId(layerIndex, "pan"))->load();
    const float speedKhz = apvts.getRawParameterValue(paramId(layerIndex, "speed"))->load();
    const float driftPercent = apvts.getRawParameterValue(paramId(layerIndex, "drift"))->load();
    const float widthPercent = apvts.getRawParameterValue(paramId(layerIndex, "width"))->load();
    const bool autoPanOn = apvts.getRawParameterValue(paramId(layerIndex, "autoPanOn"))->load() > 0.5f;
    const float autoPanAmount = apvts.getRawParameterValue(paramId(layerIndex, "autoPanAmount"))->load();
    const float autoPanRate = apvts.getRawParameterValue(paramId(layerIndex, "autoPanRate"))->load();
    const float hp = apvts.getRawParameterValue(paramId(layerIndex, "hp"))->load();
    const float lp = apvts.getRawParameterValue(paramId(layerIndex, "lp"))->load();
    const float xfadeSeconds = apvts.getRawParameterValue(paramId(layerIndex, "xfade"))->load();
    const double autoPanPhaseDelta = juce::MathConstants<double>::twoPi * (double) autoPanRate / currentSampleRate;
    constexpr double driftRateHz = 0.071;
    constexpr double maxDriftDepth = 0.03;
    const double driftPhaseDelta = juce::MathConstants<double>::twoPi * driftRateHz / currentSampleRate;

    const int srcChannels = layer.audio.getNumChannels();
    const int length = layer.audio.getNumSamples();
    const auto pendingSeek = layer.pendingSeekFraction.exchange(-1.0, std::memory_order_relaxed);
    if (pendingSeek >= 0.0)
    {
        layer.position = juce::jlimit(0.0, (double) juce::jmax(0, length - 1),
            pendingSeek * (double) juce::jmax(0, length - 1));
        layer.displayPositionSamples.store(layer.position, std::memory_order_relaxed);
    }

    int xfadeSamples = (int) std::round(xfadeSeconds * currentSampleRate);
    xfadeSamples = juce::jlimit(0, std::max(0, length / 2 - 1), xfadeSamples);

    auto* outL = output.getWritePointer(0);
    auto* outR = output.getWritePointer(1);
    const auto* srcL = layer.audio.getReadPointer(0);
    const auto* srcR = srcChannels > 1 ? layer.audio.getReadPointer(1) : nullptr;
    double displayPositionSamples = layer.displayPositionSamples.load(std::memory_order_relaxed);
    float layerPeak = 0.0f;

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

        if (srcChannels > 1)
        {
            const float width = widthPercent * 0.01f;
            const float mid = (left + right) * 0.5f;
            const float side = (left - right) * 0.5f * width;
            left = mid + side;
            right = mid - side;
        }

        if (srcChannels == 1)
        {
            float effectivePan = pan;
            if (autoPanOn)
            {
                effectivePan = juce::jlimit(-1.0f, 1.0f,
                    pan + (float) std::sin(layer.autoPanPhase) * autoPanAmount);
            }

            const float angle = (effectivePan + 1.0f) * juce::MathConstants<float>::halfPi * 0.5f;
            const float lg = std::cos(angle);
            const float rg = std::sin(angle);
            const float renderedL = left * lg * gain;
            const float renderedR = left * rg * gain;
            outL[n] += renderedL;
            outR[n] += renderedR;
            layerPeak = juce::jmax(layerPeak, juce::jmax(std::abs(renderedL), std::abs(renderedR)));
        }
        else
        {
            float effectivePan = pan;
            if (autoPanOn)
            {
                effectivePan = juce::jlimit(-1.0f, 1.0f,
                    pan + (float) std::sin(layer.autoPanPhase) * autoPanAmount);
            }

            const float lg = effectivePan <= 0.0f ? 1.0f : 1.0f - effectivePan;
            const float rg = effectivePan >= 0.0f ? 1.0f : 1.0f + effectivePan;
            const float renderedL = left * lg * gain;
            const float renderedR = right * rg * gain;
            outL[n] += renderedL;
            outR[n] += renderedR;
            layerPeak = juce::jmax(layerPeak, juce::jmax(std::abs(renderedL), std::abs(renderedR)));
        }

        if (autoPanOn)
        {
            layer.autoPanPhase += autoPanPhaseDelta;
            if (layer.autoPanPhase >= juce::MathConstants<double>::twoPi)
                layer.autoPanPhase -= juce::MathConstants<double>::twoPi;
        }

        const double drift = driftPercent > 0.0f
            ? std::sin(layer.driftPhase) * maxDriftDepth * ((double) driftPercent / 100.0)
            : 0.0;
        const double playbackRatio = juce::jmax(0.01, ((double) speedKhz / 48.0) * (1.0 + drift));

        if (driftPercent > 0.0f)
        {
            layer.driftPhase += driftPhaseDelta;
            if (layer.driftPhase >= juce::MathConstants<double>::twoPi)
                layer.driftPhase -= juce::MathConstants<double>::twoPi;
        }

        layer.position += playbackRatio;
        if (layer.position >= length)
            layer.position = (double) xfadeSamples + std::fmod(layer.position - (double) xfadeSamples, (double) juce::jmax(1, length - xfadeSamples));

        displayPositionSamples += playbackRatio;
        if (displayPositionSamples >= length)
            displayPositionSamples = std::fmod(displayPositionSamples, (double) length);
    }

    layer.displayPositionSamples.store(displayPositionSamples, std::memory_order_relaxed);
    layer.outputLevel.store(juce::jlimit(0.0f, 1.0f, layerPeak), std::memory_order_relaxed);
}

void SceneLooperAudioProcessor::applyMasterProcessing(juce::AudioBuffer<float>& buffer)
{
    const float lowCut = apvts.getRawParameterValue("masterLowCut")->load();
    const float highCut = apvts.getRawParameterValue("masterHighCut")->load();
    float peak = 0.0f;
    float leftPeak = 0.0f;
    float rightPeak = 0.0f;

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* data = buffer.getWritePointer(channel);
        auto& hp = masterHP[juce::jmin(channel, 1)];
        auto& lp = masterLP[juce::jmin(channel, 1)];

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float value = data[sample];
            if (lowCut > 20.5f)
                value = hp.processHighPass(value, lowCut, currentSampleRate);
            if (highCut < 19999.0f)
                value = lp.processLowPass(value, highCut, currentSampleRate);

            data[sample] = value;
            const auto absValue = std::abs(value);
            peak = juce::jmax(peak, absValue);
            if (channel == 0)
                leftPeak = juce::jmax(leftPeak, absValue);
            else if (channel == 1)
                rightPeak = juce::jmax(rightPeak, absValue);
        }
    }

    const auto previous = masterOutputLevel.load(std::memory_order_relaxed);
    masterOutputLevel.store(juce::jlimit(0.0f, 1.0f, juce::jmax(peak, previous * 0.82f)),
        std::memory_order_relaxed);
    const auto previousLeft = masterOutputLevelLeft.load(std::memory_order_relaxed);
    const auto previousRight = masterOutputLevelRight.load(std::memory_order_relaxed);
    if (buffer.getNumChannels() == 1)
        rightPeak = leftPeak;

    masterOutputLevelLeft.store(juce::jlimit(0.0f, 1.0f, juce::jmax(leftPeak, previousLeft * 0.82f)),
        std::memory_order_relaxed);
    masterOutputLevelRight.store(juce::jlimit(0.0f, 1.0f, juce::jmax(rightPeak, previousRight * 0.82f)),
        std::memory_order_relaxed);
}

void SceneLooperAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    const bool soloMode = anySoloActive();
    for (int i = 0; i < numLayers; ++i)
        renderLayer(layers[i], i, buffer, buffer.getNumSamples(), soloMode);

    applyMasterProcessing(buffer);
}

juce::AudioProcessorEditor* SceneLooperAudioProcessor::createEditor()
{
    return new SceneLooperAudioProcessorEditor(*this);
}

void SceneLooperAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    auto scene = juce::ValueTree("SCENE");
    scene.setProperty("name", getCurrentSceneName(), nullptr);
    state.addChild(scene, -1, nullptr);

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

    auto scene = state.getChildWithName("SCENE");
    if (scene.isValid())
    {
        auto restoredName = scene.getProperty("name", "Project State").toString();
        if (restoredName.isEmpty() || restoredName == "Untitled Scene")
            restoredName = "Project State";
        setCurrentSceneName(restoredName);
        state.removeChild(scene, nullptr);
    }
    else
    {
        setCurrentSceneName("Project State");
    }

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
            markLayerMissingFile(index, file);
        }
    }

    resetLayerPlayback();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SceneLooperAudioProcessor();
}
