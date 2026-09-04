#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================
// RG PRECISION DRIVE
//
// INPUT
//  ↓
// BUFFER / DC BLOCK
//  ↓
// GATE
//  ↓
// ATTACK RC NETWORK
//  ↓
// OP-AMP DRIVE
//  ↓
// DIODE CLIPPING
//  ↓
// BRIGHT
//  ↓
// VOLUME
//  ↓
// OUTPUT
//
// NO OVERSAMPLING
//==============================================================


//==============================================================
// RC FILTER
//==============================================================

float RG_Precision_DriveAudioProcessor::RCFilter::lowpass (
    float input,
    float cutoff,
    float sampleRate)
{
    if (sampleRate <= 0.0f)
        return input;

    cutoff = juce::jlimit (
        1.0f,
        sampleRate * 0.45f,
        cutoff);

    const float dt = 1.0f / sampleRate;
    const float rc = 1.0f /
                     (2.0f * juce::MathConstants<float>::pi * cutoff);

    const float alpha = dt / (rc + dt);

    lpState += alpha * (input - lpState);

    return lpState;
}


//==============================================================

float RG_Precision_DriveAudioProcessor::RCFilter::highpass (
    float input,
    float cutoff,
    float sampleRate)
{
    if (sampleRate <= 0.0f)
        return input;

    cutoff = juce::jlimit (
        1.0f,
        sampleRate * 0.45f,
        cutoff);

    const float dt = 1.0f / sampleRate;
    const float rc = 1.0f /
                     (2.0f * juce::MathConstants<float>::pi * cutoff);

    const float alpha = rc / (rc + dt);

    const float output =
        alpha * (hpState + input - hpInput);

    hpState = output;
    hpInput = input;

    return output;
}


//==============================================================

void RG_Precision_DriveAudioProcessor::RCFilter::reset()
{
    lpState = 0.0f;
    hpState = 0.0f;
    hpInput = 0.0f;
}


//==============================================================
// DC BLOCK
//
// MODEL / REFERENCE:
// R = 1M
// C = 47nF
//==============================================================

float RG_Precision_DriveAudioProcessor::DCBlock::process (
    float input)
{
    constexpr float R = 1000000.0f;
    constexpr float C = 47.0e-9f;

    const float alpha =
        R * C /
        (R * C + 1.0f / 44100.0f);

    const float output =
        input - x1 + alpha * y1;

    x1 = input;
    y1 = output;

    return output;
}


//==============================================================

void RG_Precision_DriveAudioProcessor::DCBlock::reset()
{
    x1 = 0.0f;
    y1 = 0.0f;
}


//==============================================================
// OP-AMP MODEL
//
// RC4558 / TL072 / TL062 style behaviour.
//
// This is a simplified component-stage model.
// It is NOT a transistor-level SPICE simulation.
//==============================================================

float RG_Precision_DriveAudioProcessor::OpAmpModel::process (
    float input,
    float gain,
    float sampleRate)
{
    gain = juce::jmax (1.0f, gain);

    // Closed-loop target
    const float target = input * gain;

    // GBW-based response
    float bandwidth =
        gainBandwidth / gain;

    bandwidth =
        juce::jlimit (
            20.0f,
            sampleRate * 0.45f,
            bandwidth);

    const float dt = 1.0f / sampleRate;

    const float alpha =
        1.0f -
        std::exp (
            -2.0f *
            juce::MathConstants<float>::pi *
            bandwidth *
            dt);

    float desired =
        state + alpha * (target - state);

    // Slew-rate limiting
    const float maxChange =
        slewRate * dt;

    const float difference =
        desired - state;

    if (difference > maxChange)
        desired = state + maxChange;

    else if (difference < -maxChange)
        desired = state - maxChange;

    // Model output rail
    desired =
        juce::jlimit (
            -outputLimit,
            outputLimit,
            desired);

    state = desired;

    return state;
}


//==============================================================

void RG_Precision_DriveAudioProcessor::OpAmpModel::reset()
{
    state = 0.0f;
}


