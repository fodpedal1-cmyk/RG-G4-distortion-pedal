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

    layout.add (
        std::make_unique<juce::AudioParameterFloat>(
            "GAIN",
            "Gain",
            Range (0.0f, 1.0f, 0.001f),
            0.55f));

    layout.add (
        std::make_unique<juce::AudioParameterFloat>(
            "BASS",
            "Bass",
            Range (0.0f, 1.0f, 0.001f),
            0.50f));

    layout.add (
        std::make_unique<juce::AudioParameterFloat>(
            "MID",
            "Mid",
            Range (0.0f, 1.0f, 0.001f),
            0.50f));

    layout.add (
        std::make_unique<juce::AudioParameterFloat>(
            "TREBLE",
            "Treble",
            Range (0.0f, 1.0f, 0.001f),
            0.50f));

    layout.add (
        std::make_unique<juce::AudioParameterFloat>(
            "VOLUME",
            "Volume",
            Range (0.0f, 1.0f, 0.001f),
            0.70f));

    layout.add (
        std::make_unique<juce::AudioParameterInt>(
            "AGGRESSION",
            "Aggression",
            0,
            2,
            1));

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

    sampleRate =
        juce::jmax (
            22050.0,
            newSampleRate);

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
    if (!std::isfinite (target))
        target = 0.0f;

    fs =
        juce::jmax (
            22050.0f,
            fs);

    slewMultiplier =
        juce::jlimit (
            0.1f,
            2.0f,
            slewMultiplier);

    // Stable normalized-audio slew limit.
    const float maximumStep =
        0.45f * slewMultiplier;

    const float difference =
        target - output;

    output +=
        juce::jlimit (
            -maximumStep,
            maximumStep,
            difference);

    // Analog-like op-amp saturation.
    const float rail =
        3.2f;

    output =
        std::tanh (
            output / rail)
        * rail;

    if (!std::isfinite (output))
        output = 0.0f;

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
    if (!std::isfinite (input))
        input = 0.0f;

    fs =
        juce::jmax (
            22050.0f,
            fs);

    cutoff =
        juce::jlimit (
            5.0f,
            fs * 0.45f,
            cutoff);

    const float coefficient =
        std::exp (
            -2.0f *
            juce::MathConstants<float>::pi *
            cutoff /
            fs);

    state =
        (1.0f - coefficient) * input
        + coefficient * state;

    if (!std::isfinite (state))
        state = 0.0f;

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

    const float result =
        input - low;

    return std::isfinite (result)
        ? result
        : 0.0f;
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
    if (!std::isfinite (input))
        input = 0.0f;

    gain =
        juce::jlimit (
            0.05f,
            40.0f,
            gain);

    const float target =
        juce::jlimit (
            -8.0f,
            8.0f,
            input * gain);

    return
        opAmp.process (
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
    if (!std::isfinite (input))
        input = 0.0f;

    drive =
        juce::jlimit (
            1.0f,
            8.0f,
            drive);

    float threshold =
        0.62f;

    if (aggression == 1)
        threshold = 0.54f;

    else if (aggression == 2)
        threshold = 0.46f;

    const float x =
        juce::jlimit (
            -8.0f,
            8.0f,
            input * drive);

    float result;

    if (x >= threshold)
    {
        const float amount =
            x - threshold;

        result =
            threshold
            + std::tanh (
                amount * 2.8f)
              * 1.05f;
    }
    else if (x <= -threshold)
    {
        const float amount =
            -x - threshold;

        result =
            -threshold
            - std::tanh (
                amount * 2.6f)
              * 1.00f;
    }
    else
    {
        result = x;
    }

    result =
        juce::jlimit (
            -1.85f,
            1.85f,
            result);

    return result;
}

//==============================================================================
// AGGRESSION
//==============================================================================

