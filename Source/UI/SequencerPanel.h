#pragma once

#include <JuceHeader.h>
#include "SequenceRowComponent.h"

class SequencerPanel : public juce::Component
{
public:
    SequencerPanel();

    void resized() override;

    void setSelectedSequence (int index);
    std::function<void (int)> onSelectionChanged;

private:
    static constexpr int numRows = 6;

    std::vector<std::unique_ptr<SequenceRowComponent>> rows;
    int selected = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SequencerPanel)
};
