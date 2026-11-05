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

    /** Highlights the playing step (-1 hides the marker). */
    void setPlayheadStep (int step);
    void setArpEnabled (bool on);

    juce::Slider& getLengthKnob()  { return lengthKnob; }
    juce::Slider& getVolumeKnob()  { return volumeKnob; }
    juce::TextButton& getArpButton() { return arpButton; }

    std::function<void()> onSelected;
    std::function<void()> onCopy;
    std::function<void()> onPaste;
    std::function<void (bool on)> onArpToggled;

    static constexpr int numSteps = 32;

private:
    struct RowMouseListener : juce::MouseListener
    {
        explicit RowMouseListener (SequenceRowComponent& o) : owner (o) {}
        void mouseDown (const juce::MouseEvent& e) override { owner.handleMouseDown (e); }
        void mouseDrag (const juce::MouseEvent& e) override { owner.handleMouseDrag (e); }
        void mouseUp   (const juce::MouseEvent& e) override { owner.handleMouseUp (e); }
        SequenceRowComponent& owner;
    };

    void handleMouseDown (const juce::MouseEvent& e);
    void handleMouseDrag (const juce::MouseEvent& e);
    void handleMouseUp   (const juce::MouseEvent& e);
    int  getStepIndexAt (juce::Point<int> pos) const;
    int  getStepIndexForDrag (juce::Point<int> pos) const;

    void rowClicked();
    void savePattern();
    void placePlayhead();

    int seqIndex = 0;
    bool selected = false;
    int playheadStep = -1;
    bool isPaintingSteps = false;
    bool paintState = false;
    int lastPaintedStep = -1;

    RowMouseListener mouseListener { *this };
    juce::ValueTree stepsTree;

    class PlayheadMarker : public juce::Component
    {
    public:
        void paint (juce::Graphics& g) override
        {
            auto b = getLocalBounds().toFloat().reduced (0.5f);
            g.setColour (juce::Colour (0x35ffffff));
            g.fillRoundedRectangle (b, 3.5f);
            g.setColour (juce::Colour (0xffff9500));
            g.drawRoundedRectangle (b, 3.5f, 2.0f);
            g.setColour (juce::Colour (0x80ffffff));
            g.drawRoundedRectangle (b.reduced (1.0f), 2.5f, 1.0f);
        }
    };

    PlayheadMarker playhead;

    juce::TextButton muteButton { "M" }, soloButton { "S" };
    juce::TextButton copyButton { "C" }, pasteButton { "P" }, arpButton { "ARP" };
    std::vector<std::unique_ptr<juce::TextButton>> stepButtons;
    juce::Slider lengthKnob, volumeKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lengthAttachment, volumeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> arpAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> muteAttachment, soloAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SequenceRowComponent)
};
