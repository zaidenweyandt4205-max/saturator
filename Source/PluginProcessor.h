#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

//==============================================================================
/**
    Mantra Saturator — a one-knob-style saturator with Drive, Tone, Mix and Output.

    DSP chain (per sample, inside 4x oversampling):
        input -> drive gain -> tanh waveshaper -> lowpass tone filter -> dry/wet mix
    Output gain is applied after the mix, also smoothed.
*/
class SaturatorProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SaturatorProcessor();
    ~SaturatorProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override                                         { return true; }

    //==============================================================================
    const juce::String getName() const override                            { return "Mantra Saturator"; }

    bool acceptsMidi() const override                                      { return false; }
    bool producesMidi() const override                                     { return false; }
    bool isMidiEffect() const override                                     { return false; }
    double getTailLengthSeconds() const override                           { return 0.0; }

    //==============================================================================
    int getNumPrograms() override                                           { return 1; }
    int getCurrentProgram() override                                        { return 0; }
    void setCurrentProgram (int) override                                   {}
    const juce::String getProgramName (int) override                       { return {}; }
    void changeProgramName (int, const juce::String&) override              {}

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Parameters", createParameterLayout() };

private:
    //==============================================================================
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    static constexpr int oversamplingStages = 2;                 // 2 stages = 4x
    static constexpr int oversamplingFactor = 1 << oversamplingStages;
    static constexpr int maxChannels = 2;

    juce::dsp::Oversampling<float> oversampling;
    std::array<juce::dsp::IIR::Filter<float>, maxChannels> toneFilters;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> driveGainSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outGainSmoothed;

    double oversampledRate = 44100.0 * oversamplingFactor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SaturatorProcessor)
};
