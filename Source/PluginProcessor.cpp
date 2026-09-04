#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

//==============================================================================
// Constructor
//==============================================================================

RG_G4AudioProcessor::RG_G4AudioProcessor()

#ifndef JucePlugin_PreferredChannelConfigurations

    : AudioProcessor (
        BusesProperties()
            .withInput (
                "Input",
                juce::AudioChannelSet::stereo(),
                true)

            .withOutput (
                "Output",
                juce::AudioChannelSet::stereo(),
                true)),

#endif

      apvts (
          *this,
          nullptr,
          "RG_G4_STATE",
          createParameterLayout())
{
    gainParameter =
        apvts.getRawParameterValue ("GAIN");

    bassParameter =
        apvts.getRawParameterValue ("BASS");

    midParameter =
        apvts.getRawParameterValue ("MID");

    trebleParameter =
        apvts.getRawParameterValue ("TREBLE");

    volumeParameter =
        apvts.getRawParameterValue ("VOLUME");

    aggressionParameter =
        apvts.getRawParameterValue ("AGGRESSION");

    bypassParameter =
        apvts.getRawParameterValue ("BYPASS");
}

//==============================================================================
// PARAMETERS
//==============================================================================

juce::AudioProcessorValueTreeState::ParameterLayout
RG_G4AudioProcessor::createParameterLayout()
{
    using Range = juce::NormalisableRange<float>;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    //==========================================================================
    // GAIN
    //==========================================================================

    layout.add (
        std::make_unique<juce::AudioParameterFloat>(
            "GAIN",
            "Gain",
            Range (0.0f, 1.0f, 0.001f),
            0.55f));

    //==========================================================================
    // BASS
    //==========================================================================

    layout.add (
        std::make_unique<juce::AudioParameterFloat>(
            "BASS",
            "Bass",
            Range (0.0f, 1.0f, 0.001f),
            0.50f));

    //==========================================================================
    // MID
    //==========================================================================

    layout.add (
        std::make_unique<juce::AudioParameterFloat>(
            "MID",
            "Mid",
            Range (0.0f, 1.0f, 0.001f),
            0.50f));

    //==========================================================================
    // TREBLE
    //==========================================================================

    layout.add (
        std::make_unique<juce::AudioParameterFloat>(
            "TREBLE",
            "Treble",
            Range (0.0f, 1.0f, 0.001f),
            0.50f));

    //==========================================================================
    // VOLUME
    //==========================================================================

    layout.add (
        std::make_unique<juce::AudioParameterFloat>(
            "VOLUME",
            "Volume",
            Range (0.0f, 1.0f, 0.001f),
            0.70f));

    //==========================================================================
    // AGGRESSION
    //
    // 0 = OFF
    // 1 = BLUE
    // 2 = RED
    //==========================================================================

    layout.add (
        std::make_unique<juce::AudioParameterInt>(
            "AGGRESSION",
            "Aggression",
            0,
            2,
            1));

    //==========================================================================
    // BYPASS
    //==========================================================================

    layout.add (
        std::make_unique<juce::AudioParameterBool>(
            "BYPASS",
            "Bypass",
            false));

    return layout;
}

//==============================================================================
// PREPARE
//==============================================================================

void RG_G4AudioProcessor::prepareToPlay (
    double newSampleRate,
    int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);

    sampleRate = newSampleRate;

    resetDSP();
}

//==============================================================================
// RELEASE
//==============================================================================

void RG_G4AudioProcessor::releaseResources()
{
}

//==============================================================================
// BUS LAYOUT
//==============================================================================

bool RG_G4AudioProcessor::isBusesLayoutSupported (
    const BusesLayout& layouts) const
{
    const auto input =
        layouts.getMainInputChannelSet();

    const auto output =
        layouts.getMainOutputChannelSet();

    if (output != juce::AudioChannelSet::mono()
        && output != juce::AudioChannelSet::stereo())
    {
        return false;
    }

    return input == output;
}

//==============================================================================
// TL072 RESET
//==============================================================================

