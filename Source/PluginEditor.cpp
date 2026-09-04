#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================
// G4 LOOK AND FEEL
//==============================================================

G4LookAndFeel::G4LookAndFeel()
{
    setColour (juce::Slider::thumbColourId,
               juce::Colours::white);

    setColour (juce::Slider::rotarySliderFillColourId,
               juce::Colours::white);

    setColour (juce::Slider::rotarySliderOutlineColourId,
               juce::Colours::black);

    setColour (juce::ToggleButton::textColourId,
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
        static_cast<float> (x),
        static_cast<float> (y),
        static_cast<float> (width),
        static_cast<float> (height));

    const float size = juce::jmin (
        bounds.getWidth(),
        bounds.getHeight());

    const float radius = size * 0.34f;
    const auto centre = bounds.getCentre();

    // Shadow
    g.setColour (
        juce::Colours::black.withAlpha (0.65f));

    g.fillEllipse (
        centre.x - radius - 3.0f,
        centre.y - radius + 5.0f,
        (radius + 3.0f) * 2.0f,
        (radius + 3.0f) * 2.0f);

    // Outer metal ring
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

    // Dark knob
    const float innerRadius = radius * 0.88f;

    g.setGradientFill (
        juce::ColourGradient (
            juce::Colour (0xff444444),
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

    // Highlight
    g.setColour (
        juce::Colours::white.withAlpha (0.10f));

    g.fillEllipse (
        centre.x - innerRadius * 0.70f,
        centre.y - innerRadius * 0.70f,
        innerRadius * 1.40f,
        innerRadius * 1.40f);

    // Pointer
    const auto angle =
        rotaryStartAngle
        + sliderPosProportional
        * (rotaryEndAngle - rotaryStartAngle);

    juce::Path pointer;

    const float pointerLength = radius * 0.70f;
    const float pointerThickness = 3.0f;

    pointer.addRoundedRectangle (
        -pointerThickness * 0.5f,
        -pointerLength,
        pointerThickness,
        pointerLength,
        1.2f);

    g.setColour (juce::Colours::white);

    g.fillPath (
        pointer,
        juce::AffineTransform::rotation (angle)
            .translated (centre.x, centre.y));

    // Center cap
    g.setColour (juce::Colour (0xff111111));

    g.fillEllipse (
        centre.x - 4.0f,
        centre.y - 4.0f,
        8.0f,
        8.0f);
}

//==============================================================
// AGGRESSION MINI METAL TOGGLE
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
        static_cast<float> (x),
        static_cast<float> (y),
        static_cast<float> (width),
        static_cast<float> (height));

    const float centreX = area.getCentreX();

    // Switch plate
    auto plate = juce::Rectangle<float> (
        centreX - 12.0f,
        area.getY() + 10.0f,
        24.0f,
        area.getHeight() - 20.0f);

    g.setColour (juce::Colour (0xff141414));

    g.fillRoundedRectangle (
        plate,
        7.0f);

    g.setColour (juce::Colour (0xff777777));

    g.drawRoundedRectangle (
        plate,
        7.0f,
        1.5f);

    const float topY =
        plate.getY() + 13.0f;

    const float midY =
        plate.getCentreY();

    const float bottomY =
        plate.getBottom() - 13.0f;

    // BLUE
    g.setColour (juce::Colour (0xff2474ff));

    g.fillEllipse (
        centreX - 3.0f,
        topY - 3.0f,
        6.0f,
        6.0f);

    // OFF
    g.setColour (juce::Colour (0xff888888));

    g.fillEllipse (
        centreX - 3.0f,
        midY - 3.0f,
        6.0f,
        6.0f);

    // RED
    g.setColour (juce::Colour (0xffe52b2b));

    g.fillEllipse (
        centreX - 3.0f,
        bottomY - 3.0f,
        6.0f,
        6.0f);

    // Metal toggle handle
    const float knobY =
        juce::jlimit (
            plate.getY() + 13.0f,
            plate.getBottom() - 13.0f,
            sliderPos);

    g.setGradientFill (
        juce::ColourGradient (
            juce::Colour (0xffffffff),
            centreX - 6.0f,
            knobY - 10.0f,
            juce::Colour (0xff555555),
            centreX + 6.0f,
            knobY + 10.0f,
            false));

    g.fillRoundedRectangle (
        centreX - 6.0f,
        knobY - 11.0f,
        12.0f,
        22.0f,
        5.0f);

    g.setColour (
        juce::Colours::black.withAlpha (0.7f));

    g.drawRoundedRectangle (
        centreX - 6.0f,
        knobY - 11.0f,
        12.0f,
        22.0f,
        5.0f,
        1.0f);
}

