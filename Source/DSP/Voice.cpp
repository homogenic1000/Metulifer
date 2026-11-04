#include "Voice.h"
#include <cmath>

static int buildNotePool (int chordIndex, int numOctaves, int* outPool)
{
    static const int chordIntervals[10][14] = {
        { 4, 0, 4, 7, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0 },                   // 0: Major
        { 4, 0, 3, 7, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0 },                   // 1: Minor
        { 6, 0, 3, 5, 7, 10, 12, 0, 0, 0, 0, 0, 0, 0 },                  // 2: Pentatonic
        { 3, 0, 12, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },                  // 3: Octaves
        { 5, 0, 4, 7, 11, 12, 0, 0, 0, 0, 0, 0, 0, 0 },                  // 4: Maj7
        { 5, 0, 3, 7, 10, 12, 0, 0, 0, 0, 0, 0, 0, 0 },                  // 5: Min7
        { 5, 0, 4, 7, 10, 12, 0, 0, 0, 0, 0, 0, 0, 0 },                  // 6: Dom7
        { 4, 0, 5, 7, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0 },                   // 7: Sus4
        { 4, 0, 7, 12, 19, 0, 0, 0, 0, 0, 0, 0, 0, 0 },                  // 8: Fifths
        { 13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 }                // 9: Chromatic
    };

    chordIndex = juce::jlimit (0, 9, chordIndex);
    const int count = chordIntervals[chordIndex][0];
    const int* intervals = &chordIntervals[chordIndex][1];

    int total = 0;
    numOctaves = juce::jlimit (1, 3, numOctaves);

    for (int oct = 0; oct < numOctaves; ++oct)
    {
        for (int i = 0; i < count; ++i)
        {
            if (oct > 0 && i == 0)
                continue;
            outPool[total++] = intervals[i] + oct * 12;
        }
    }
    return total;
}

double Voice::frequencyFor (int octave, int note) noexcept
{
    // Base C3 (130.81 Hz); octave lever -2..+2, note knob 0..11 semitones.
    return 130.81 * std::exp2 ((double) octave + (double) note / 12.0);
}

void Voice::prepare (double sr, int maxBlock)
{
    sampleRate = sr;
    maxBlockSize = juce::jmax (1, maxBlock);

    scratch1.setSize (1, maxBlockSize);
    scratch2.setSize (1, maxBlockSize);
    scratchStereo.setSize (2, maxBlockSize);
    scratch1.clear();
    scratch2.clear();
    scratchStereo.clear();

    const int delayBufferSize = (int) (sampleRate * 2.5);
    delayBuffer.setSize (2, delayBufferSize);
    delayBuffer.clear();
    delayWritePos = 0;

    reduxCounter = 0;
    reduxHeldSample = 0.0f;

    arpStep = 0;
    currentArpOffset = 0;

    adsr1.setSampleRate (sampleRate);
    adsr2.setSampleRate (sampleRate);
    adsr1.reset();
    adsr2.reset();

    filter.prepare ({ sampleRate, (juce::uint32) maxBlockSize, 1u });
    filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    filter.setCutoffFrequency (1000.0f);
    filter.reset();

    reverb.prepare ({ sampleRate, (juce::uint32) maxBlockSize, 2u });
    reverb.reset();

    osc1 = WaveOsc();
    osc2 = WaveOsc();
}

void Voice::reset()
{
    adsr1.reset();
    adsr2.reset();
    reverb.reset();
    delayBuffer.clear();
    delayWritePos = 0;
    reduxCounter = 0;
    reduxHeldSample = 0.0f;
    arpStep = 0;
    currentArpOffset = 0;
    osc1.phase = 0.0;
    osc2.phase = 0.0;
}

void Voice::resetArp()
{
    arpStep = 0;
    currentArpOffset = 0;
    updateFrequencies();
}