void RG_G4AudioProcessor::TL072::reset()
{
    output = 0.0f;
}

//==============================================================================
// TL072 PROCESS
//==============================================================================

float RG_G4AudioProcessor::TL072::process (
    float target,
    float fs,
    float slewMultiplier)
{
    // 16 V/us
    const float slewRate =
        SLEW_V_PER_US *
        1.0e6f *
        slewMultiplier;

    const float maximumStep =
        slewRate / fs;

    const float difference =
        target - output;

    const float limited =
        juce::jlimit (
            -maximumStep,
            maximumStep,
            difference);

    output += limited;

    // Practical op-amp output rail.
    //
    // Smooth saturation instead of a hard digital clip.
    const float railScale =
        RAIL * 0.55f;

    output =
        std::tanh (
            output / railScale)
        * railScale;

    return output;
}

//==============================================================================
// FILTER RESET
//==============================================================================

void RG_G4AudioProcessor::Filter::reset()
{
    state = 0.0f;
}

//==============================================================================
// LOWPASS
//==============================================================================

float RG_G4AudioProcessor::Filter::lowpass (
    float input,
    float cutoff,
    float fs)
{
    const float safeCutoff =
        juce::jlimit (
            5.0f,
            fs * 0.45f,
            cutoff);

    const float coefficient =
        std::exp (
            -2.0f *
            juce::MathConstants<float>::pi *
            safeCutoff /
            fs);

    state =
        (1.0f - coefficient) * input
        + coefficient * state;

    return state;
}

//==============================================================================
// HIGHPASS
//==============================================================================

float RG_G4AudioProcessor::Filter::highpass (
    float input,
    float cutoff,
    float fs)
{
    const float low =
        lowpass (
            input,
            cutoff,
            fs);

    return input - low;
}

//==============================================================================
// OP AMP STAGE
//==============================================================================

float RG_G4AudioProcessor::opAmpStage (
    TL072& opAmp,
    float input,
    float gain,
    float slewMultiplier)
{
    const float target =
        input * gain;

    return opAmp.process (
        target,
        static_cast<float> (sampleRate),
        slewMultiplier);
}

//==============================================================================
// LED CLIPPING
//==============================================================================

float RG_G4AudioProcessor::ledClip (
    float input,
    float drive,
    int aggression)
{
    float threshold = 0.65f;

    // Aggression changes clipping threshold.
    if (aggression == 1)
        threshold = 0.57f;

    else if (aggression == 2)
        threshold = 0.49f;

    const float x =
        input * drive;

    // Slightly asymmetric LED behaviour.
    const float positive =
        std::tanh (
            juce::jmax (
                0.0f,
                x - threshold)
            * 2.5f);

    const float negative =
        std::tanh (
            juce::jmax (
                0.0f,
                -x - threshold)
            * 2.2f);

    float result = x;

    if (x > threshold)
    {
        result =
            threshold
            + positive *
              (1.0f - threshold);
    }
    else if (x < -threshold)
    {
        result =
            -threshold
            - negative *
              (1.0f - threshold);
    }

    return result;
}

//==============================================================================
// AGGRESSION
//==============================================================================

float RG_G4AudioProcessor::aggressionStage (
    float input,
    int aggression)
{
    if (aggression == 0)
        return input;

    //==========================================================================
    // BLUE
    //==========================================================================
    if (aggression == 1)
    {
        const float tight =
            stage2HP.highpass (
                input,
                75.0f,
                static_cast<float> (sampleRate));

        const float body =
            input * 0.15f;

        return
            tight * 0.85f
            + body;
    }

    //==========================================================================
    // RED
    //==========================================================================

    const float tight =
        stage2HP.highpass (
            input,
            115.0f,
            static_cast<float> (sampleRate));

    const float top =
        input -
        stage2LP.lowpass (
            input,
            5200.0f,
            static_cast<float> (sampleRate));

    return
        tight * 0.88f
        + top * 0.12f;
}

