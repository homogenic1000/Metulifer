#include "SequencerEngine.h"

#include <algorithm>

SequencerEngine::SequencerEngine()
{
    for (int i = 0; i < numSequences; ++i)
        currentSteps[i].store (-1, std::memory_order_relaxed);

    events.reserve (256);
}

void SequencerEngine::prepare (double sr)
{
    sampleRate = sr;

    for (int i = 0; i < numSequences; ++i)
    {
        accumulators[i] = 0.0;
        steps[i] = 0;

        if (! isPlaying())
            currentSteps[i].store (-1, std::memory_order_relaxed);
    }
}

void SequencerEngine::setSources (const Sources& s)
{
    sources = s;
}

double SequencerEngine::samplesPerStep() const
{
    const float bpm = sources.tempo != nullptr
                        ? juce::jlimit (20.0f, 300.0f, sources.tempo->load (std::memory_order_relaxed))
                        : 120.0f;

    // One step = one 1/16 note.
    return sampleRate * 60.0 / ((double) bpm * 4.0);
}

int SequencerEngine::lengthOf (int sequence) const
{
    const auto* p = sources.lengths[sequence];

    if (p == nullptr)
        return 32;

    return juce::jlimit (1, 32, juce::roundToInt (p->load (std::memory_order_relaxed)));
}

bool SequencerEngine::gateOf (int sequence, int step) const
{
    if (sources.patterns == nullptr)
        return false;

    return (sources.patterns[sequence].load (std::memory_order_relaxed) & (1 << step)) != 0;
}

void SequencerEngine::setPlaying (bool shouldPlay)
{
    if (shouldPlay)
    {
        pendingNoteOff.store (false, std::memory_order_relaxed);
        playing.store (true, std::memory_order_relaxed);
        pendingStart.store (true, std::memory_order_release);
    }
    else
    {
        const bool wasPlaying = playing.exchange (false, std::memory_order_relaxed);
        pendingStart.store (false, std::memory_order_relaxed);

        if (wasPlaying)
            pendingNoteOff.store (true, std::memory_order_release);

        for (int i = 0; i < numSequences; ++i)
            currentSteps[i].store (-1, std::memory_order_relaxed);
    }
}

bool SequencerEngine::takePendingNoteOff()
{
    return pendingNoteOff.exchange (false, std::memory_order_relaxed);
}

int SequencerEngine::getCurrentStep (int sequence) const
{
    return juce::isPositiveAndBelow (sequence, numSequences)
             ? currentSteps[sequence].load (std::memory_order_relaxed)
             : -1;
}

const std::vector<SequencerEngine::StepEvent>& SequencerEngine::process (int numSamples)
{
    events.clear();

    if (! isPlaying())
        return events;

    if (pendingStart.exchange (false, std::memory_order_relaxed))
    {
        for (int s = 0; s < numSequences; ++s)
        {
            steps[s] = 0;
            accumulators[s] = 0.0;
            currentSteps[s].store (0, std::memory_order_relaxed);
            events.push_back ({ 0, s, gateOf (s, 0) });
        }

        if (numSamples <= 1)
            return events;
    }

    const double stepLen = samplesPerStep();

    for (int s = 0; s < numSequences; ++s)
    {
        int remaining = numSamples;
        double acc = accumulators[s];

        while (remaining > 0)
        {
            const double need = stepLen - acc;

            if (need > (double) remaining)
            {
                acc += (double) remaining;
                break;
            }

            const int consumed = juce::jmax (1, (int) std::ceil (need));
            const int offset = numSamples - remaining + consumed - 1;

            int next = steps[s] + 1;
            if (next >= lengthOf (s))
                next = 0;

            steps[s] = next;
            currentSteps[s].store (next, std::memory_order_relaxed);
            events.push_back ({ juce::jlimit (0, numSamples - 1, offset), s, gateOf (s, next) });

            acc += (double) consumed - stepLen;
            remaining -= consumed;
        }

        accumulators[s] = acc;
    }

    // Events arrive per-sequence; the processor expects ascending offsets.
    std::sort (events.begin(), events.end(),
               [] (const StepEvent& a, const StepEvent& b)
               {
                   return a.sampleOffset < b.sampleOffset;
               });

    return events;
}