//==============================================================
// DIODE CLIPPER
//
// MODEL / REFERENCE:
// Forward voltage ≈ 0.62 V
//
// Smooth diode transition rather than generic final tanh.
//==============================================================

float RG_Precision_DriveAudioProcessor::DiodeClipper::process (
    float input)
{
    const float sign =
        input >= 0.0f ? 1.0f : -1.0f;

    const float x =
        std::abs (input);

    if (x <= forwardVoltage)
        return input;

    const float excess =
        x - forwardVoltage;

    const float clipped =
        forwardVoltage +
        softness *
        std::tanh (
            excess / softness);

    return sign * clipped;
}


//==============================================================
// ATTACK NETWORK
//==============================================================

float RG_Precision_DriveAudioProcessor::AttackNetwork::process (
    float input,
    int attackPosition,
    float sampleRate)
{
    attackPosition =
        juce::jlimit (
            1,
            6,
            attackPosition);

    const int index =
        attackPosition - 1;

    const float C =
        capacitors[index];

    // RC high-pass corner:
    //
    // Fc = 1 / (2πRC)
    //
    const float cutoff =
        1.0f /
        (2.0f *
         juce::MathConstants<float>::pi *
         resistance *
         C);

    return filter.highpass (
        input,
        cutoff,
        sampleRate);
}


//==============================================================

void RG_Precision_DriveAudioProcessor::AttackNetwork::reset()
{
    filter.reset();
}


//==============================================================
// GATE DETECTOR
//==============================================================

float RG_Precision_DriveAudioProcessor::GateDetector::process (
    float input,
    float gateAmount,
    float sampleRate)
{
    gateAmount =
        juce::jlimit (
            0.0f,
            1.0f,
            gateAmount);

    const float level =
        std::abs (input);

    // Envelope follower
    constexpr float attackMs = 2.5f;
    constexpr float releaseMs = 80.0f;

    const float attackCoeff =
        std::exp (
            -1.0f /
            (sampleRate *
             attackMs *
             0.001f));

    const float releaseCoeff =
        std::exp (
            -1.0f /
            (sampleRate *
             releaseMs *
             0.001f));

    if (level > envelope)
    {
        envelope =
            attackCoeff * envelope +
            (1.0f - attackCoeff) * level;
    }
    else
    {
        envelope =
            releaseCoeff * envelope +
            (1.0f - releaseCoeff) * level;
    }

    // Gate threshold increases with Gate knob.
    //
    // MODEL / REFERENCE:
    // Gate potentiometer reported around B10K.
    //
    const float threshold =
        0.002f +
        gateAmount * 0.065f;

    float targetGain = 1.0f;

    if (envelope < threshold)
    {
        const float ratio =
            envelope /
            juce::jmax (
                threshold,
                0.00001f);

        // Soft attenuation.
        targetGain =
            0.08f +
            0.92f *
            std::pow (
                juce::jlimit (
                    0.0f,
                    1.0f,
                    ratio),
                4.0f);
    }

    // Smooth gate gain.
    constexpr float gateAttackMs = 1.0f;
    constexpr float gateReleaseMs = 30.0f;

    const float attackGainCoeff =
        std::exp (
            -1.0f /
            (sampleRate *
             gateAttackMs *
             0.001f));

    const float releaseGainCoeff =
        std::exp (
            -1.0f /
            (sampleRate *
             gateReleaseMs *
             0.001f));

    if (targetGain > gain)
    {
        gain =
            attackGainCoeff * gain +
            (1.0f - attackGainCoeff) *
            targetGain;
    }
    else
    {
        gain =
            releaseGainCoeff * gain +
            (1.0f - releaseGainCoeff) *
            targetGain;
    }

    // Gate amount determines how strongly the attenuation works.
    const float appliedGain =
        1.0f -
        gateAmount * (1.0f - gain);

    return input * appliedGain;
}


//==============================================================

void RG_Precision_DriveAudioProcessor::GateDetector::reset()
{
    envelope = 0.0f;
    gain = 1.0f;
}


