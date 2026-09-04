#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================
// G4 LOOK AND FEEL
//==============================================================

G4LookAndFeel::G4LookAndFeel()
{
    setColour (
        juce::Slider::thumbColourId,
        juce::Colours::white);

    setColour (
        juce::Slider::rotarySliderFillColourId,
        juce::Colours::white);

    setColour (
        juce::Slider::rotarySliderOutlineColourId,
        juce::Colours::black);

    setColour (
        juce::ToggleButton::textColourId,
        juce::Colours::white);
}

//==============================================================
// REALISTIC METAL KNOB
//==============================================================

void G4LookAndFeel::drawRotarySlider (
    juce::Graphics& g,
    int x,
    int y,
    int width,
    int height,
    float sliderPosProportional,
    float rotaryStartAngle,
    float rotaryEndAngle,
    juce::Slider& slider)
{
    juce::ignoreUnused (slider);

    auto bounds = juce::Rectangle<float> (
        (float) x,
        (float) y,
        (float) width,
        (float) height);

    auto size = juce::jmin (
        bounds.getWidth(),
        bounds.getHeight());

    auto radius = size * 0.34f;

    auto centre = bounds.getCentre();

    //==========================================================
    // SHADOW
    //==========================================================

    g.setColour (
        juce::Colours::black.withAlpha (0.65f));

    g.fillEllipse (
        centre.x - radius - 5.0f,
        centre.y - radius + 6.0f,
        (radius + 5.0f) * 2.0f,
        (radius + 5.0f) * 2.0f);

    //==========================================================
    // OUTER METAL RING
    //==========================================================

    g.setGradientFill (
        juce::ColourGradient (
            juce::Colour (0xffeeeeee),
            centre.x,
            centre.y - radius,

            juce::Colour (0xff555555),
            centre.x,
            centre.y + radius,

            false));

    g.fillEllipse (
        centre.x - radius,
        centre.y - radius,
        radius * 2.0f,
        radius * 2.0f);

    //==========================================================
    // DARK KNOB BODY
    //==========================================================

    auto innerRadius = radius * 0.88f;

    g.setGradientFill (
        juce::ColourGradient (
            juce::Colour (0xff3b3b3b),
            centre.x - innerRadius,
            centre.y - innerRadius,

            juce::Colour (0xff080808),
            centre.x + innerRadius,
            centre.y + innerRadius,

            true));

    g.fillEllipse (
        centre.x - innerRadius,
        centre.y - innerRadius,
        innerRadius * 2.0f,
        innerRadius * 2.0f);

    //==========================================================
    // KNOB HIGHLIGHT
    //==========================================================

    g.setColour (
        juce::Colours::white.withAlpha (0.12f));

    g.fillEllipse (
        centre.x - innerRadius * 0.72f,
        centre.y - innerRadius * 0.72f,
        innerRadius * 1.44f,
        innerRadius * 1.44f);

    //==========================================================
    // POINTER
    //==========================================================

    auto angle =
        rotaryStartAngle
        + sliderPosProportional
        * (rotaryEndAngle - rotaryStartAngle);

    juce::Path pointer;

    auto pointerLength = radius * 0.72f;
    auto pointerThickness = 3.5f;

    pointer.addRoundedRectangle (
        -pointerThickness * 0.5f,
        -pointerLength,
        pointerThickness,
        pointerLength,
        1.5f);

    g.setColour (juce::Colours::white);

    g.fillPath (
        pointer,
        juce::AffineTransform::rotation (
            angle).translated (
                centre.x,
                centre.y));

    //==========================================================
    // CENTER CAP
    //==========================================================

    g.setColour (
        juce::Colour (0xff111111));

    g.fillEllipse (
        centre.x - 5.0f,
        centre.y - 5.0f,
        10.0f,
        10.0f);

    g.setColour (
        juce::Colours::white.withAlpha (0.25f));

    g.drawEllipse (
        centre.x - 5.0f,
        centre.y - 5.0f,
        10.0f,
        10.0f,
        1.0f);
}

//==============================================================
// AGGRESSION SWITCH
//==============================================================