//==============================================================================
// TONE STACK
//==============================================================================

float RG_G4AudioProcessor::toneStack (
    float input,
    float bass,
    float mid,
    float treble)
{
    const float fs =
        static_cast<float> (sampleRate);

    //==========================================================================
    // LOW
    //==========================================================================

    const float low =
        bassLP.lowpass (
            input,
            180.0f,
            fs);

    //==========================================================================
    // MID
    //==========================================================================

    const float midLow =
        midLP.lowpass (
            input,
            900.0f,
            fs);

    const float middle =
        midLow - low;

    //==========================================================================
    // HIGH
    //==========================================================================

    const float high =
        input -
        trebleLP.lowpass (
            input,
            3500.0f,
            fs);

    //==========================================================================
    // POT CURVES
    //==========================================================================

    const float bassGain =
        juce::jmap (
            bass,
            0.0f,
            1.0f,
            0.30f,
            1.70f);

    const float midGain =
        juce::jmap (
            mid,
            0.0f,
            1.0f,
            0.25f,
            1.55f);

    const float trebleGain =
        juce::jmap (
            treble,
            0.0f,
            1.0f,
            0.30f,
            1.70f);

    //==========================================================================
    // PASSIVE-LIKE MIX
    //==========================================================================

    const float dry =
        input * 0.12f;

    return
        dry
        + low * bassGain * 0.50f
        + middle * midGain * 0.72f
        + high * trebleGain * 0.52f;
}

//==============================================================================
// RESET DSP
//==============================================================================

void RG_G4AudioProcessor::resetDSP()
{
    U1A.reset();
    U1B.reset();

    U2A.reset();
    U2B.reset();

    U3A.reset();
    U3B.reset();

    inputHP.reset();
    inputLP.reset();

    stage1HP.reset();
    stage1LP.reset();

    stage2HP.reset();
    stage2LP.reset();

    bassLP.reset();
    midLP.reset();
    trebleLP.reset();

    outputHP.reset();
}

//==============================================================================
// PROCESS BLOCK
//==============================================================================

