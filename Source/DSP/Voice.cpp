#include "Voice.h"

double Voice::frequencyFor (int octave, int note) noexcept
{
    // Base C3; octave lever -2..+2, note knob 0..11 semitones.
    return 130.81 * std::exp2 ((double) octave + (double) note / 12.0);
}

void Voice::prepare (double sr, int maxBlock)
{
    sampleRate = sr;
    maxBlockSize = juce::jmax (1, maxBlock);

    scratch1.setSize (1, maxBlockSize);
    scratch2.setSize (1, maxBlockSize);
    scratch1.clear();
    scratch2.clear();

    adsr1.setSampleRate (sampleRate);
    adsr2.setSampleRate (sampleRate);
    adsr1.reset();
    adsr2.reset();

    filter.prepare ({ sampleRate, (juce::uint32) maxBlockSize, 1u });
    filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    filter.setCutoffFrequency (1000.0f);
    filter.reset();

    osc1 = WaveOsc();
    osc2 = WaveOsc();
}

void Voice::reset()
{
    adsr1.reset();
    adsr2.reset();
    osc1.phase = 0.0;
    osc2.phase = 0.0;
}

void Voice::setParams (const Params& p)
{
    osc1.setWave (p.wave1);
    osc1.setFrequency (frequencyFor (p.octave1, p.note1), sampleRate);

    osc2.setWave (p.wave2);
    osc2.setFrequency (frequencyFor (p.octave2, p.note2), sampleRate);

    adsr1.setParameters ({ p.attack1 / 1000.0f, p.decay1 / 1000.0f,
                           juce::jlimit (0.0f, 1.0f, p.sustain1 / 100.0f),
                           p.release1 / 1000.0f });
    adsr2.setParameters ({ p.attack2 / 1000.0f, p.decay2 / 1000.0f,
                           juce::jlimit (0.0f, 1.0f, p.sustain2 / 100.0f),
                           p.release2 / 1000.0f });

    mixGain1  = juce::Decibels::decibelsToGain (p.mixDb1);
    mixGain2  = juce::Decibels::decibelsToGain (p.mixDb2);
    volumeGain = juce::jlimit (0.0f, 1.0f, p.volumePct / 100.0f);

    filter.setType (p.useLpf ? juce::dsp::StateVariableTPTFilterType::lowpass
                             : juce::dsp::StateVariableTPTFilterType::highpass);
    filter.setCutoffFrequency (juce::jlimit (20.0f, (float) (sampleRate * 0.49), p.cutoff));
}

void Voice::noteOn()
{
    adsr1.noteOn();
    adsr2.noteOn();
}

void Voice::noteOff()
{
    adsr1.noteOff();
    adsr2.noteOff();
}

void Voice::render (juce::AudioBuffer<float>& out, int startSample, int numSamples)
{
    while (numSamples > 0)
    {
        const int chunk = juce::jmin (numSamples, maxBlockSize);
        renderChunk (out, startSample, chunk);
        startSample += chunk;
        numSamples -= chunk;
    }
}

void Voice::renderChunk (juce::AudioBuffer<float>& out, int startSample, int numSamples)
{
    auto* s1 = scratch1.getWritePointer (0);
    auto* s2 = scratch2.getWritePointer (0);

    for (int i = 0; i < numSamples; ++i)
        s1[i] = osc1.next();

    adsr1.applyEnvelopeToBuffer (scratch1, 0, numSamples);
    scratch1.applyGain (mixGain1, 0, numSamples);

    for (int i = 0; i < numSamples; ++i)
        s2[i] = osc2.next();

    adsr2.applyEnvelopeToBuffer (scratch2, 0, numSamples);
    scratch2.applyGain (mixGain2, 0, numSamples);

    scratch1.addFrom (0, 0, scratch2, 0, 0, numSamples);

    juce::dsp::AudioBlock<float> block (scratch1);
    auto subBlock = block.getSubBlock (0, (size_t) numSamples);
    juce::dsp::ProcessContextReplacing<float> context (subBlock);
    filter.process (context);

    scratch1.applyGain (volumeGain, 0, numSamples);

    const int channels = juce::jmin (2, out.getNumChannels());
    for (int ch = 0; ch < channels; ++ch)
        out.addFrom (ch, startSample, scratch1, 0, 0, numSamples);
}
