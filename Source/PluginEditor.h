#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class SaturatorProcessor;

//==============================================================================
/** Dark minimal look: near-black background, amber (sodium-light) accent. */
class SaturatorLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SaturatorLookAndFeel();

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override;

    static const juce::Colour background;
    static const juce::Colour accent;
    static const juce::Colour knobBody;
    static const juce::Colour trackBg;
    static const juce::Colour textColour;
};

//==============================================================================
class SaturatorEditor : public juce::AudioProcessorEditor
{
public:
    explicit SaturatorEditor (SaturatorProcessor&);
    ~SaturatorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SaturatorProcessor& processorRef;
    SaturatorLookAndFeel lookAndFeel;

    juce::Slider driveKnob, toneKnob, mixKnob, outKnob;
    juce::Label  driveLabel, toneLabel, mixLabel, outLabel, titleLabel;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> driveAttach, toneAttach, mixAttach, outAttach;

    void setupKnob (juce::Slider& knob, juce::Label& label, const juce::String& text,
                    double min, double max, double interval);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SaturatorEditor)
};