float RG_G4AudioProcessor::aggressionStage (
    float input,
    int aggression)
{
    if (!std::isfinite (input))
        input = 0.0f;

    if (aggression == 0)
        return input;

    const float fs =
        static_cast<float> (sampleRate);

    //==========================================================================
    // BLUE
    //==========================================================================

    if (aggression == 1)
    {
        const float tight =
            stage2HP.highpass (
                input,
                85.0f,
                fs);

        const float body =
            stage2LP.lowpass (
                input,
                6500.0f,
                fs);

        return
            tight * 0.82f
            + body * 0.18f;
    }

    //==========================================================================
    // RED
    //==========================================================================

    const float tight =
        stage2HP.highpass (
            input,
            105.0f,
            fs);

    const float body =
        stage2LP.lowpass (
            input,
            7200.0f,
            fs);

    const float saturated =
        std::tanh (
            input * 1.35f);

    return
        tight * 0.68f
        + body * 0.18f
        + saturated * 0.14f;
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
    if (!std::isfinite (input))
        input = 0.0f;

    const float fs =
        static_cast<float> (sampleRate);

    bass =
        juce::jlimit (
            0.0f,
            1.0f,
            bass);

    mid =
        juce::jlimit (
            0.0f,
            1.0f,
            mid);

    treble =
        juce::jlimit (
            0.0f,
            1.0f,
            treble);

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
            950.0f,
            fs);

    const float middle =
        midLow - low;

    //==========================================================================
    // HIGH
    //==========================================================================

    const float trebleLow =
        trebleLP.lowpass (
            input,
            3400.0f,
            fs);

    const float high =
        input - trebleLow;

    //==========================================================================
    // CONTROL RESPONSE
    //==========================================================================

    const float bassGain =
        juce::jmap (
            bass,
            0.0f,
            1.0f,
            0.45f,
            1.55f);

    const float midGain =
        juce::jmap (
            mid,
            0.0f,
            1.0f,
            0.35f,
            1.45f);

    const float trebleGain =
        juce::jmap (
            treble,
            0.0f,
            1.0f,
            0.45f,
            1.60f);

    float result =
        input * 0.10f
        + low * bassGain * 0.55f
        + middle * midGain * 0.80f
        + high * trebleGain * 0.55f;

    return
        juce::jlimit (
            -3.0f,
            3.0f,
            result);
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

    if (channels <= 0 || samples <= 0)
        return;

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
            ? juce::jlimit (
                0,
                2,
                static_cast<int> (
                    aggressionParameter->load()))
            : 1;

    //==========================================================================
    // SAFE PARAMETERS
    //==========================================================================

    const float safeGain =
        juce::jlimit (
            0.0f,
            1.0f,
            gain);

    const float safeBass =
        juce::jlimit (
            0.0f,
            1.0f,
            bass);

    const float safeMid =
        juce::jlimit (
            0.0f,
            1.0f,
            mid);

    const float safeTreble =
        juce::jlimit (
            0.0f,
            1.0f,
            treble);

    const float safeVolume =
        juce::jlimit (
            0.0f,
            1.0f,
            volume);

    //==========================================================================
    // HIGH GAIN ARCHITECTURE
    //==========================================================================

    const float preGain =
        juce::jmap (
            safeGain,
            0.0f,
            1.0f,
            1.0f,
            5.0f);

    const float mainGain =
        juce::jmap (
            safeGain,
            0.0f,
            1.0f,
            1.0f,
            7.5f);

    const float clipDrive =
        juce::jmap (
            safeGain,
            0.0f,
            1.0f,
            1.0f,
            3.2f);

    const float postGain =
        juce::jmap (
            safeGain,
            0.0f,
            1.0f,
            1.0f,
            3.8f);

    const float finalGain =
        juce::jmap (
            safeGain,
            0.0f,
            1.0f,
            0.90f,
            1.45f);

    const float outputLevel =
        juce::jmap (
            safeVolume,
            0.0f,
            1.0f,
            0.0f,
            0.82f);

    //==========================================================================
    // MONO DSP STATE
    //
    // The existing processor stores one state per filter/op-amp.
    // Therefore process the first channel and copy it to the other channel.
    // This prevents L/R state contamination and crackling.
    //==========================================================================

    float* inputData =
        buffer.getWritePointer (0);

    for (int sample = 0;
         sample < samples;
         ++sample)
    {
        float x =
            inputData[sample];

        if (!std::isfinite (x))
            x = 0.0f;

        x =
            juce::jlimit (
                -2.0f,
                2.0f,
                x);

        //==================================================================
        // INPUT HIGH PASS
        //==================================================================

        x =
            inputHP.highpass (
                x,
                24.0f,
                static_cast<float> (sampleRate));

        //==================================================================
        // U1A PREAMP
        //==================================================================

        x =
            opAmpStage (
                U1A,
                x,
                preGain,
                0.95f);

        //==================================================================
        // INPUT LOW PASS
        //==================================================================

        x =
            inputLP.lowpass (
                x,
                17500.0f,
                static_cast<float> (sampleRate));

        //==================================================================
        // U1B MAIN HIGH GAIN
        //==================================================================

        x =
            opAmpStage (
                U1B,
                x,
                mainGain,
                0.95f);

        //==================================================================
        // PRE-CLIP TIGHTENING
        //==================================================================

        x =
            stage1HP.highpass (
                x,
                45.0f,
                static_cast<float> (sampleRate));

        x =
            stage1LP.lowpass (
                x,
                14500.0f,
                static_cast<float> (sampleRate));

        //==================================================================
        // LED CLIPPING
        //==================================================================

        x =
            ledClip (
                x,
                clipDrive,
                aggression);

        //==================================================================
        // U2A POST CLIP GAIN
        //==================================================================

        x =
            opAmpStage (
                U2A,
                x,
                postGain,
                0.90f);

        //==================================================================
        // SECOND SATURATION STAGE
        //==================================================================

        const float saturationAmount =
            juce::jmap (
                safeGain,
                0.0f,
                1.0f,
                1.0f,
                2.4f);

        x =
            std::tanh (
                x * saturationAmount);

        //==================================================================
        // AGGRESSION
        //==================================================================

        x =
            aggressionStage (
                x,
                aggression);

        //==================================================================
        // U2B RECOVERY
        //==================================================================

        x =
            opAmpStage (
                U2B,
                x,
                1.25f,
                0.85f);

        //==================================================================
        // SECOND SOFT CLIP
        //==================================================================

        const float secondClip =
            juce::jmap (
                safeGain,
                0.0f,
                1.0f,
                1.0f,
                1.75f);

        x =
            std::tanh (
                x * secondClip);

        //==================================================================
        // TONE STACK
        //==================================================================

        x =
            toneStack (
                x,
                safeBass,
                safeMid,
                safeTreble);

        //==================================================================
        // U3A TONE BUFFER
        //==================================================================

        x =
            opAmpStage (
                U3A,
                x,
                1.05f,
                0.80f);

        //==================================================================
        // U3B OUTPUT BUFFER
        //==================================================================

        x =
            opAmpStage (
                U3B,
                x,
                finalGain,
                0.80f);

        //==================================================================
        // OUTPUT HIGH PASS
        //==================================================================

        x =
            outputHP.highpass (
                x,
                18.0f,
                static_cast<float> (sampleRate));

        //==================================================================
        // VOLUME
        //==================================================================

        x *= outputLevel;

        //==================================================================
        // FINAL SAFETY LIMIT
        //==================================================================

        x =
            std::tanh (
                x * 1.05f);

        if (!std::isfinite (x))
            x = 0.0f;

        inputData[sample] = x;
    }

    //==========================================================================
    // COPY PROCESSED MONO SIGNAL TO OTHER CHANNELS
    //==========================================================================

    for (int channel = 1;
         channel < channels;
         ++channel)
    {
        buffer.copyFrom (
            channel,
            0,
            buffer,
            0,
            0,
            samples);
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

//==============================================================================
// CREATE PLUGIN INSTANCE
//==============================================================================

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RG_G4AudioProcessor();
}
