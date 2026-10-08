#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
const juce::Colour SaturatorLookAndFeel::background (0xff111111);
const juce::Colour SaturatorLookAndFeel::accent     (0xffE8821E);  // sodium amber
const juce::Colour SaturatorLookAndFeel::knobBody   (0xff1e1e1e);
const juce::Colour SaturatorLookAndFeel::trackBg    (0xff2b2b2b);
const juce::Colour SaturatorLookAndFeel::textColour (0xffd8d8d8);

SaturatorLookAndFeel::SaturatorLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId, textColour);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId, textColour);
}

void SaturatorLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPosProportional,
                                             float rotaryStartAngle, float rotaryEndAngle,
                                             juce::Slider& slider)
{
    const float radius  = juce::jmin (width, height) * 0.40f;
    const float centreX = x + width  * 0.5f;
    const float centreY = y + height * 0.5f;
    const float angle   = rotaryStartAngle
                        + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // Track arc (background)
    juce::Path track;
    track.addCentredArc (centreX, centreY, radius, radius, 0.0f,
                         rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (trackBg);
    g.strokePath (track, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

    // Value arc
    juce::Path value;
    value.addCentredArc (centreX, centreY, radius, radius, 0.0f,
                         rotaryStartAngle, angle, true);
    g.setColour (slider.isEnabled() ? accent : juce::Colour (0xff555555));
    g.strokePath (value, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

    // Knob body
    const float bodyR = radius * 0.68f;
    g.setColour (knobBody);
    g.fillEllipse (centreX - bodyR, centreY - bodyR, bodyR * 2.0f, bodyR * 2.0f);
    g.setColour (juce::Colour (0xff333333));
    g.drawEllipse (centreX - bodyR, centreY - bodyR, bodyR * 2.0f, bodyR * 2.0f, 1.0f);

    // Pointer (angle 0 = 12 o'clock, positive = clockwise)
    const float pointerLen = bodyR * 0.85f;
    const float px = centreX + std::cos (angle - juce::MathConstants<float>::halfPi) * pointerLen;
    const float py = centreY + std::sin (angle - juce::MathConstants<float>::halfPi) * pointerLen;
    juce::Path pointer;
    pointer.addLineSegment (juce::Line<float> (centreX, centreY, px, py), 3.0f);
    g.setColour (accent);
    g.strokePath (pointer, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
}

//==============================================================================
SaturatorEditor::SaturatorEditor (SaturatorProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setLookAndFeel (&lookAndFeel);

    setupKnob (driveKnob, driveLabel, "DRIVE", 0.0, 100.0, 0.1);
    setupKnob (toneKnob,  toneLabel,  "TONE",  0.0, 100.0, 0.1);
    setupKnob (mixKnob,   mixLabel,   "MIX",   0.0, 100.0, 0.1);
    setupKnob (outKnob,   outLabel,   "OUTPUT", -12.0, 12.0, 0.1);

    titleLabel.setText ("MANTRA SATURATOR", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (18.0f, juce::Font::bold));
    titleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (titleLabel);

    auto& apvts = processorRef.apvts;
    driveAttach = std::make_unique<Attachment> (apvts, "drive",  driveKnob);
    toneAttach  = std::make_unique<Attachment> (apvts, "tone",   toneKnob);
    mixAttach   = std::make_unique<Attachment> (apvts, "mix",    mixKnob);
    outAttach   = std::make_unique<Attachment> (apvts, "output", outKnob);

    setSize (480, 300);
}

SaturatorEditor::~SaturatorEditor()
{
    setLookAndFeel (nullptr);
}

void SaturatorEditor::setupKnob (juce::Slider& knob, juce::Label& label, const juce::String& text,
                                 double min, double max, double interval)
{
    knob.setRange (min, max, interval);
    knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    knob.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                              juce::MathConstants<float>::pi * 2.75f,
                              true);
    knob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 20);
    addAndMakeVisible (knob);

    label.setText (text, juce::dontSendNotification);
    label.setFont (juce::Font (12.0f, juce::Font::bold));
    label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (label);
}

void SaturatorEditor::paint (juce::Graphics& g)
{
    g.fillAll (SaturatorLookAndFeel::background);
}

void SaturatorEditor::resized()
{
    auto area = getLocalBounds();
    titleLabel.setBounds (area.removeFromTop (44));

    const int knobW = area.getWidth() / 4;
    auto place = [&] (juce::Slider& knob, juce::Label& label, int index)
    {
        auto col = area.withX (index * knobW).withWidth (knobW);
        auto labelArea = col.removeFromBottom (24);
        label.setBounds (labelArea);
        knob.setBounds (col.reduced (8, 4));
    };

    place (driveKnob, driveLabel, 0);
    place (toneKnob,  toneLabel,  1);
    place (mixKnob,   mixLabel,   2);
    place (outKnob,   outLabel,   3);
}
