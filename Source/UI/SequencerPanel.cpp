#include "SequencerPanel.h"

SequencerPanel::SequencerPanel (juce::AudioProcessorValueTreeState& apvts)
{
    for (int i = 0; i < numRows; ++i)
    {
        auto row = std::make_unique<SequenceRowComponent> (i, apvts);
        row->onSelected = [this, i]
        {
            setSelectedSequence (i);
            if (onSelectionChanged)
                onSelectionChanged (selected);
        };
        row->onCopy = [this, i]
        {
            clipboard = rows[(size_t) i]->getPattern();
        };
        row->onPaste = [this, i]
        {
            rows[(size_t) i]->setPattern ((int) clipboard);
        };
        addAndMakeVisible (*row);
        rows.push_back (std::move (row));
    }

    rows[0]->setSelected (true);
}

SequencerPanel::~SequencerPanel()
{
    if (stepsTree.isValid())
        stepsTree.removeListener (this);
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

void SequencerPanel::setStepsTree (juce::ValueTree tree)
{
    if (stepsTree.isValid())
        stepsTree.removeListener (this);

    stepsTree = tree;

    if (stepsTree.isValid())
    {
        stepsTree.addListener (this);

        for (auto& row : rows)
            row->loadPatternFromTree();
    }
}

void SequencerPanel::setPlayhead (int sequenceIndex, int step)
{
    if (auto* row = getRow (sequenceIndex))
        row->setPlayheadStep (step);
}

SequenceRowComponent* SequencerPanel::getRow (int index)
{
    return juce::isPositiveAndBelow (index, (int) rows.size()) ? rows[(size_t) index].get()
                                                               : nullptr;
}

void SequencerPanel::valueTreePropertyChanged (juce::ValueTree&,
                                               const juce::Identifier& property)
{
    const auto name = property.toString();

    if (name.startsWithChar ('p'))
    {
        const int index = name.substring (1).getIntValue();

        if (auto* row = getRow (index))
            row->loadPatternFromTree();
    }
}
