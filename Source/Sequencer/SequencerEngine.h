
#pragma once

#include <JuceHeader.h>
#include <vector>

class SequencerEngine
{
public:
    static constexpr int numSequences = 6;

    SequencerEngine();

    struct StepEvent
    {
        int sampleOffset;
        int sequence;
        bool gateOn;
    };

    struct Sources
    {
        const std::atomic<int>* patterns = nullptr;
        std::atomic<float>* lengths[numSequences] {};
        std::atomic<float>* tempo = nullptr;
    };

    void prepare (double sampleRate);
    void setSources (const Sources& s);

    void setPlaying (bool shouldPlay);
    bool isPlaying() const noexcept { return playing.load (std::memory_order_relaxed); }

    /** Synchronizes transport, tempo and beat phase to DAW timeline. */
    void syncToHost (double ppqPosition, double bpm, bool isHostPlaying);

    /** True once when playback was stopped: the audio thread must release all voices. */
    bool takePendingNoteOff();

    /** Advances the independent per-sequence counters; returns gate events for this block. */
    const std::vector<StepEvent>& process (int numSamples);

    /** Current step of a sequence, -1 when stopped. */
    int getCurrentStep (int sequence) const;

private:
    int lengthOf (int sequence) const;
    bool gateOf (int sequence, int step) const;
    double samplesPerStep() const;

    Sources sources;
    double sampleRate = 44100.0;
    std::atomic<float> hostBpm { 0.0f };
    double masterAccumulator = 0.0;
    int64_t globalStep = 0;
    int steps[numSequences] {};
    std::atomic<bool> playing { false };
    std::atomic<bool> pendingStart { false };
    std::atomic<bool> pendingNoteOff { false };
    std::atomic<int> currentSteps[numSequences];
    std::vector<StepEvent> events;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SequencerEngine)
};