//==============================================================
// BRIGHT NETWORK
//
// MODEL / REFERENCE:
// Bright pot ≈ B5K.
//
// Higher knob position raises the high-frequency corner.
//==============================================================

float RG_Precision_DriveAudioProcessor::BrightNetwork::process (
    float input,
    float brightAmount,
    float sampleRate)
{
    brightAmount =
        juce::jlimit (
            0.0f,
            1.0f,
            brightAmount);

    const float cutoff =
        1800.0f +
        brightAmount * 10000.0f;

    const float high =
        input -
        filter.lowpass (
            input,
            cutoff,
            sampleRate);

    // Keep the main signal and add controlled high-frequency content.
    const float amount =
        0.05f +
        brightAmount * 0.45f;

    return input + high * amount;
}


//==============================================================

void RG_Precision_DriveAudioProcessor::BrightNetwork::reset()
{
    filter.reset();
}


//==============================================================
// CONSTRUCTOR
//==============================================================

RG_Precision_DriveAudioProcessor::
RG_Precision_DriveAudioProcessor()
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
      parameters (
          *this,
          nullptr,
          "PARAMETERS",
          createParameterLayout())
{
}


//==============================================================

RG_Precision_DriveAudioProcessor::
~RG_Precision_DriveAudioProcessor()
{
}


//==============================================================
// PARAMETER LAYOUT
//==============================================================

juce::AudioProcessorValueTreeState::ParameterLayout
RG_Precision_DriveAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<
        juce::RangedAudioParameter>> params;

    //==========================================================
    // VOLUME
    //
    // Reference pot:
    // B100K
    //==========================================================

    params.push_back (
        std::make_unique<
        juce::AudioParameterFloat> (
            "VOLUME",
            "VOL",
            juce::NormalisableRange<float> (
                0.0f,
                1.0f,
                0.001f),
            0.75f));

    //==========================================================
    // BRIGHT
    //
    // Reference pot:
    // B5K
    //==========================================================

    params.push_back (
        std::make_unique<
        juce::AudioParameterFloat> (
            "BRIGHT",
            "BRIGHT",
            juce::NormalisableRange<float> (
                0.0f,
                1.0f,
                0.001f),
            0.50f));

    //==========================================================
    // ATTACK
    //
    // 6-position rotary switch
    //==========================================================

    params.push_back (
        std::make_unique<
        juce::AudioParameterInt> (
            "ATTACK",
            "ATTACK",
            1,
            6,
            3));

    //==========================================================
    // DRIVE
    //
    // GUI range 1 - 11
    //==========================================================

    params.push_back (
        std::make_unique<
        juce::AudioParameterFloat> (
            "DRIVE",
            "DRIVE",
            juce::NormalisableRange<float> (
                1.0f,
                11.0f,
                0.01f),
            5.0f));

    //==========================================================
    // GATE
    //
    // Reference pot around B10K
    //==========================================================

    params.push_back (
        std::make_unique<
        juce::AudioParameterFloat> (
            "GATE",
            "GATE",
            juce::NormalisableRange<float> (
                0.0f,
                1.0f,
                0.001f),
            0.15f));

    //==========================================================
    // BYPASS
    //==========================================================

    params.push_back (
        std::make_unique<
        juce::AudioParameterBool> (
            "BYPASS",
            "BYPASS",
            false));

    return {
        params.begin(),
        params.end()
    };
}


//==============================================================
// PREPARE
//==============================================================

