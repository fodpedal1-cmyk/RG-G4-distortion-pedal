#include "PluginEditor.h"

//==============================================================
// PEDAL KNOB
//==============================================================

RG_Precision_DriveAudioProcessorEditor::PedalKnob::PedalKnob()
{
    setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    setTextBoxStyle(
        juce::Slider::NoTextBox,
        false,
        0,
        0);

    setRange(0.0, 1.0, 0.001);
    setPopupDisplayEnabled(true, true, nullptr);
    setMouseDragSensitivity(180);
}

void RG_Precision_DriveAudioProcessorEditor::PedalKnob::paint(
    juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    const float size =
        juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.72f;

    const auto knobBounds =
        juce::Rectangle<float>(
            bounds.getCentreX() - size * 0.5f,
            bounds.getCentreY() - size * 0.5f,
            size,
            size);

    // Outer metal ring
    g.setColour(juce::Colours::black);
    g.fillEllipse(knobBounds);

    g.setColour(juce::Colour(0xff505050));
    g.drawEllipse(knobBounds.reduced(1.5f), 2.0f);

    // Knob body
    auto body = knobBounds.reduced(size * 0.075f);

    g.setColour(juce::Colour(0xff202020));
    g.fillEllipse(body);

    g.setColour(juce::Colour(0xff686868));
    g.drawEllipse(body, 1.5f);

    // Pointer
    const double value =
        juce::jlimit(
            0.0,
            1.0,
            static_cast<double>(getValue()));

    const double startAngle =
        juce::MathConstants<double>::pi * 1.25;

    const double endAngle =
        juce::MathConstants<double>::pi * 2.75;

    const double angle =
        startAngle + (endAngle - startAngle) * value;

    const auto centre = body.getCentre();

    const float pointerLength = size * 0.30f;

    juce::Point<float> pointerEnd(
        centre.x + std::cos(angle) * pointerLength,
        centre.y + std::sin(angle) * pointerLength);

    g.setColour(juce::Colours::white);
    g.drawLine(
        centre.x,
        centre.y,
        pointerEnd.x,
        pointerEnd.y,
        3.0f);

    g.fillEllipse(
        centre.x - 3.0f,
        centre.y - 3.0f,
        6.0f,
        6.0f);
}

void RG_Precision_DriveAudioProcessorEditor::PedalKnob::mouseDown(
    const juce::MouseEvent& e)
{
    juce::Slider::mouseDown(e);
}

void RG_Precision_DriveAudioProcessorEditor::PedalKnob::mouseDrag(
    const juce::MouseEvent& e)
{
    juce::Slider::mouseDrag(e);
}

//==============================================================
// CONSTRUCTOR
//==============================================================

RG_Precision_DriveAudioProcessorEditor::
    RG_Precision_DriveAudioProcessorEditor(
        RG_Precision_DriveAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p)
{
    //==========================================================
    // KNOBS
    //==========================================================

    setupKnob(
        volumeKnob,
        juce::Slider::RotaryHorizontalVerticalDrag);

    setupKnob(
        brightKnob,
        juce::Slider::RotaryHorizontalVerticalDrag);

    setupKnob(
        attackKnob,
        juce::Slider::RotaryHorizontalVerticalDrag);

    setupKnob(
        driveKnob,
        juce::Slider::RotaryHorizontalVerticalDrag);

    setupKnob(
        gateKnob,
        juce::Slider::RotaryHorizontalVerticalDrag);

    //==========================================================
    // ATTACK = 6 POSITIONS
    //==========================================================

    attackKnob.setRange(1.0, 6.0, 1.0);
    attackKnob.setNumDecimalPlacesToDisplay(0);

    //==========================================================
    // LABELS
    //==========================================================

    setupLabel(volumeLabel, "VOL");
    setupLabel(brightLabel, "BRIGHT");
    setupLabel(attackLabel, "ATTACK");
    setupLabel(driveLabel, "DRIVE");
    setupLabel(gateLabel, "GATE");

    //==========================================================
    // FOOTSWITCH
    //==========================================================

    bypassButton.setButtonText("");

    bypassButton.setClickingTogglesState(true);

    bypassButton.setColour(
        juce::ToggleButton::tickColourId,
        juce::Colours::transparentBlack);

    addAndMakeVisible(bypassButton);

    //==========================================================
    // BLUE LED
    //==========================================================

    led.setText(
        "",
        juce::dontSendNotification);

    led.setColour(
        juce::Label::backgroundColourId,
        juce::Colour(0xff202020));

    addAndMakeVisible(led);

    //==========================================================
    // PARAMETER ATTACHMENTS
    //==========================================================

    volumeAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.parameters,
                "VOLUME",
                volumeKnob);

    brightAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.parameters,
                "BRIGHT",
                brightKnob);

    attackAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.parameters,
                "ATTACK",
                attackKnob);

    driveAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.parameters,
                "DRIVE",
                driveKnob);

    gateAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.parameters,
                "GATE",
                gateKnob);

    bypassAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::ButtonAttachment>(
                audioProcessor.parameters,
                "BYPASS",
                bypassButton);

    //==========================================================
    // EDITOR SIZE
    //==========================================================

    setSize(700, 900);

    startTimerHz(30);
}