void Voice::setParams (const Params& p)
{
    currentParams = p;

    osc1.setWave (p.wave1);
    osc2.setWave (p.wave2);
    updateFrequencies();

    adsr1.setParameters ({ p.attack1 / 1000.0f, p.decay1 / 1000.0f,
                           juce::jlimit (0.0f, 1.0f, p.sustain1 / 100.0f),
                           p.release1 / 1000.0f });
    adsr2.setParameters ({ p.attack2 / 1000.0f, p.decay2 / 1000.0f,
                           juce::jlimit (0.0f, 1.0f, p.sustain2 / 100.0f),
                           p.release2 / 1000.0f });

    mixGain1   = (p.mixDb1 <= -59.0f) ? 0.0f : juce::Decibels::decibelsToGain (p.mixDb1);
    mixGain2   = (p.mixDb2 <= -59.0f) ? 0.0f : juce::Decibels::decibelsToGain (p.mixDb2);
    volumeGain = juce::jlimit (0.0f, 1.0f, p.volumePct / 100.0f);

    filter.setType (p.useLpf ? juce::dsp::StateVariableTPTFilterType::lowpass
                             : juce::dsp::StateVariableTPTFilterType::highpass);
    filter.setCutoffFrequency (juce::jlimit (20.0f, (float) (sampleRate * 0.49), p.cutoff));

    reverbActive = p.reverbEnabled && p.reverbMix > 0.001f;
    if (reverbActive)
    {
        juce::dsp::Reverb::Parameters rp;
        const float wet = juce::jlimit (0.0f, 1.0f, p.reverbMix / 100.0f);
        if (p.reverbMode == 0) // Room
        {
            rp.roomSize = 0.45f;
            rp.damping  = 0.55f;
            rp.width    = 0.8f;
        }
        else // Hall
        {
            rp.roomSize = 0.85f;
            rp.damping  = 0.25f;
            rp.width    = 1.0f;
        }
        rp.wetLevel = wet * 0.35f;
        rp.dryLevel = 1.0f - (wet * 0.5f);
        rp.freezeMode = 0.0f;
        reverb.setParameters (rp);
    }
}

void Voice::updateFrequencies()
{
    osc1.setFrequency (frequencyFor (currentParams.octave1, currentParams.note1 + currentArpOffset), sampleRate);
    osc2.setFrequency (frequencyFor (currentParams.octave2, currentParams.note2 + currentArpOffset), sampleRate);
}

void Voice::advanceArp()
{
    int pool[64];
    const int N = buildNotePool (currentParams.arpChord, currentParams.arpOctaves, pool);

    if (N <= 0)
    {
        currentArpOffset = 0;
        updateFrequencies();
        return;
    }

    int idx = 0;
    const int mode = juce::jlimit (0, 10, currentParams.arpMode);

    switch (mode)
    {
        case 0: // Up
            idx = arpStep % N;
            break;

        case 1: // Down
            idx = (N - 1) - (arpStep % N);
            break;

        case 2: // Up & Down
        {
            const int cycle = (N > 1) ? (2 * N - 2) : 1;
            const int p = arpStep % cycle;
            idx = (p < N) ? p : (2 * N - 2 - p);
            break;
        }

        case 3: // Down & Up
        {
            const int cycle = (N > 1) ? (2 * N - 2) : 1;
            const int p = arpStep % cycle;
            idx = (p < N) ? (N - 1 - p) : (p - N + 1);
            break;
        }

        case 4: // Pinky Up
        {
            if (N <= 1)
            {
                idx = 0;
            }
            else
            {
                const int cycle = 2 * (N - 1);
                const int p = arpStep % cycle;
                idx = (p % 2 == 0) ? 0 : (1 + (p / 2));
            }
            break;
        }

        case 5: // Pinky Down
        {
            if (N <= 1)
            {
                idx = 0;
            }
            else
            {
                const int cycle = 2 * (N - 1);
                const int p = arpStep % cycle;
                idx = (p % 2 == 0) ? (N - 1) : ((N - 2) - (p / 2));
            }
            break;
        }

        case 6: // Thumb-Pinky
        {
            const int p = arpStep % N;
            idx = (p % 2 == 0) ? (p / 2) : (N - 1 - p / 2);
            break;
        }

        case 7: // Converge
        {
            const int p = arpStep % N;
            idx = (p % 2 == 0) ? (p / 2) : (N - 1 - p / 2);
            break;
        }

        case 8: // Diverge
        {
            const int p = arpStep % N;
            const int mid = N / 2;
            idx = (p % 2 == 0) ? (mid + p / 2) : (mid - 1 - p / 2);
            idx = juce::jlimit (0, N - 1, idx);
            break;
        }

        case 9: // Random
        {
            idx = juce::Random::getSystemRandom().nextInt (N);
            break;
        }

        case 10: // Order
        default:
            idx = arpStep % N;
            break;
    }

    currentArpOffset = pool[juce::jlimit (0, N - 1, idx)];
    arpStep++;
    updateFrequencies();
}

