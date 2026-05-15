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
        layers[i].autoPanPhase = 0.0;
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

bool SceneLooperAudioProcessor::saveSceneToFile(const juce::File& file, juce::String& errorMessage) const
{
    juce::var rootVar(new juce::DynamicObject());
    auto* root = rootVar.getDynamicObject();

    juce::var globalVar(new juce::DynamicObject());
    auto* global = globalVar.getDynamicObject();
    global->setProperty("masterVolume", getParameterValue("masterVolume"));
    global->setProperty("globalXFade", getParameterValue("globalXFade"));
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

    resetLayerPlayback();
    errorMessage.clear();
    return true;
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
    const bool autoPanOn = apvts.getRawParameterValue(paramId(layerIndex, "autoPanOn"))->load() > 0.5f;
    const float autoPanAmount = apvts.getRawParameterValue(paramId(layerIndex, "autoPanAmount"))->load();
    const float autoPanRate = apvts.getRawParameterValue(paramId(layerIndex, "autoPanRate"))->load();
    const float hp = apvts.getRawParameterValue(paramId(layerIndex, "hp"))->load();
    const float lp = apvts.getRawParameterValue(paramId(layerIndex, "lp"))->load();
    const float xfadeSeconds = apvts.getRawParameterValue(paramId(layerIndex, "xfade"))->load();
    const double autoPanPhaseDelta = juce::MathConstants<double>::twoPi * (double) autoPanRate / currentSampleRate;

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
            float effectivePan = pan;
            if (autoPanOn)
            {
                effectivePan = juce::jlimit(-1.0f, 1.0f,
                    pan + (float) std::sin(layer.autoPanPhase) * autoPanAmount);
            }

            const float angle = (effectivePan + 1.0f) * juce::MathConstants<float>::halfPi * 0.5f;
            const float lg = std::cos(angle);
            const float rg = std::sin(angle);
            outL[n] += left * lg * gain;
            outR[n] += left * rg * gain;
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
            outL[n] += left * lg * gain;
            outR[n] += right * rg * gain;
        }

        if (autoPanOn)
        {
            layer.autoPanPhase += autoPanPhaseDelta;
            if (layer.autoPanPhase >= juce::MathConstants<double>::twoPi)
                layer.autoPanPhase -= juce::MathConstants<double>::twoPi;
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
            markLayerMissingFile(index, file);
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SceneLooperAudioProcessor();
}