//==============================================================
// DESTRUCTOR
//==============================================================

RG_Precision_DriveAudioProcessorEditor::
    ~RG_Precision_DriveAudioProcessorEditor()
{
}

//==============================================================
// SETUP KNOB
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::setupKnob(
    PedalKnob& knob,
    juce::Slider::SliderStyle style)
{
    knob.setSliderStyle(style);

    knob.setTextBoxStyle(
        juce::Slider::NoTextBox,
        false,
        0,
        0);

    knob.setRange(0.0, 1.0, 0.001);

    addAndMakeVisible(knob);
}

//==============================================================
// SETUP LABEL
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::setupLabel(
    juce::Label& label,
    const juce::String& text)
{
    label.setText(
        text,
        juce::dontSendNotification);

    label.setFont(
        juce::Font(
            20.0f,
            juce::Font::bold));

    label.setColour(
        juce::Label::textColourId,
        juce::Colours::white);

    label.setJustificationType(
        juce::Justification::centred);

    addAndMakeVisible(label);
}

//==============================================================
// LED
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::updateLED()
{
    const bool bypassed =
        audioProcessor.parameters
            .getRawParameterValue("BYPASS")
            ->load() > 0.5f;

    if (bypassed)
    {
        led.setColour(
            juce::Label::backgroundColourId,
            juce::Colour(0xff101010));
    }
    else
    {
        led.setColour(
            juce::Label::backgroundColourId,
            juce::Colour(0xff0088ff));
    }
}

//==============================================================
// PAINT
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::paint(
    juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();

    //==========================================================
    // PEDAL BODY
    //==========================================================

    g.setColour(juce::Colour(0xff202020));
    g.fillRoundedRectangle(
        area.reduced(12.0f),
        18.0f);

    // Metal border
    g.setColour(juce::Colour(0xff707070));
    g.drawRoundedRectangle(
        area.reduced(12.0f),
        18.0f,
        3.0f);

    // Inner border
    g.setColour(juce::Colour(0xff303030));
    g.drawRoundedRectangle(
        area.reduced(22.0f),
        14.0f,
        1.0f);

    //==========================================================
    // BRAND
    //==========================================================

    g.setColour(juce::Colours::white);

    g.setFont(
        juce::Font(
            27.0f,
            juce::Font::bold));

    g.drawFittedText(
        "RG PRECISION DRIVE",
        0,
        510,
        getWidth(),
        45,
        juce::Justification::centred,
        1);

    //==========================================================
    // FOOTSWITCH METAL PLATE
    //==========================================================

    g.setColour(juce::Colour(0xff303030));

    g.fillRoundedRectangle(
        255.0f,
        650.0f,
        190.0f,
        105.0f,
        15.0f);

    g.setColour(juce::Colour(0xff707070));

    g.drawRoundedRectangle(
        255.0f,
        650.0f,
        190.0f,
        105.0f,
        15.0f,
        2.0f);

    //==========================================================
    // FOOTSWITCH
    //==========================================================

    g.setColour(juce::Colour(0xffaaaaaa));

    g.fillRoundedRectangle(
        292.0f,
        675.0f,
        116.0f,
        55.0f,
        27.0f);

    g.setColour(juce::Colour(0xff303030));

    g.drawRoundedRectangle(
        292.0f,
        675.0f,
        116.0f,
        55.0f,
        27.0f,
        3.0f);

    //==========================================================
    // BOTTOM BRAND
    //==========================================================

    g.setColour(juce::Colour(0xffaaaaaa));

    g.setFont(
        juce::Font(
            15.0f,
            juce::Font::bold));

    g.drawFittedText(
        "RG ELECTRONICS",
        0,
        805,
        getWidth(),
        25,
        juce::Justification::centred,
        1);
}

//==============================================================
// RESIZED
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::resized()
{
    //==========================================================
    // TOP ROW
    //==========================================================

    volumeLabel.setBounds(
        70, 65, 180, 30);

    volumeKnob.setBounds(
        60, 90, 200, 170);

    brightLabel.setBounds(
        450, 65, 180, 30);

    brightKnob.setBounds(
        440, 90, 200, 170);

    //==========================================================
    // SECOND ROW
    //==========================================================

    attackLabel.setBounds(
        70, 275, 180, 30);

    attackKnob.setBounds(
        60, 300, 200, 170);

    driveLabel.setBounds(
        450, 275, 180, 30);

    driveKnob.setBounds(
        440, 300, 200, 170);

    //==========================================================
    // GATE
    //==========================================================

    gateLabel.setBounds(
        260, 425, 180, 30);

    gateKnob.setBounds(
        250, 450, 200, 170);

    //==========================================================
    // BLUE LED
    //==========================================================

    led.setBounds(
        332, 590, 36, 36);

    //==========================================================
    // FOOTSWITCH BUTTON
    // Transparent clickable area over metal switch
    //==========================================================

    bypassButton.setBounds(
        275, 660, 150, 90);
}

//==============================================================
// TIMER
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::timerCallback()
{
    updateLED();
}
