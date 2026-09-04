#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================
// RG PRECISION DRIVE - EDITOR
//==============================================================

class RG_Precision_DriveAudioProcessorEditor
    : public juce::AudioProcessorEditor,
      private juce::Timer
{
public:
    RG_Precision_DriveAudioProcessorEditor(
        RG_Precision_DriveAudioProcessor&);

    ~RG_Precision_DriveAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:

    //==========================================================
    // TIMER
    //==========================================================

    void timerCallback() override;

    //==========================================================
    // PROCESSOR
    //==========================================================

    RG_Precision_DriveAudioProcessor& audioProcessor;

    //==========================================================
    // PEDAL KNOB
    //==========================================================

    class PedalKnob : public juce::Slider
    {
    public:
        PedalKnob();

        void paint(juce::Graphics&) override;

        void mouseDown(
            const juce::MouseEvent&) override;

        void mouseDrag(
            const juce::MouseEvent&) override;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PedalKnob)
    };

    //==========================================================
    // KNOBS
    //==========================================================

    PedalKnob volumeKnob;
    PedalKnob brightKnob;
    PedalKnob attackKnob;
    PedalKnob driveKnob;
    PedalKnob gateKnob;

    //==========================================================
    // LABELS
    //==========================================================

    juce::Label volumeLabel;
    juce::Label brightLabel;
    juce::Label attackLabel;
    juce::Label driveLabel;
    juce::Label gateLabel;

    //==========================================================
    // FOOTSWITCH
    //==========================================================

    juce::ToggleButton bypassButton;

    //==========================================================
    // BLUE LED
    //==========================================================

    juce::Label led;

    //==========================================================
    // PARAMETER ATTACHMENTS
    //==========================================================

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        volumeAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        brightAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        attackAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        driveAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        gateAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::ButtonAttachment>
        bypassAttachment;

    //==========================================================
    // SETUP
    //==========================================================

    void setupKnob(
        PedalKnob& knob,
        juce::Slider::SliderStyle style);

    void setupLabel(
        juce::Label& label,
        const juce::String& text);

    void updateLED();

    //==========================================================
    // EDITOR SIZE
    //==========================================================

    static constexpr int editorWidth  = 700;
    static constexpr int editorHeight = 900;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        RG_Precision_DriveAudioProcessorEditor)
};
