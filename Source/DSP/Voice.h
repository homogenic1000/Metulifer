#pragma once

#include <JuceHeader.h>

class Voice
{
public:
    Voice() = default;

    struct Params
    {
        float attack1  = 5.0f,  decay1  = 200.0f, sustain1  = 70.0f, release1  = 300.0f;
        float attack2  = 5.0f,  decay2  = 200.0f, sustain2  = 70.0f, release2  = 300.0f;
        int   octave1  = 0, note1  = 0, wave1  = 1;
        int   octave2  = 0, note2  = 0, wave2  = 1;
        float mixDb1   = 0.0f, mixDb2 = 0.0f;
        bool  useLpf   = true;
        float cutoff   = 20000.0f;
        float volumePct = 80.0f;

        // Reverb
        bool  reverbEnabled = false;
        int   reverbMode    = 0; // 0: Room, 1: Hall
        float reverbMix     = 30.0f; // 0..100%

        // Delay
        bool  delayEnabled  = false;
        float delayTime     = 250.0f; // 20..1000 ms
        float delayFeedback = 40.0f;  // 0..90%
        float delayMix      = 35.0f;  // 0..100%
        bool  delayPingpong = true;

        // Redux
        bool  reduxEnabled  = false;
        int   reduxBits     = 8;      // 2..16 bits
        int   reduxRate     = 4;      // 1..32 downsample factor
        float reduxMix      = 50.0f;  // 0..100%

        // Arpeggiator
        bool  arpEnabled    = false;
        int   arpMode       = 0;      // 0..10
        int   arpChord      = 1;      // 0..9
        int   arpOctaves    = 1;      // 1..3
    };

    void prepare (double sampleRate, int maxBlockSize);
    void reset();
    void resetArp();
    void setParams (const Params& p);
    void noteOn();
    void noteOff();
    void render (juce::AudioBuffer<float>& out, int startSample, int numSamples);

private:
    struct WaveOsc
    {
        void setFrequency (double hz, double sr) noexcept { increment = hz / sr; }
        void setWave (int w) noexcept                    { wave = w; }

        float next() noexcept
        {
            float value = 0.0f;

            switch (wave)
            {
                case 0:
                {
                    value = (float) std::sin (phase * juce::MathConstants<double>::twoPi);
                    break;
                }
                case 1:
                {
                    value = (float) (2.0 * phase - 1.0) - polyBlep ((float) phase, (float) increment);
                    break;
                }
                case 2:
                {
                    value = phase < 0.5 ? 1.0f : -1.0f;
                    value += polyBlep ((float) phase, (float) increment);

                    double second = phase + 0.5;
                    if (second >= 1.0)
                        second -= 1.0;

                    value -= polyBlep ((float) second, (float) increment);
                    break;
                }
                case 3:
                {
                    value = 1.0f - 4.0f * std::abs ((float) phase - 0.5f);
                    break;
                }
                default:
                    break;
            }

            phase += increment;
            if (phase >= 1.0)
                phase -= 1.0;

            return value;
        }

        static float polyBlep (float t, float dt) noexcept
        {
            if (dt <= 0.0f)
                return 0.0f;

            if (t < dt)
            {
                t /= dt;
                return t + t - t * t - 1.0f;
            }

            if (t > 1.0f - dt)
            {
                t = (t - 1.0f) / dt;
                return t * t + t + t + 1.0f;
            }

            return 0.0f;
        }

        double phase = 0.0;
        double increment = 0.0;
        int wave = 0;
    };

    static double frequencyFor (int octave, int note) noexcept;
    void renderChunk (juce::AudioBuffer<float>& out, int startSample, int numSamples);
    void advanceArp();
    void updateFrequencies();

    double sampleRate = 44100.0;
    int maxBlockSize = 512;

    Params currentParams;

    WaveOsc osc1, osc2;
    juce::ADSR adsr1, adsr2;
    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::dsp::Reverb reverb;
    juce::AudioBuffer<float> scratch1, scratch2, scratchStereo;

    // Delay state
    juce::AudioBuffer<float> delayBuffer;
    int delayWritePos = 0;

    // Redux state
    int reduxCounter = 0;
    float reduxHeldSample = 0.0f;

    // Arp state
    int arpStep = 0;
    int currentArpOffset = 0;

    float mixGain1 = 1.0f, mixGain2 = 1.0f, volumeGain = 0.8f;
    bool reverbActive = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Voice)
};