void Voice::noteOn()
{
    if (currentParams.arpEnabled)
    {
        advanceArp();
    }
    else
    {
        currentArpOffset = 0;
        updateFrequencies();
    }

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
    scratch1.applyGain (0, numSamples, mixGain1);

    for (int i = 0; i < numSamples; ++i)
        s2[i] = osc2.next();

    adsr2.applyEnvelopeToBuffer (scratch2, 0, numSamples);
    scratch2.applyGain (0, numSamples, mixGain2);

    scratch1.addFrom (0, 0, scratch2, 0, 0, numSamples);
    scratch1.applyGain (0, numSamples, 0.5f);

    juce::dsp::AudioBlock<float> block (scratch1);
    auto subBlock = block.getSubBlock (0, (size_t) numSamples);
    juce::dsp::ProcessContextReplacing<float> context (subBlock);
    filter.process (context);

    if (volumeGain <= 0.0001f)
    {
        if (reverbActive)
            reverb.reset();
        return;
    }

    scratch1.applyGain (0, numSamples, volumeGain);

    // 1. Redux (Bitcrusher + Downsampler)
    if (currentParams.reduxEnabled && currentParams.reduxMix > 0.001f)
    {
        auto* data = scratch1.getWritePointer (0);
        const float wet = juce::jlimit (0.0f, 1.0f, currentParams.reduxMix / 100.0f);
        const int rate = juce::jlimit (1, 32, currentParams.reduxRate);
        const int bits = juce::jlimit (2, 16, currentParams.reduxBits);
        const float steps = (float) (1 << (bits - 1));

        for (int i = 0; i < numSamples; ++i)
        {
            const float in = data[i];
            if (++reduxCounter >= rate)
            {
                reduxCounter = 0;
                float q = in;
                if (bits < 16)
                    q = std::round (in * steps) / steps;
                reduxHeldSample = q;
            }
            data[i] = (1.0f - wet) * in + wet * reduxHeldSample;
        }
    }

    // Prepare stereo scratch buffer
    auto* st0 = scratchStereo.getWritePointer (0);
    auto* st1 = scratchStereo.getWritePointer (1);
    auto* mono = scratch1.getReadPointer (0);

    const bool delayActive = currentParams.delayEnabled && currentParams.delayMix > 0.001f;

    // 2. Stereo Delay
    if (delayActive)
    {
        const float wet = juce::jlimit (0.0f, 1.0f, currentParams.delayMix / 100.0f);
        const float fb  = juce::jlimit (0.0f, 0.90f, currentParams.delayFeedback / 100.0f);
        const int delayLen = delayBuffer.getNumSamples();
        const int delaySamplesL = juce::jlimit (1, delayLen - 1,
            juce::roundToInt (currentParams.delayTime * 0.001f * (float) sampleRate));
        const int delaySamplesR = currentParams.delayPingpong
            ? juce::jlimit (1, delayLen - 1, juce::roundToInt ((float) delaySamplesL * 0.75f))
            : delaySamplesL;

        auto* delayL = delayBuffer.getWritePointer (0);
        auto* delayR = delayBuffer.getWritePointer (1);

        for (int i = 0; i < numSamples; ++i)
        {
            const float in = mono[i];

            int rposL = delayWritePos - delaySamplesL;
            if (rposL < 0) rposL += delayLen;

            int rposR = delayWritePos - delaySamplesR;
            if (rposR < 0) rposR += delayLen;

            const float outL = delayL[rposL];
            const float outR = delayR[rposR];

            delayL[delayWritePos] = in + std::tanh (outL * fb);
            delayR[delayWritePos] = in + std::tanh (outR * fb);

            if (++delayWritePos >= delayLen)
                delayWritePos = 0;

            st0[i] = (1.0f - wet) * in + wet * outL;
            st1[i] = (1.0f - wet) * in + wet * outR;
        }
    }
    else
    {
        for (int i = 0; i < numSamples; ++i)
        {
            st0[i] = mono[i];
            st1[i] = mono[i];
        }
    }

    // 3. Reverb
    if (reverbActive)
    {
        juce::dsp::AudioBlock<float> stereoBlock (scratchStereo);
        auto stereoSubBlock = stereoBlock.getSubBlock (0, (size_t) numSamples);
        juce::dsp::ProcessContextReplacing<float> revContext (stereoSubBlock);
        reverb.process (revContext);
    }

    // Sum into output channels
    const int channels = juce::jmin (2, out.getNumChannels());
    for (int ch = 0; ch < channels; ++ch)
        out.addFrom (ch, startSample, scratchStereo, ch, 0, numSamples);
}
