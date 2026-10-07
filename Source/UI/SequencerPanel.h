#pragma once

#include <JuceHeader.h>
#include "SequenceRowComponent.h"

class SequencerPanel : public juce::Component,
                       private juce::ValueTree::Listener
{
public:
    explicit SequencerPanel (juce::AudioProcessorValueTreeState& apvts);
    ~SequencerPanel() override;

    void resized() override;

    void setSelectedSequence (int index);
    void setStepsTree (juce::ValueTree tree);
    void setPlayhead (int sequenceIndex, int step);
    SequenceRowComponent* getRow (int index);

    std::function<void (int)> onSelectionChanged;

private:
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged,
                                   const juce::Identifier& property) override;

    static constexpr int numRows = 6;

    juce::ValueTree stepsTree;
    juce::var clipboard { 0 };
    std::vector<std::unique_ptr<SequenceRowComponent>> rows;
    int selected = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SequencerPanel)
};
