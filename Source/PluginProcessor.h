#pragma once

#include <JuceHeader.h>

//==============================================================
// RG PRECISION DRIVE AUDIO PROCESSOR
//
// Signal chain:
//
// INPUT
//   ↓
// BUFFER / DC BLOCK
//   ↓
// NOISE GATE
//   ↓
// ATTACK 1-6 RC NETWORK
//   ↓
// OP-AMP DRIVE
//   ↓
// DIODE CLIPPING
//   ↓
// BRIGHT
//   ↓
// VOLUME
//   ↓
// OUTPUT
//
// NOTE:
// No JUCE Oversampling is used.
// Component values marked MODEL/REFERENCE are modelling values,
// not a claim of the exact original pedal schematic.
//==============================================================

class RG_Precision_DriveAudioProcessor
    : public juce::AudioProcessor
{
public:

    //==========================================================
    RG_Precision_DriveAudioProcessor();
    ~RG_Precision_DriveAudioProcessor() override;

    //==========================================================
    void prepareToPlay (double sampleRate,
                        int samplesPerBlock) override;

    void releaseResources() override;

    bool isBusesLayoutSupported (
        const BusesLayout& layouts) const override;

    void processBlock (
        juce::AudioBuffer<float>&,
        juce::MidiBuffer&) override;

    //==========================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==========================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;

    double getTailLengthSeconds() const override;

    //==========================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;

    const juce::String getProgramName (int index) override;
    void changeProgramName (
        int index,
        const juce::String& newName) override;

    //==========================================================
    void getStateInformation (
        juce::MemoryBlock& destData) override;

    void setStateInformation (
        const void* data,
        int sizeInBytes) override;

    //==========================================================
    // PARAMETER LAYOUT
    //==========================================================

    static juce::AudioProcessorValueTreeState::ParameterLayout
    createParameterLayout();

    juce::AudioProcessorValueTreeState parameters;

private:

    //==========================================================
    // RC FILTER
    //==========================================================

    struct RCFilter
    {
        float lowpass (
            float input,
            float cutoff,
            float sampleRate);

        float highpass (
            float input,
            float cutoff,
            float sampleRate);

        void reset();

        float lpState = 0.0f;
        float hpState = 0.0f;
        float hpInput = 0.0f;
    };

    //==========================================================
    // DC BLOCK
    //==========================================================

    struct DCBlock
    {
        float process (float input);
        void reset();

        float x1 = 0.0f;
        float y1 = 0.0f;
    };

    //==========================================================
    // OP-AMP MODEL
    //==========================================================

    struct OpAmpModel
    {
        float gainBandwidth = 3000000.0f;
        float slewRate = 1700000.0f;

        // MODEL / REFERENCE VALUES
        float openLoopGain = 100000.0f;
        float outputLimit = 4.2f;

        float process (
            float input,
            float gain,
            float sampleRate);

        void reset();

        float state = 0.0f;
    };

    //==========================================================
    // DIODE CLIPPER
    //==========================================================

    struct DiodeClipper
    {
        float process (float input);

        // MODEL / REFERENCE diode parameters
        float forwardVoltage = 0.62f;
        float softness = 0.12f;
    };

    //==========================================================
    // ATTACK NETWORK
    //==========================================================

    struct AttackNetwork
    {
        float process (
            float input,
            int attackPosition,
            float sampleRate);

        void reset();

        RCFilter filter;

        // MODEL / REFERENCE RC values
        //
        // Position 1 = thicker / lower cutoff
        // Position 6 = tighter / higher cutoff
        //
        static constexpr float resistance = 10000.0f;

        static constexpr float capacitors[6] =
        {
            470.0e-9f,
            220.0e-9f,
            100.0e-9f,
             68.0e-9f,
             47.0e-9f,
             33.0e-9f
        };
    };

    //==========================================================
    // GATE
    //==========================================================

    struct GateDetector
    {
        float process (
            float input,
            float gateAmount,
            float sampleRate);

        void reset();

        float envelope = 0.0f;
        float gain = 1.0f;
    };

    //==========================================================
    // BRIGHT NETWORK
    //==========================================================

    struct BrightNetwork
    {
        float process (
            float input,
            float brightAmount,
            float sampleRate);

        void reset();

        RCFilter filter;
    };

    //==========================================================
    // PROCESSING FUNCTIONS
    //==========================================================

    float processSample (
        float input,
        int channel);

    float processGate (
        float input,
        float gateAmount,
        int channel);

    float processAttack (
        float input,
        float attack,
        int channel);

    float processDrive (
        float input,
        float drive,
        int channel);

    float processBright (
        float input,
        float bright,
        int channel);

    //==========================================================
    // PARAMETER SMOOTHING
    //==========================================================

    juce::SmoothedValue<float> volumeSmoothed;
    juce::SmoothedValue<float> brightSmoothed;
    juce::SmoothedValue<float> driveSmoothed;
    juce::SmoothedValue<float> gateSmoothed;

    juce::SmoothedValue<float> attackSmoothed;

    //==========================================================
    // DSP STAGES - LEFT
    //==========================================================

    DCBlock dcBlockL;

    GateDetector gateL;

    AttackNetwork attackL;

    OpAmpModel opAmpBufferL;
    OpAmpModel opAmpDriveL;

    DiodeClipper diodeL;

    BrightNetwork brightL;

    //==========================================================
    // DSP STAGES - RIGHT
    //==========================================================

    DCBlock dcBlockR;

    GateDetector gateR;

    AttackNetwork attackR;

    OpAmpModel opAmpBufferR;
    OpAmpModel opAmpDriveR;

    DiodeClipper diodeR;

    BrightNetwork brightR;

    //==========================================================
    // SAMPLE RATE
    //==========================================================

    double currentSampleRate = 44100.0;

    //==========================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (
        RG_Precision_DriveAudioProcessor)
};