void RG_G4AudioProcessor::processBlock (
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;

    const int channels =
        buffer.getNumChannels();

    const int samples =
        buffer.getNumSamples();

    //==========================================================================
    // BYPASS
    //==========================================================================

    if (bypassParameter != nullptr
        && bypassParameter->load() > 0.5f)
    {
        return;
    }

    //==========================================================================
    // PARAMETERS
    //==========================================================================

    const float gain =
        gainParameter != nullptr
            ? gainParameter->load()
            : 0.55f;

    const float bass =
        bassParameter != nullptr
            ? bassParameter->load()
            : 0.50f;

    const float mid =
        midParameter != nullptr
            ? midParameter->load()
            : 0.50f;

    const float treble =
        trebleParameter != nullptr
            ? trebleParameter->load()
            : 0.50f;

    const float volume =
        volumeParameter != nullptr
            ? volumeParameter->load()
            : 0.70f;

    const int aggression =
        aggressionParameter != nullptr
            ? static_cast<int> (
                aggressionParameter->load())
            : 1;

    //==========================================================================
    // GAIN POT
    //==========================================================================
    //
    // 1M reference pot.
    //
    // We map it into a practical high-gain range.
    //==========================================================================

    const float gainAmount =
        juce::jmap (
            gain,
            0.0f,
            1.0f,
            1.0f,
            22.0f);

    //==========================================================================
    // OUTPUT LEVEL
    //==========================================================================

    const float outputLevel =
        juce::jmap (
            volume,
            0.0f,
            1.0f,
            0.0f,
            1.35f);

    //==========================================================================
    // PROCESS CHANNELS
    //==========================================================================

    for (int channel = 0;
         channel < channels;
         ++channel)
    {
        float* data =
            buffer.getWritePointer (channel);

        for (int sample = 0;
             sample < samples;
             ++sample)
        {
            float x =
                data[sample];

            //==============================================================
            // INPUT
            //==============================================================

            x =
                inputHP.highpass (
                    x,
                    18.0f,
                    static_cast<float> (sampleRate));

            // C1 / R1 style input coupling.
            x *= 1.10f;

            //==============================================================
            // U1A
            // INPUT PREAMP
            //==============================================================

            x =
                opAmpStage (
                    U1A,
                    x,
                    1.15f,
                    1.0f);

            //==============================================================
            // INPUT LOW PASS
            //==============================================================

            x =
                inputLP.lowpass (
                    x,
                    18000.0f,
                    static_cast<float> (sampleRate));

            //==============================================================
            // U1B
            // MAIN GAIN
            //==============================================================

            const float mainGain =
                1.0f
                + gainAmount
                * 0.90f;

            x =
                opAmpStage (
                    U1B,
                    x,
                    mainGain,
                    1.0f);

            //==============================================================
            // STAGE 1 RC
            //==============================================================

            x =
                stage1HP.highpass (
                    x,
                    35.0f,
                    static_cast<float> (sampleRate));

            x =
                stage1LP.lowpass (
                    x,
                    15000.0f,
                    static_cast<float> (sampleRate));

            //==============================================================
            // LED CLIPPING
            //==============================================================

            x =
                ledClip (
                    x,
                    1.0f + gain * 1.25f,
                    aggression);

            //==============================================================
            // U2A
            // POST CLIP GAIN
            //==============================================================

            x =
                opAmpStage (
                    U2A,
                    x,
                    1.20f,
                    0.90f);

            //==============================================================
            // AGGRESSION
            //==============================================================

            x =
                aggressionStage (
                    x,
                    aggression);

            //==============================================================
            // U2B
            // RECOVERY / BUFFER
            //==============================================================

            x =
                opAmpStage (
                    U2B,
                    x,
                    1.08f,
                    0.90f);

            //==============================================================
            // TONE
            //==============================================================

            x =
                toneStack (
                    x,
                    bass,
                    mid,
                    treble);

            //==============================================================
            // U3A
            // TONE BUFFER
            //==============================================================

            x =
                opAmpStage (
                    U3A,
                    x,
                    1.04f,
                    0.80f);

            //==============================================================
            // U3B
            // OUTPUT BUFFER
            //==============================================================

            x =
                opAmpStage (
                    U3B,
                    x,
                    1.0f,
                    0.80f);

            //==============================================================
            // OUTPUT HIGH PASS
            //==============================================================

            x =
                outputHP.highpass (
                    x,
                    12.0f,
                    static_cast<float> (sampleRate));

            //==============================================================
            // VOLUME
            //==============================================================

            x *= outputLevel;

            //==============================================================
            // FINAL ANALOG-LIKE OUTPUT LIMIT
            //==============================================================

            x =
                std::tanh (
                    x * 0.92f);

            data[sample] = x;
        }
    }
}

//==============================================================================
// EDITOR
//==============================================================================

juce::AudioProcessorEditor*
RG_G4AudioProcessor::createEditor()
{
    return new RG_G4AudioProcessorEditor (*this);
}

//==============================================================================
// NAME
//==============================================================================

const juce::String
RG_G4AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

//==============================================================================
// SAVE STATE
//==============================================================================

void RG_G4AudioProcessor::getStateInformation (
    juce::MemoryBlock& destData)
{
    auto state =
        apvts.copyState();

    std::unique_ptr<juce::XmlElement> xml (
        state.createXml());

    copyXmlToBinary (
        *xml,
        destData);
}

//==============================================================================
// LOAD STATE
//==============================================================================

void RG_G4AudioProcessor::setStateInformation (
    const void* data,
    int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (
        getXmlFromBinary (
            data,
            sizeInBytes));

    if (xml != nullptr
        && xml->hasTagName (
            apvts.state.getType()))
    {
        apvts.replaceState (
            juce::ValueTree::fromXml (
                *xml));
    }
}
//==============================================================
// CREATE PLUGIN INSTANCE
//==============================================================

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RG_G4AudioProcessor();
}