void G4LookAndFeel::drawLinearSlider (
    juce::Graphics& g,
    int x,
    int y,
    int width,
    int height,
    float sliderPos,
    float minSliderPos,
    float maxSliderPos,
    juce::Slider::SliderStyle style,
    juce::Slider& slider)
{
    juce::ignoreUnused (
        minSliderPos,
        maxSliderPos,
        style,
        slider);

    auto area = juce::Rectangle<float> (
        (float) x,
        (float) y,
        (float) width,
        (float) height);

    auto centreX = area.getCentreX();

    //==========================================================
    // SWITCH PLATE
    //==========================================================

    auto plate =
        juce::Rectangle<float> (
            centreX - 24.0f,
            area.getY() + 18.0f,
            48.0f,
            area.getHeight() - 36.0f);

    g.setColour (
        juce::Colour (0xff171717));

    g.fillRoundedRectangle (
        plate,
        10.0f);

    g.setColour (
        juce::Colour (0xff777777));

    g.drawRoundedRectangle (
        plate,
        10.0f,
        2.0f);

    //==========================================================
    // THREE POSITIONS
    //==========================================================

    float topY =
        plate.getY() + 18.0f;

    float midY =
        plate.getCentreY();

    float bottomY =
        plate.getBottom() - 18.0f;

    // position indicators
    g.setColour (
        juce::Colour (0xff2474ff));

    g.fillEllipse (
        centreX - 5.0f,
        topY - 5.0f,
        10.0f,
        10.0f);

    g.setColour (
        juce::Colour (0xff777777));

    g.fillEllipse (
        centreX - 5.0f,
        midY - 5.0f,
        10.0f,
        10.0f);

    g.setColour (
        juce::Colour (0xffe52b2b));

    g.fillEllipse (
        centreX - 5.0f,
        bottomY - 5.0f,
        10.0f,
        10.0f);

    //==========================================================
    // METAL TOGGLE
    //==========================================================

    auto knobY = sliderPos;

    g.setGradientFill (
        juce::ColourGradient (
            juce::Colour (0xffffffff),
            centreX - 8.0f,
            knobY - 10.0f,

            juce::Colour (0xff555555),
            centreX + 8.0f,
            knobY + 10.0f,

            false));

    g.fillRoundedRectangle (
        centreX - 9.0f,
        knobY - 15.0f,
        18.0f,
        30.0f,
        7.0f);

    g.setColour (
        juce::Colours::black.withAlpha (0.7f));

    g.drawRoundedRectangle (
        centreX - 9.0f,
        knobY - 15.0f,
        18.0f,
        30.0f,
        7.0f,
        1.0f);
}

//==============================================================
// FOOTSWITCH
//==============================================================

void G4LookAndFeel::drawToggleButton (
    juce::Graphics& g,
    juce::ToggleButton& button,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    auto bounds =
        button.getLocalBounds().toFloat();

    auto centre =
        bounds.getCentre();

    auto radius =
        juce::jmin (
            bounds.getWidth(),
            bounds.getHeight()) * 0.28f;

    //==========================================================
    // SHADOW
    //==========================================================

    g.setColour (
        juce::Colours::black.withAlpha (0.7f));

    g.fillEllipse (
        centre.x - radius,
        centre.y - radius + 7.0f,
        radius * 2.0f,
        radius * 2.0f);

    //==========================================================
    // METAL FOOTSWITCH
    //==========================================================

    g.setGradientFill (
        juce::ColourGradient (
            juce::Colour (0xffffffff),
            centre.x,
            centre.y - radius,

            juce::Colour (0xff555555),
            centre.x,
            centre.y + radius,

            false));

    g.fillEllipse (
        centre.x - radius,
        centre.y - radius,
        radius * 2.0f,
        radius * 2.0f);

    //==========================================================
    // INNER FACE
    //==========================================================

    auto inner =
        radius * 0.78f;

    g.setColour (
        button.getToggleState()
            ? juce::Colour (0xff20d060)
            : juce::Colour (0xff303030));

    g.fillEllipse (
        centre.x - inner,
        centre.y - inner,
        inner * 2.0f,
        inner * 2.0f);

    //==========================================================
    // HIGHLIGHT
    //==========================================================

    g.setColour (
        juce::Colours::white.withAlpha (0.25f));

    g.drawEllipse (
        centre.x - radius,
        centre.y - radius,
        radius * 2.0f,
        radius * 2.0f,
        2.0f);

    //==========================================================
    // DOWN EFFECT
    //==========================================================

    if (shouldDrawButtonAsDown)
    {
        g.setColour (
            juce::Colours::black.withAlpha (0.25f));

        g.fillEllipse (
            centre.x - inner,
            centre.y - inner,
            inner * 2.0f,
            inner * 2.0f);
    }

    if (shouldDrawButtonAsHighlighted)
    {
        g.setColour (
            juce::Colours::white.withAlpha (0.25f));

        g.drawEllipse (
            centre.x - radius - 3.0f,
            centre.y - radius - 3.0f,
            radius * 2.0f + 6.0f,
            radius * 2.0f + 6.0f,
            2.0f);
    }
}

//==============================================================
// EDITOR CONSTRUCTOR
//==============================================================

