#pragma once

#include <JuceHeader.h>

class SequenceRowComponent : public juce::Component
{
public:
    SequenceRowComponent (int sequenceIndex, juce::AudioProcessorValueTreeState& apvts);
    ~SequenceRowComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void setSelected (bool nowSelected);

    void setStepsTree (juce::ValueTree tree);
    void loadPatternFromTree();
    int  getPattern() const;
    void setPattern (int pattern);

    juce::Slider& getLengthKnob()  { return lengthKnob; }
    juce::Slider& getVolumeKnob()  { return volumeKnob; }

    std::function<void()> onSelected;
    std::function<void()> onCopy;
    std::function<void()> onPaste;

    static constexpr int numSteps = 32;

private:
    struct RowMouseListener : juce::MouseListener
    {
        explicit RowMouseListener (SequenceRowComponent& o) : owner (o) {}
        void mouseDown (const juce::MouseEvent&) override { owner.rowClicked(); }
        SequenceRowComponent& owner;
    };

    void rowClicked();
    void savePattern();

    int seqIndex = 0;
    bool selected = false;
    RowMouseListener mouseListener { *this };
    juce::ValueTree stepsTree;

    juce::TextButton copyButton { "Copy" }, pasteButton { "Paste" };
    std::vector<std::unique_ptr<juce::TextButton>> stepButtons;
    juce::Slider lengthKnob, volumeKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lengthAttachment, volumeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SequenceRowComponent)
};
