#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

//==============================================================================
// RG G4 - High Gain VST3
//
// Component-based reference DSP model
//
// 3 x TL072 ICs
// 6 individual op-amp sections:
//
// U1A = Input / Preamp
// U1B = Main Gain
// U2A = Clipping / Post Gain
// U2B = Aggression / Recovery
// U3A = Tone Buffer
// U3B = Output
//
// This is an original RG DSP implementation based on documented
// component/reference characteristics. It is not an official Revv circuit.
//==============================================================================

class RG_G4AudioProcessor : public juce::AudioProcessor
{
public:

    RG_G4AudioProcessor();
    ~RG_G4AudioProcessor() override = default;

    //==========================================================================
    void prepareToPlay (double sampleRate,
                        int samplesPerBlock) override;

    void releaseResources() override;

    bool isBusesLayoutSupported (
        const BusesLayout& layouts) const override;

    void processBlock (
        juce::AudioBuffer<float>&,
        juce::MidiBuffer&) override;

    //==========================================================================

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override;

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }

    double getTailLengthSeconds() const override
    {
        return 0.0;
    }

    //==========================================================================

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }

    void setCurrentProgram (int) override {}

    const juce::String getProgramName (int) override
    {
        return {};
    }

    void changeProgramName (
        int,
        const juce::String&) override {}

    //==========================================================================

    void getStateInformation (
        juce::MemoryBlock& destData) override;

    void setStateInformation (
        const void* data,
        int sizeInBytes) override;

    //==========================================================================

    static juce::AudioProcessorValueTreeState::ParameterLayout
    createParameterLayout();

    juce::AudioProcessorValueTreeState apvts;

private:

    //==========================================================================
    // COMPONENT VALUES
    //==========================================================================
    struct Components
    {
        // Resistors
        static constexpr float R1  = 1'000'000.0f;
        static constexpr float R2  = 470'000.0f;
        static constexpr float R3  = 10'000.0f;
        static constexpr float R4  = 47'000.0f;
        static constexpr float R5  = 82'000.0f;
        static constexpr float R6  = 56'000.0f;
        static constexpr float R7  = 33'000.0f;
        static constexpr float R8  = 22'000.0f;
        static constexpr float R9  = 20'000.0f;
        static constexpr float R10 = 4'700.0f;
        static constexpr float R11 = 2'000.0f;
        static constexpr float R12 = 1'500.0f;
        static constexpr float R13 = 10.0f;

        // Capacitors
        static constexpr float C1 = 22.0e-9f;
        static constexpr float C2 = 100.0e-12f;
        static constexpr float C3 = 4.7e-9f;
        static constexpr float C4 = 2.2e-9f;
        static constexpr float C5 = 1.0e-9f;
        static constexpr float C6 = 220.0e-9f;
        static constexpr float C7 = 100.0e-9f;
        static constexpr float C8 = 47.0e-9f;
        static constexpr float C9 = 10.0e-9f;

        // Pots
        static constexpr float GAIN_POT   = 1'000'000.0f;
        static constexpr float BASS_POT   = 100'000.0f;
        static constexpr float MID_POT    = 100'000.0f;
        static constexpr float TREBLE_POT = 50'000.0f;
        static constexpr float VOLUME_POT = 50'000.0f;
    };

    //==========================================================================
    // TL072 MODEL
    //==========================================================================

    struct TL072
    {
        // Reference electrical characteristics
        static constexpr float GBW_HZ = 3.0e6f;
        static constexpr float SLEW_V_PER_US = 16.0f;
        static constexpr float INPUT_NOISE = 15.0e-9f;
        static constexpr float INPUT_RESISTANCE = 1.0e12f;

        // Practical DSP rail
        static constexpr float RAIL = 12.0f;

        float output = 0.0f;

        void reset();

        float process (
            float target,
            float sampleRate,
            float slewMultiplier);
    };

    //==========================================================================
    // FIRST ORDER FILTER
    //==========================================================================

    struct Filter
    {
        float state = 0.0f;

        void reset();

        float lowpass (
            float input,
            float cutoff,
            float sampleRate);

        float highpass (
            float input,
            float cutoff,
            float sampleRate);
    };

    //==========================================================================
    // SIX OP-AMP SECTIONS
    //==========================================================================

    TL072 U1A;
    TL072 U1B;

    TL072 U2A;
    TL072 U2B;

    TL072 U3A;
    TL072 U3B;

    //==========================================================================
    // FILTERS
    //==========================================================================

    Filter inputHP;
    Filter inputLP;

    Filter stage1HP;
    Filter stage1LP;

    Filter stage2HP;
    Filter stage2LP;

    Filter bassLP;
    Filter midLP;
    Filter trebleLP;

    Filter outputHP;

    //==========================================================================
    // PARAMETERS
    //==========================================================================

    std::atomic<float>* gainParameter = nullptr;
    std::atomic<float>* bassParameter = nullptr;
    std::atomic<float>* midParameter = nullptr;
    std::atomic<float>* trebleParameter = nullptr;
    std::atomic<float>* volumeParameter = nullptr;
    std::atomic<float>* aggressionParameter = nullptr;
    std::atomic<float>* bypassParameter = nullptr;

    //==========================================================================
    double sampleRate = 44100.0;

    //==========================================================================
    // DSP FUNCTIONS
    //==========================================================================

    float opAmpStage (
        TL072& opAmp,
        float input,
        float gain,
        float slewMultiplier);

    float ledClip (
        float input,
        float drive,
        int aggression);

    float aggressionStage (
        float input,
        int aggression);

    float toneStack (
        float input,
        float bass,
        float mid,
        float treble);

    void resetDSP();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (
        RG_G4AudioProcessor)
};
