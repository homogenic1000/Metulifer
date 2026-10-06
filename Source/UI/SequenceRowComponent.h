#pragma once

#include <JuceHeader.h>

class SequenceRowComponent : public juce::Component
{
public:
    explicit SequenceRowComponent (int sequenceIndex);
    ~SequenceRowComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void setSelected (bool nowSelected);

    std::function<void()> onSelected;

    static constexpr int numSteps = 32;

private:
    struct RowMouseListener : juce::MouseListener
    {
        explicit RowMouseListener (SequenceRowComponent& o) : owner (o) {}
        void mouseDown (const juce::MouseEvent&) override { owner.rowClicked(); }
        SequenceRowComponent& owner;
    };

    void rowClicked();

    int seqIndex = 0;
    bool selected = false;
    RowMouseListener mouseListener { *this };

    juce::TextButton copyButton { "Copy" }, pasteButton { "Paste" };
    std::vector<std::unique_ptr<juce::TextButton>> stepButtons;
    juce::Slider lengthKnob, volumeKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SequenceRowComponent)
};
