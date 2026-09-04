#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

//==============================================================
// RG G4 LOOK AND FEEL
//==============================================================

class G4LookAndFeel : public juce::LookAndFeel_V4
{
public:
    G4LookAndFeel();

    void drawRotarySlider (
        juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider& slider) override;

    void drawLinearSlider (
        juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPos,
        float minSliderPos,
        float maxSliderPos,
        juce::Slider::SliderStyle style,
        juce::Slider& slider) override;

    void drawToggleButton (
        juce::Graphics& g,
        juce::ToggleButton& button,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;
};

//==============================================================
// RG G4 EDITOR
//==============================================================

class RG_G4AudioProcessorEditor
    : public juce::AudioProcessorEditor
{
public:
    explicit RG_G4AudioProcessorEditor (RG_G4AudioProcessor&);
    ~RG_G4AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:

    RG_G4AudioProcessor& processor;

    G4LookAndFeel g4LookAndFeel;

    //==========================================================
    // KNOBS
    //==========================================================

    juce::Slider bassKnob;
    juce::Slider middleKnob;
    juce::Slider trebleKnob;

    juce::Slider volumeKnob;
    juce::Slider gainKnob;

    //==========================================================
    // AGGRESSION 3-WAY MINI METAL TOGGLE
    //==========================================================

    juce::Slider aggressionSwitch;

    //==========================================================
    // ROUND METAL 3PDT FOOTSWITCH
    //==========================================================

    juce::ToggleButton footswitch;

    //==========================================================
    // LABELS
    //==========================================================

    juce::Label bassLabel;
    juce::Label middleLabel;
    juce::Label trebleLabel;

    juce::Label volumeLabel;
    juce::Label aggressionLabel;
    juce::Label gainLabel;

    juce::Label blueLabel;
    juce::Label offLabel;
    juce::Label redLabel;

    //==========================================================
    // APVTS ATTACHMENTS
    //==========================================================

    using SliderAttachment =
        juce::AudioProcessorValueTreeState::SliderAttachment;

    using ButtonAttachment =
        juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> bassAttachment;
    std::unique_ptr<SliderAttachment> middleAttachment;
    std::unique_ptr<SliderAttachment> trebleAttachment;

    std::unique_ptr<SliderAttachment> volumeAttachment;
    std::unique_ptr<SliderAttachment> gainAttachment;

    std::unique_ptr<SliderAttachment> aggressionAttachment;

    std::unique_ptr<ButtonAttachment> bypassAttachment;

    //==========================================================

    void setupKnob (
        juce::Slider& slider,
        const juce::String& parameterID);

    void setupLabel (
        juce::Label& label,
        const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (
        RG_G4AudioProcessorEditor)
};