RG_G4AudioProcessorEditor::RG_G4AudioProcessorEditor (
    RG_G4AudioProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p)
{
    setLookAndFeel (&g4LookAndFeel);

    //==========================================================
    // KNOBS
    //==========================================================

    setupKnob (
        bassKnob,
        "BASS");

    setupKnob (
        middleKnob,
        "MID");

    setupKnob (
        trebleKnob,
        "TREBLE");

    setupKnob (
        volumeKnob,
        "VOLUME");

    setupKnob (
        gainKnob,
        "GAIN");

    //==========================================================
    // AGGRESSION
    //==========================================================

    aggressionSwitch.setSliderStyle (
        juce::Slider::LinearVertical);

    aggressionSwitch.setTextBoxStyle (
        juce::Slider::NoTextBox,
        false,
        0,
        0);

    aggressionSwitch.setRange (
        0.0,
        2.0,
        1.0);

    aggressionSwitch.setDoubleClickReturnValue (
        true,
        1.0);

    addAndMakeVisible (
        aggressionSwitch);

    aggressionAttachment =
        std::make_unique<SliderAttachment> (
            processor.apvts,
            "AGGRESSION",
            aggressionSwitch);

    //==========================================================
    // FOOTSWITCH
    //==========================================================

    footswitch.setButtonText ("");

    footswitch.setClickingTogglesState (
        true);

    addAndMakeVisible (
        footswitch);

    bypassAttachment =
        std::make_unique<ButtonAttachment> (
            processor.apvts,
            "BYPASS",
            footswitch);

    //==========================================================
    // LABELS
    //==========================================================

    setupLabel (
        bassLabel,
        "BASS");

    setupLabel (
        middleLabel,
        "MIDDLE");

    setupLabel (
        trebleLabel,
        "TREBLE");

    setupLabel (
        volumeLabel,
        "VOLUME");

    setupLabel (
        aggressionLabel,
        "AGGRESSION");

    setupLabel (
        gainLabel,
        "GAIN");

    setupLabel (
        blueLabel,
        "BLUE");

    setupLabel (
        offLabel,
        "OFF");

    setupLabel (
        redLabel,
        "RED");

    //==========================================================

    setSize (
        640,
        1088);

    setResizable (
        false,
        false);
}

//==============================================================
// DESTRUCTOR
//==============================================================

RG_G4AudioProcessorEditor::~RG_G4AudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================
// SETUP KNOB
//==============================================================

void RG_G4AudioProcessorEditor::setupKnob (
    juce::Slider& slider,
    const juce::String& parameterID)
{
    slider.setSliderStyle (
        juce::Slider::RotaryHorizontalVerticalDrag);

    slider.setTextBoxStyle (
        juce::Slider::NoTextBox,
        false,
        0,
        0);

    slider.setRange (
        0.0,
        1.0,
        0.001);

    slider.setDoubleClickReturnValue (
        true,
        0.5);

    addAndMakeVisible (
        slider);

    auto attachment =
        std::make_unique<SliderAttachment> (
            processor.apvts,
            parameterID,
            slider);

    if (parameterID == "BASS")
        bassAttachment = std::move (attachment);

    else if (parameterID == "MID")
        middleAttachment = std::move (attachment);

    else if (parameterID == "TREBLE")
        trebleAttachment = std::move (attachment);

    else if (parameterID == "VOLUME")
        volumeAttachment = std::move (attachment);

    else if (parameterID == "GAIN")
        gainAttachment = std::move (attachment);
}

//==============================================================
// SETUP LABEL
//==============================================================

void RG_G4AudioProcessorEditor::setupLabel (
    juce::Label& label,
    const juce::String& text)
{
    label.setText (
        text,
        juce::dontSendNotification);

    label.setFont (
        juce::Font (
            15.0f,
            juce::Font::bold));

    label.setColour (
        juce::Label::textColourId,
        juce::Colours::white);

    label.setJustificationType (
        juce::Justification::centred);

    addAndMakeVisible (
        label);
}

//==============================================================
// PAINT
//==============================================================

