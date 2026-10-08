#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SaturatorProcessor::SaturatorProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      oversampling (maxChannels,
                    oversamplingStages,
                    juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
                    true,   // maximum quality
                    true)   // integer latency (easier host compensation)
{
}

SaturatorProcessor::~SaturatorProcessor() = default;

//==============================================================================
bool SaturatorProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& in  = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();

    if (in.isDisabled() || out.isDisabled())
        return false;

    if (in != out)
        return false;

    return in == juce::AudioChannelSet::mono()
        || in == juce::AudioChannelSet::stereo();
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout SaturatorProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("drive", 1), "Drive",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 25.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("tone", 1), "Tone",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 70.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("mix", 1), "Mix",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("output", 1), "Output",
        juce::NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    return layout;
}

//==============================================================================
void SaturatorProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    oversampledRate = sampleRate * oversamplingFactor;

    oversampling.reset();
    oversampling.initProcessing (static_cast<size_t> (samplesPerBlock));

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = oversampledRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock * oversamplingFactor);
    spec.numChannels      = static_cast<juce::uint32> (maxChannels);

    for (auto& filter : toneFilters)
    {
        filter.prepare (spec);
        filter.reset();
    }

    // Smoothing runs at the oversampled rate because it is consumed per oversampled sample.
    driveGainSmoothed.reset (oversampledRate, 0.02);
    mixSmoothed.reset       (oversampledRate, 0.02);
    outGainSmoothed.reset   (oversampledRate, 0.02);

    setLatencySamples (static_cast<int> (std::ceil (oversampling.getLatencySamples())));
}

void SaturatorProcessor::releaseResources()
{
    oversampling.reset();
    for (auto& filter : toneFilters)
        filter.reset();
}

//==============================================================================
void SaturatorProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numChannels = buffer.getNumChannels();
    jassert (numChannels <= maxChannels);

    //---- Read parameters -----------------------------------------------------
    const float drive01 = *apvts.getRawParameterValue ("drive")  / 100.0f;
    const float tone01  = *apvts.getRawParameterValue ("tone")   / 100.0f;
    const float mix01   = *apvts.getRawParameterValue ("mix")    / 100.0f;
    const float outDb   = *apvts.getRawParameterValue ("output");

    // Drive knob -> pre-gain into the waveshaper. Quadratic curve keeps the
    // low end of the knob subtle and the top end aggressive.
    const float driveGainTarget = 1.0f + drive01 * drive01 * 40.0f;
    // Tone knob -> lowpass cutoff on the saturated signal, 2 kHz .. 20 kHz.
    const float toneFreq        = 2000.0f * std::pow (10.0f, tone01);
    const float outGainTarget   = juce::Decibels::decibelsToGain (outDb);

    driveGainSmoothed.setTargetValue (driveGainTarget);
    mixSmoothed.setTargetValue (mix01);
    outGainSmoothed.setTargetValue (outGainTarget);

    auto toneCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass (oversampledRate, toneFreq);
    for (int ch = 0; ch < numChannels; ++ch)
        toneFilters[(size_t) ch].coefficients = toneCoeffs;

    //---- Oversampled saturation ----------------------------------------------
    juce::dsp::AudioBlock<float> block (buffer);
    auto upBlock = oversampling.processSamplesUp (block);

    const auto upChannels = upBlock.getNumChannels();
    const auto upSamples  = upBlock.getNumSamples();

    for (size_t ch = 0; ch < upChannels; ++ch)
    {
        float* data = upBlock.getChannelPointer (ch);
        auto& toneFilter = toneFilters[ch];

        for (size_t i = 0; i < upSamples; ++i)
        {
            const float dry    = data[i];
            const float driven = dry * driveGainSmoothed.getNextValue();
            const float shaped = std::tanh (driven);                    // the saturation
            const float toned  = toneFilter.processSample (shaped);    // tame the harshness
            const float mix    = mixSmoothed.getNextValue();
            const float outGain = outGainSmoothed.getNextValue();

            data[i] = (dry * (1.0f - mix) + toned * mix) * outGain;
        }
    }

    oversampling.processSamplesDown (block);
}

//==============================================================================
juce::AudioProcessorEditor* SaturatorProcessor::createEditor()
{
    return new SaturatorEditor (*this);
}

//==============================================================================
void SaturatorProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void SaturatorProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}