void RG_Precision_DriveAudioProcessor::prepareToPlay (
    double sampleRate,
    int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);

    currentSampleRate =
        sampleRate;

    //==========================================================
    // Smooth parameter changes
    //==========================================================

    volumeSmoothed.reset (
        sampleRate,
        0.020);

    brightSmoothed.reset (
        sampleRate,
        0.020);

    driveSmoothed.reset (
        sampleRate,
        0.020);

    gateSmoothed.reset (
        sampleRate,
        0.020);

    attackSmoothed.reset (
        sampleRate,
        0.020);

    //==========================================================
    // Initial values
    //==========================================================

    volumeSmoothed.setCurrentAndTargetValue (
        parameters.getRawParameterValue (
            "VOLUME")->load());

    brightSmoothed.setCurrentAndTargetValue (
        parameters.getRawParameterValue (
            "BRIGHT")->load());

    driveSmoothed.setCurrentAndTargetValue (
        parameters.getRawParameterValue (
            "DRIVE")->load());

    gateSmoothed.setCurrentAndTargetValue (
        parameters.getRawParameterValue (
            "GATE")->load());

    attackSmoothed.setCurrentAndTargetValue (
        parameters.getRawParameterValue (
            "ATTACK")->load());

    //==========================================================
    // RESET DSP
    //==========================================================

    dcBlockL.reset();
    dcBlockR.reset();

    gateL.reset();
    gateR.reset();

    attackL.reset();
    attackR.reset();

    opAmpBufferL.reset();
    opAmpBufferR.reset();

    opAmpDriveL.reset();
    opAmpDriveR.reset();

    brightL.reset();
    brightR.reset();
}


//==============================================================
// RELEASE
//==============================================================

void RG_Precision_DriveAudioProcessor::releaseResources()
{
}


//==============================================================
// BUS LAYOUT
//==============================================================

bool RG_Precision_DriveAudioProcessor::
isBusesLayoutSupported (
    const BusesLayout& layouts) const
{
    const auto& mainIn =
        layouts.getChannelSet (
            true,
            0);

    const auto& mainOut =
        layouts.getChannelSet (
            false,
            0);

    if (mainOut !=
        juce::AudioChannelSet::mono() &&
        mainOut !=
        juce::AudioChannelSet::stereo())
    {
        return false;
    }

    if (mainIn != mainOut)
        return false;

    return true;
}


//==============================================================
// PROCESS BLOCK
//==============================================================

void RG_Precision_DriveAudioProcessor::processBlock (
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;

    const int numChannels =
        buffer.getNumChannels();

    const int numSamples =
        buffer.getNumSamples();

    if (numChannels == 0 ||
        numSamples == 0)
    {
        return;
    }

    //==========================================================
    // BYPASS
    //==========================================================

    const bool bypass =
        parameters.getRawParameterValue (
            "BYPASS")->load() > 0.5f;

    if (bypass)
        return;

    //==========================================================
    // TARGET PARAMETERS
    //==========================================================

    volumeSmoothed.setTargetValue (
        parameters.getRawParameterValue (
            "VOLUME")->load());

    brightSmoothed.setTargetValue (
        parameters.getRawParameterValue (
            "BRIGHT")->load());

    driveSmoothed.setTargetValue (
        parameters.getRawParameterValue (
            "DRIVE")->load());

    gateSmoothed.setTargetValue (
        parameters.getRawParameterValue (
            "GATE")->load());

    attackSmoothed.setTargetValue (
        parameters.getRawParameterValue (
            "ATTACK")->load());

    const int attackPosition =
        juce::jlimit (
            1,
            6,
            static_cast<int> (
                std::round (
                    parameters.getRawParameterValue (
                        "ATTACK")->load())));

    //==========================================================
    // PROCESS EACH SAMPLE
    //==========================================================

    for (int sample = 0;
         sample < numSamples;
         ++sample)
    {
        const float volume =
            volumeSmoothed.getNextValue();

        const float bright =
            brightSmoothed.getNextValue();

        const float drive =
            driveSmoothed.getNextValue();

        const float gate =
            gateSmoothed.getNextValue();

        const float attack =
            attackSmoothed.getNextValue();

        juce::ignoreUnused (attack);

        const int currentAttack =
            juce::jlimit (
                1,
                6,
                static_cast<int> (
                    std::round (
                        attack)));

        //======================================================
        // LEFT
        //======================================================

        if (numChannels >= 1)
        {
            float x =
                buffer.getSample (
                    0,
                    sample);

            x =
                processSample (
                    x,
                    0);

            // GATE
            x =
                processGate (
                    x,
                    gate,
                    0);

            // ATTACK
            x =
                processAttack (
                    x,
                    static_cast<float> (
                        currentAttack),
                    0);

            // DRIVE
            x =
                processDrive (
                    x,
                    drive,
                    0);

            // BRIGHT
            x =
                processBright (
                    x,
                    bright,
                    0);

            // VOLUME
            //
            // MODEL:
            // output range approximately
            // 0.05 -> 2.45
            //
            const float outputGain =
                0.05f +
                volume * 2.40f;

            x *= outputGain;

            // Safety only.
            // This is not an additional distortion stage.
            x =
                juce::jlimit (
                    -1.0f,
                    1.0f,
                    x);

            buffer.setSample (
                0,
                sample,
                x);
        }

        //======================================================
        // RIGHT
        //======================================================

        if (numChannels >= 2)
        {
            float x =
                buffer.getSample (
                    1,
                    sample);

            x =
                processSample (
                    x,
                    1);

            x =
                processGate (
                    x,
                    gate,
                    1);

            x =
                processAttack (
                    x,
                    static_cast<float> (
                        currentAttack),
                    1);

            x =
                processDrive (
                    x,
                    drive,
                    1);

            x =
                processBright (
                    x,
                    bright,
                    1);

            const float outputGain =
                0.05f +
                volume * 2.40f;

            x *= outputGain;

            x =
                juce::jlimit (
                    -1.0f,
                    1.0f,
                    x);

            buffer.setSample (
                1,
                sample,
                x);
        }
    }
}


