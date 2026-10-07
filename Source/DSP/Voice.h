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
        float cutoff   = 1000.0f;
        float volumePct = 80.0f;
    };

    void prepare (double sampleRate, int maxBlockSize);
    void reset();
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

    double sampleRate = 44100.0;
    int maxBlockSize = 512;

    WaveOsc osc1, osc2;
    juce::ADSR adsr1, adsr2;
    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::AudioBuffer<float> scratch1, scratch2;

    float mixGain1 = 1.0f, mixGain2 = 1.0f, volumeGain = 0.8f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Voice)
};