//==============================================================
// ROUND METAL 3PDT FOOTSWITCH
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

    const float radius =
        juce::jmin (
            bounds.getWidth(),
            bounds.getHeight()) * 0.34f;

    // Shadow
    g.setColour (
        juce::Colours::black.withAlpha (0.75f));

    g.fillEllipse (
        centre.x - radius,
        centre.y - radius + 7.0f,
        radius * 2.0f,
        radius * 2.0f);

    // Outer metal
    g.setGradientFill (
        juce::ColourGradient (
            juce::Colour (0xffffffff),
            centre.x,
            centre.y - radius,
            juce::Colour (0xff4c4c4c),
            centre.x,
            centre.y + radius,
            false));

    g.fillEllipse (
        centre.x - radius,
        centre.y - radius,
        radius * 2.0f,
        radius * 2.0f);

    // Inner face
    const float inner =
        radius * 0.78f;

    g.setGradientFill (
        juce::ColourGradient (
            button.getToggleState()
                ? juce::Colour (0xff50ff75)
                : juce::Colour (0xff444444),
            centre.x,
            centre.y - inner,
            button.getToggleState()
                ? juce::Colour (0xff08752b)
                : juce::Colour (0xff171717),
            centre.x,
            centre.y + inner,
            false));

    g.fillEllipse (
        centre.x - inner,
        centre.y - inner,
        inner * 2.0f,
        inner * 2.0f);

    // Metal ring
    g.setColour (
        juce::Colours::white.withAlpha (0.35f));

    g.drawEllipse (
        centre.x - radius,
        centre.y - radius,
        radius * 2.0f,
        radius * 2.0f,
        2.0f);

    // Press effect
    if (shouldDrawButtonAsDown)
    {
        g.setColour (
            juce::Colours::black.withAlpha (0.20f));

        g.fillEllipse (
            centre.x - inner,
            centre.y - inner,
            inner * 2.0f,
            inner * 2.0f);
    }

    // Highlight
    if (shouldDrawButtonAsHighlighted)
    {
        g.setColour (
            juce::Colours::white.withAlpha (0.25f));

        g.drawEllipse (
            centre.x - radius - 2.0f,
            centre.y - radius - 2.0f,
            radius * 2.0f + 4.0f,
            radius * 2.0f + 4.0f,
            1.5f);
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

    setupKnob (bassKnob, "BASS");
    setupKnob (middleKnob, "MID");
    setupKnob (trebleKnob, "TREBLE");

    setupKnob (volumeKnob, "VOLUME");
    setupKnob (gainKnob, "GAIN");

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

    aggressionSwitch.setValue (
        1.0,
        juce::dontSendNotification);

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

    footswitch.setClickingTogglesState (true);

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

    setupLabel (bassLabel, "BASS");
    setupLabel (middleLabel, "MIDDLE");
    setupLabel (trebleLabel, "TREBLE");

    setupLabel (volumeLabel, "VOLUME");
    setupLabel (aggressionLabel, "AGGRESSION");
    setupLabel (gainLabel, "GAIN");

    setupLabel (blueLabel, "BLUE");
    setupLabel (offLabel, "OFF");
    setupLabel (redLabel, "RED");

    //==========================================================
    // EXACT 500 x 700
    //==========================================================

    setSize (500, 700);

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

    addAndMakeVisible (slider);

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
            11.0f,
            juce::Font::bold));

    label.setColour (
        juce::Label::textColourId,
        juce::Colours::white);

    label.setJustificationType (
        juce::Justification::centred);

    addAndMakeVisible (label);
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
        area.reduced (6.0f),
        24.0f);

    // Metal border
    g.setColour (
        juce::Colour (0xffb9b9b9));

    g.drawRoundedRectangle (
        area.reduced (6.0f),
        24.0f,
        3.0f);

    // Inner border
    g.setColour (
        juce::Colours::black.withAlpha (0.55f));

    g.drawRoundedRectangle (
        area.reduced (14.0f),
        18.0f,
        1.5f);

    //==========================================================
    // TOP JACK AREA
    //==========================================================

    g.setColour (
        juce::Colours::black.withAlpha (0.32f));

    g.fillRoundedRectangle (
        28.0f,
        22.0f,
        444.0f,
        42.0f,
        10.0f);

    g.setColour (
        juce::Colours::white.withAlpha (0.80f));

    g.setFont (
        juce::Font (
            10.0f,
            juce::Font::bold));

    g.drawText (
        "OUTPUT",
        38,
        34,
        70,
        18,
        juce::Justification::centredLeft);

    g.drawText (
        "INPUT",
        392,
        34,
        70,
        18,
        juce::Justification::centredRight);

    //==========================================================
    // RG G4
    //==========================================================

    g.setColour (juce::Colours::white);

    g.setFont (
        juce::Font (
            28.0f,
            juce::Font::bold));

    g.drawText (
        "RG G4",
        0,
        430,
        500,
        38,
        juce::Justification::centred);

    //==========================================================
    // REALISTIC PLASTIC LED
    //==========================================================

    const float ledX = 250.0f;
    const float ledY = 478.0f;

    auto* bypass =
        processor.apvts.getRawParameterValue ("BYPASS");

    const bool active =
        bypass != nullptr
            ? bypass->load() < 0.5f
            : true;

    // Soft green glow
    if (active)
    {
        for (int i = 5; i >= 1; --i)
        {
            const float glowRadius =
                8.0f + static_cast<float> (i) * 5.0f;

            const float alpha =
                0.025f * static_cast<float> (6 - i);

            g.setColour (
                juce::Colour (0xff20ff55)
                    .withAlpha (alpha));

            g.fillEllipse (
                ledX - glowRadius,
                ledY - glowRadius,
                glowRadius * 2.0f,
                glowRadius * 2.0f);
        }
    }

    // Plastic / metal outer ring
    const float ringRadius = 13.0f;

    g.setGradientFill (
        juce::ColourGradient (
            juce::Colour (0xffeeeeee),
            ledX - 8.0f,
            ledY - 10.0f,
            juce::Colour (0xff555555),
            ledX + 10.0f,
            ledY + 10.0f,
            true));

    g.fillEllipse (
        ledX - ringRadius,
        ledY - ringRadius,
        ringRadius * 2.0f,
        ringRadius * 2.0f);

    // Dark inner ring
    const float innerRing = 10.5f;

    g.setColour (
        juce::Colour (0xff111111));

    g.fillEllipse (
        ledX - innerRing,
        ledY - innerRing,
        innerRing * 2.0f,
        innerRing * 2.0f);

    // Transparent green plastic lens
    const float lensRadius = 8.5f;

    g.setGradientFill (
        juce::ColourGradient (
            active
                ? juce::Colour (0xffbaffc8)
                : juce::Colour (0xff24452d),

            ledX - 3.0f,
            ledY - 6.0f,

            active
                ? juce::Colour (0xff08a83b)
                : juce::Colour (0xff08150c),

            ledX + 5.0f,
            ledY + 7.0f,

            true));

    g.fillEllipse (
        ledX - lensRadius,
        ledY - lensRadius,
        lensRadius * 2.0f,
        lensRadius * 2.0f);

    // Bright inner core
    if (active)
    {
        g.setColour (
            juce::Colour (0xff39ff69)
                .withAlpha (0.55f));

        g.fillEllipse (
            ledX - 5.0f,
            ledY - 5.0f,
            10.0f,
            10.0f);
    }

    // Glass reflection
    g.setColour (
        juce::Colours::white.withAlpha (
            active ? 0.75f : 0.18f));

    g.fillEllipse (
        ledX - 4.0f,
        ledY - 5.0f,
        3.5f,
        2.5f);

    // Lens edge
    g.setColour (
        juce::Colours::black.withAlpha (0.65f));

    g.drawEllipse (
        ledX - lensRadius,
        ledY - lensRadius,
        lensRadius * 2.0f,
        lensRadius * 2.0f,
        1.0f);

    //==========================================================
    // FOOTSWITCH STATUS TEXT
    //==========================================================

    g.setColour (
        juce::Colours::white.withAlpha (0.75f));

    g.setFont (
        juce::Font (
            9.0f,
            juce::Font::bold));

    g.drawText (
        active ? "ON" : "BYPASS",
        0,
        620,
        500,
        16,
        juce::Justification::centred);

    //==========================================================
    // RG ELECTRONICS
    //==========================================================

    g.setColour (
        juce::Colours::white.withAlpha (0.60f));

    g.setFont (
        juce::Font (8.0f));

    g.drawText (
        "RG electronics",
        0,
        677,
        500,
        12,
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
        20,
        105,
        145,
        145);

    middleKnob.setBounds (
        178,
        105,
        145,
        145);

    trebleKnob.setBounds (
        335,
        105,
        145,
        145);

    bassLabel.setBounds (
        20,
        88,
        145,
        20);

    middleLabel.setBounds (
        178,
        88,
        145,
        20);

    trebleLabel.setBounds (
        335,
        88,
        145,
        20);

    //==========================================================
    // SECOND ROW
    //==========================================================

    volumeKnob.setBounds (
        35,
        285,
        135,
        135);

    gainKnob.setBounds (
        330,
        285,
        135,
        135);

    volumeLabel.setBounds (
        35,
        268,
        135,
        20);

    gainLabel.setBounds (
        330,
        268,
        135,
        20);

    //==========================================================
    // AGGRESSION
    //==========================================================

    aggressionLabel.setBounds (
        205,
        268,
        90,
        20);

    aggressionSwitch.setBounds (
        215,
        285,
        70,
        135);

    blueLabel.setBounds (
        286,
        286,
        42,
        16);

    offLabel.setBounds (
        286,
        344,
        42,
        16);

    redLabel.setBounds (
        286,
        402,
        42,
        16);

    //==========================================================
    // ROUND METAL 3PDT FOOTSWITCH
    //==========================================================

    footswitch.setBounds (
        175,
        510,
        150,
        105);
}