//==============================================================
// INPUT / BUFFER / DC BLOCK
//==============================================================

float RG_Precision_DriveAudioProcessor::processSample (
    float input,
    int channel)
{
    juce::ignoreUnused (channel);

    // INPUT COUPLING
    //
    // MODEL / REFERENCE:
    // R = 1M
    // C = 47nF
    //
    // DC blocker keeps the drive stage centered.

    if (channel == 0)
    {
        float x =
            dcBlockL.process (input);

        // TL062/TL072 style buffer model.
        //
        // Unity-gain buffer.
        x =
            opAmpBufferL.process (
                x,
                1.0f,
                static_cast<float> (
                    currentSampleRate));

        return x;
    }

    float x =
        dcBlockR.process (input);

    x =
        opAmpBufferR.process (
            x,
            1.0f,
            static_cast<float> (
                currentSampleRate));

    return x;
}


//==============================================================
// GATE
//==============================================================

float RG_Precision_DriveAudioProcessor::processGate (
    float input,
    float gateAmount,
    int channel)
{
    gateAmount =
        juce::jlimit (
            0.0f,
            1.0f,
            gateAmount);

    if (channel == 0)
        return gateL.process (
            input,
            gateAmount,
            static_cast<float> (
                currentSampleRate));

    return gateR.process (
        input,
        gateAmount,
        static_cast<float> (
            currentSampleRate));
}


//==============================================================
// ATTACK
//==============================================================

float RG_Precision_DriveAudioProcessor::processAttack (
    float input,
    float attack,
    int channel)
{
    const int position =
        juce::jlimit (
            1,
            6,
            static_cast<int> (
                std::round (
                    attack)));

    if (channel == 0)
        return attackL.process (
            input,
            position,
            static_cast<float> (
                currentSampleRate));

    return attackR.process (
        input,
        position,
        static_cast<float> (
            currentSampleRate));
}


//==============================================================
// DRIVE
//
// MODEL / REFERENCE VALUES:
//
// Rg = 4.7K
// Rf minimum = 10K
// Rf maximum additional = 500K
//
// Closed-loop gain:
//
// Gain = 1 + Rf / Rg
//
// Then op-amp → diode clipping.
//==============================================================

