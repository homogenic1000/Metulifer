#include "SequencerPanel.h"
#include "SequenceRowComponent.h"

SequencerPanel::SequencerPanel()
{
    for (int i = 0; i < numRows; ++i)
    {
        auto row = std::make_unique<SequenceRowComponent> (i);
        row->onSelected = [this, i]
        {
            setSelectedSequence (i);
            if (onSelectionChanged)
                onSelectionChanged (selected);
        };
        addAndMakeVisible (*row);
        rows.push_back (std::move (row));
    }

    rows[0]->setSelected (true);
}

void SequencerPanel::resized()
{
    const float rowH = 61.065f;
    const float pitch = 71.065f;

    for (int i = 0; i < (int) rows.size(); ++i)
        rows[(size_t) i]->setBounds (0,
                                     juce::roundToInt ((float) i * pitch),
                                     getWidth(),
                                     juce::roundToInt (rowH));
}

void SequencerPanel::setSelectedSequence (int index)
{
    if (! juce::isPositiveAndBelow (index, (int) rows.size()))
        return;

    selected = index;

    for (int i = 0; i < (int) rows.size(); ++i)
        rows[(size_t) i]->setSelected (i == selected);
}