void RG_G4AudioProcessorEditor::paint (
    juce::Graphics& g)
{
    auto area =
        getLocalBounds().toFloat();

    //==========================================================
    // PEDAL BODY
    //==========================================================

    g.setGradientFill (
        juce::ColourGradient (
            juce::Colour (0xff8b1717),
            area.getTopLeft(),

            juce::Colour (0xff3e0808),
            area.getBottomRight(),

            false));

    g.fillRoundedRectangle (
        area.reduced (8.0f),
        28.0f);

    //==========================================================
    // METAL EDGE
    //==========================================================

    g.setColour (
        juce::Colour (0xffb9b9b9));

    g.drawRoundedRectangle (
        area.reduced (8.0f),
        28.0f,
        3.0f);

    //==========================================================
    // INNER BORDER
    //==========================================================

    g.setColour (
        juce::Colours::black.withAlpha (0.55f));

    g.drawRoundedRectangle (
        area.reduced (20.0f),
        20.0f,
        2.0f);

    //==========================================================
    // TOP JACK AREA
    //==========================================================

    g.setColour (
        juce::Colours::black.withAlpha (0.35f));

    g.fillRoundedRectangle (
        38.0f,
        32.0f,
        564.0f,
        60.0f,
        12.0f);

    g.setColour (
        juce::Colours::white.withAlpha (0.8f));

    g.setFont (
        juce::Font (
            13.0f,
            juce::Font::bold));

    g.drawText (
        "OUTPUT",
        52,
        52,
        90,
        20,
        juce::Justification::centredLeft);

    g.drawText (
        "INPUT",
        498,
        52,
        90,
        20,
        juce::Justification::centredRight);

    //==========================================================
    // BRANDING
    //==========================================================

    g.setColour (
        juce::Colours::white);

    g.setFont (
        juce::Font (
            34.0f,
            juce::Font::bold));

    g.drawText (
        "RG",
        0,
        640,
        640,
        42,
        juce::Justification::centred);

    g.setFont (
        juce::Font (
            19.0f,
            juce::Font::bold));

    g.drawText (
        "G4",
        0,
        683,
        640,
        30,
        juce::Justification::centred);

    g.setColour (
        juce::Colours::white.withAlpha (0.65f));

    g.setFont (
        juce::Font (
            11.0f));

    g.drawText (
        "HIGH GAIN DISTORTION",
        0,
        716,
        640,
        20,
        juce::Justification::centred);

    //==========================================================
    // STATUS LED
    //==========================================================

    auto ledX = 320.0f;
    auto ledY = 770.0f;

    bool active =
        ! processor.apvts
            .getRawParameterValue (
                "BYPASS")
            ->load();

    if (active)
    {
        g.setColour (
            juce::Colour (0xff20ff55)
                .withAlpha (0.18f));

        g.fillEllipse (
            ledX - 20.0f,
            ledY - 20.0f,
            40.0f,
            40.0f);

        g.setColour (
            juce::Colour (0xff20ff55));

        g.fillEllipse (
            ledX - 7.0f,
            ledY - 7.0f,
            14.0f,
            14.0f);
    }
    else
    {
        g.setColour (
            juce::Colour (0xff151515));

        g.fillEllipse (
            ledX - 7.0f,
            ledY - 7.0f,
            14.0f,
            14.0f);
    }

    //==========================================================
    // FOOTSWITCH TEXT
    //==========================================================

    g.setColour (
        juce::Colours::white);

    g.setFont (
        juce::Font (
            13.0f,
            juce::Font::bold));

    g.drawText (
        active ? "ON" : "BYPASS",
        0,
        965,
        640,
        22,
        juce::Justification::centred);
}

//==============================================================
// RESIZED
//==============================================================

void RG_G4AudioProcessorEditor::resized()
{
    //==========================================================
    // TOP ROW
    //==========================================================

    bassKnob.setBounds (
        55,
        135,
        150,
        150);

    middleKnob.setBounds (
        245,
        135,
        150,
        150);

    trebleKnob.setBounds (
        435,
        135,
        150,
        150);

    //==========================================================
    // TOP LABELS
    //==========================================================

    bassLabel.setBounds (
        55,
        105,
        150,
        25);

    middleLabel.setBounds (
        245,
        105,
        150,
        25);

    trebleLabel.setBounds (
        435,
        105,
        150,
        25);

    //==========================================================
    // SECOND ROW
    //==========================================================

    volumeKnob.setBounds (
        55,
        355,
        150,
        150);

    gainKnob.setBounds (
        435,
        355,
        150,
        150);

    //==========================================================
    // SECOND LABELS
    //==========================================================

    volumeLabel.setBounds (
        55,
        325,
        150,
        25);

    gainLabel.setBounds (
        435,
        325,
        150,
        25);

    //==========================================================
    // AGGRESSION SWITCH
    //==========================================================

    aggressionSwitch.setBounds (
        270,
        335,
        100,
        210);

    aggressionLabel.setBounds (
        230,
        305,
        180,
        25);

    //==========================================================
    // AGGRESSION TEXT
    //==========================================================

    blueLabel.setBounds (
        370,
        345,
        75,
        25);

    offLabel.setBounds (
        370,
        420,
        75,
        25);

    redLabel.setBounds (
        370,
        495,
        75,
        25);

    //==========================================================
    // FOOTSWITCH
    //==========================================================

    footswitch.setBounds (
        190,
        815,
        260,
        145);
}