float RG_Precision_DriveAudioProcessor::processDrive (
    float input,
    float drive,
    int channel)
{
    // DRIVE parameter = 1.0 -> 11.0

    drive =
        juce::jlimit (
            1.0f,
            11.0f,
            drive);

    constexpr float Rg =
        4700.0f;

    constexpr float RfMinimum =
        10000.0f;

    constexpr float RfMaximumAdditional =
        500000.0f;

    const float driveNormalized =
        (drive - 1.0f) / 10.0f;

    const float Rf =
        RfMinimum +
        driveNormalized *
        RfMaximumAdditional;

    const float gain =
        1.0f +
        Rf / Rg;

    if (channel == 0)
    {
        float x =
            opAmpDriveL.process (
                input,
                gain,
                static_cast<float> (
                    currentSampleRate));

        x =
            diodeL.process (x);

        return x;
    }

    float x =
        opAmpDriveR.process (
            input,
            gain,
            static_cast<float> (
                currentSampleRate));

    x =
        diodeR.process (x);

    return x;
}


//==============================================================
// BRIGHT
//==============================================================

float RG_Precision_DriveAudioProcessor::processBright (
    float input,
    float bright,
    int channel)
{
    bright =
        juce::jlimit (
            0.0f,
            1.0f,
            bright);

    if (channel == 0)
        return brightL.process (
            input,
            bright,
            static_cast<float> (
                currentSampleRate));

    return brightR.process (
        input,
        bright,
        static_cast<float> (
            currentSampleRate));
}


//==============================================================
// EDITOR
//==============================================================

bool RG_Precision_DriveAudioProcessor::hasEditor() const
{
    return true;
}


//==============================================================

juce::AudioProcessorEditor*
RG_Precision_DriveAudioProcessor::createEditor()
{
    return new RG_Precision_DriveAudioProcessorEditor (
        *this);
}


//==============================================================
// BASIC INFORMATION
//==============================================================

const juce::String
RG_Precision_DriveAudioProcessor::getName() const
{
    return JucePlugin_Name;
}


//==============================================================

bool RG_Precision_DriveAudioProcessor::acceptsMidi() const
{
    return false;
}


//==============================================================

bool RG_Precision_DriveAudioProcessor::producesMidi() const
{
    return false;
}


//==============================================================

bool RG_Precision_DriveAudioProcessor::isMidiEffect() const
{
    return false;
}


//==============================================================

double RG_Precision_DriveAudioProcessor::
getTailLengthSeconds() const
{
    return 0.0;
}


//==============================================================
// PROGRAMS
//==============================================================

int RG_Precision_DriveAudioProcessor::
getNumPrograms()
{
    return 1;
}


//==============================================================

int RG_Precision_DriveAudioProcessor::
getCurrentProgram()
{
    return 0;
}


//==============================================================

void RG_Precision_DriveAudioProcessor::
setCurrentProgram (
    int index)
{
    juce::ignoreUnused (index);
}


//==============================================================

const juce::String
RG_Precision_DriveAudioProcessor::
getProgramName (
    int index)
{
    juce::ignoreUnused (index);

    return {};
}


//==============================================================

void RG_Precision_DriveAudioProcessor::
changeProgramName (
    int index,
    const juce::String& newName)
{
    juce::ignoreUnused (
        index,
        newName);
}


//==============================================================
// SAVE STATE
//==============================================================

void RG_Precision_DriveAudioProcessor::
getStateInformation (
    juce::MemoryBlock& destData)
{
    if (auto xml =
            parameters.copyState().createXml())
    {
        copyXmlToBinary (
            *xml,
            destData);
    }
}


//==============================================================
// LOAD STATE
//==============================================================

void RG_Precision_DriveAudioProcessor::
setStateInformation (
    const void* data,
    int sizeInBytes)
{
    if (auto xmlState =
            getXmlFromBinary (
                data,
                sizeInBytes))
    {
        if (xmlState->hasTagName (
                parameters.state.getType()))
        {
            parameters.replaceState (
                juce::ValueTree::fromXml (
                    *xmlState));
        }
    }
}


//==============================================================
// CREATE PLUGIN INSTANCE
//==============================================================

juce::AudioProcessor*
JUCE_CALLTYPE createPluginFilter()
{
    return new RG_Precision_DriveAudioProcessor();
}
