#include "SequenceRowComponent.h"
#include "../PluginProcessor.h"
#include "Skin.h"

SequenceRowComponent::SequenceRowComponent (int sequenceIndex, juce::AudioProcessorValueTreeState& apvts)
    : seqIndex (sequenceIndex)
{
    addAndMakeVisible (copyButton);
    addAndMakeVisible (pasteButton);
    addAndMakeVisible (arpButton);

    copyButton.setTooltip ("Copy pattern (Seq " + juce::String (seqIndex + 1) + ")");
    pasteButton.setTooltip ("Paste pattern (Seq " + juce::String (seqIndex + 1) + ")");
    arpButton.setTooltip ("Toggle Arpeggiator (Seq " + juce::String (seqIndex + 1) + ")");

    copyButton.onClick = [this] { if (onCopy) onCopy(); };
    pasteButton.onClick = [this] { if (onPaste) onPaste(); };
    arpButton.setClickingTogglesState (true);
    arpButton.onClick = [this]
    {
        if (onArpToggled)
            onArpToggled (arpButton.getToggleState());
    };

    for (int i = 0; i < numSteps; ++i)
    {
        auto step = std::make_unique<juce::TextButton> ("step " + juce::String (i + 1));
        step->setClickingTogglesState (true);
        step->onClick = [this] { savePattern(); };
        addAndMakeVisible (*step);
        stepButtons.push_back (std::move (step));
    }

    auto setupKnob = [this] (juce::Slider& knob, const juce::String& name, const juce::String& tip)
    {
        knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        knob.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
        knob.setName (name);
        knob.setTooltip (tip);
        addAndMakeVisible (knob);
    };

    setupKnob (lengthKnob, "length", "Length (Seq " + juce::String (seqIndex + 1) + ")");
    setupKnob (volumeKnob, "volume", "Volume (Seq " + juce::String (seqIndex + 1) + ")");

    addAndMakeVisible (playhead);
    playhead.setInterceptsMouseClicks (false, false);
    playhead.setVisible (false);

    lengthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, MetuliferAudioProcessor::seqParamId (seqIndex, "length"), lengthKnob);
    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, MetuliferAudioProcessor::seqParamId (seqIndex, "volume"), volumeKnob);
    arpAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, MetuliferAudioProcessor::seqParamId (seqIndex, "arp_enabled"), arpButton);

    addMouseListener (&mouseListener, true);
}

SequenceRowComponent::~SequenceRowComponent()
{
    removeMouseListener (&mouseListener);
}

void SequenceRowComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (0.5f);

    const auto cardColour = selected ? juce::Colours::white : juce::Colour (0xfff7f8fa);
    Skin::drawPanel (g, "panel_row", bounds, cardColour, 5.0f);

    if (selected)
    {
        // Highlight outline
        g.setColour (juce::Colour (0xff2563eb));
        g.drawRoundedRectangle (bounds, 5.0f, 1.5f);

        // Left accent bar
        juce::Path accent;
        accent.addRoundedRectangle (bounds.getX(), bounds.getY(), 4.0f, bounds.getHeight(),
                                    5.0f, 5.0f, true, false, true, false);
        g.fillPath (accent);
    }

    // Sequence badge: S1 .. S6
    const float badgeW = 22.0f;
    const float badgeH = 22.0f;
    auto badgeArea = juce::Rectangle<float> (bounds.getX() + 8.0f,
                                             bounds.getY() + (bounds.getHeight() - badgeH) * 0.5f,
                                             badgeW, badgeH);

    g.setColour (selected ? juce::Colour (0xff2563eb) : juce::Colour (0xff333842));
    g.fillRoundedRectangle (badgeArea, 4.0f);

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (10.5f, juce::Font::bold));
    g.drawText ("S" + juce::String (seqIndex + 1), badgeArea, juce::Justification::centred);
}

void SequenceRowComponent::resized()
{
    const float pad = 5.0f;
    const float gap = 3.0f;
    auto area = getLocalBounds().toFloat().reduced (pad);

    // Left area: Badge space (30) + C (24) + P (24) + ARP (28) = 112px
    auto leftArea = area.removeFromLeft (116.0f);
    leftArea.removeFromLeft (30.0f); // Space for S1..S6 badge

    const float btnSize = 24.0f;
    const float btnY = (getHeight() - btnSize) * 0.5f;

    auto copyArea = leftArea.removeFromLeft (btnSize);
    copyButton.setBounds (juce::roundToInt (copyArea.getX()), juce::roundToInt (btnY),
                          juce::roundToInt (btnSize), juce::roundToInt (btnSize));

    leftArea.removeFromLeft (3.0f);
    auto pasteArea = leftArea.removeFromLeft (btnSize);
    pasteButton.setBounds (juce::roundToInt (pasteArea.getX()), juce::roundToInt (btnY),
                           juce::roundToInt (btnSize), juce::roundToInt (btnSize));

    leftArea.removeFromLeft (3.0f);
    auto arpArea = leftArea.removeFromLeft (30.0f);
    arpButton.setBounds (juce::roundToInt (arpArea.getX()), juce::roundToInt (btnY),
                         30, juce::roundToInt (btnSize));

    area.removeFromLeft (gap);

    // Right area: Knobs
    const float knobSize = 48.0f;
    const float knobY = (getHeight() - knobSize) * 0.5f;

    auto volumeArea = area.removeFromRight (knobSize);
    volumeKnob.setBounds (juce::roundToInt (volumeArea.getX()), juce::roundToInt (knobY),
                          juce::roundToInt (knobSize), juce::roundToInt (knobSize));

    area.removeFromRight (gap);

    auto lengthArea = area.removeFromRight (knobSize);
    lengthKnob.setBounds (juce::roundToInt (lengthArea.getX()), juce::roundToInt (knobY),
                          juce::roundToInt (knobSize), juce::roundToInt (knobSize));

    area.removeFromRight (gap);

    // Middle area: 32 Steps
    const int n = (int) stepButtons.size();
    const float stepW = juce::jmin (26.0f, (area.getWidth() - gap * (float) (n - 1)) / (float) n);
    const float startX = area.getX() + (area.getWidth() - (stepW * n + gap * (float) (n - 1))) * 0.5f;
    const float y = (getHeight() - stepW) * 0.5f;

    for (int i = 0; i < n; ++i)
        stepButtons[(size_t) i]->setBounds (juce::roundToInt (startX + i * (stepW + gap)),
                                            juce::roundToInt (y),
                                            juce::roundToInt (stepW),
                                            juce::roundToInt (stepW));

    placePlayhead();
}

void SequenceRowComponent::setArpEnabled (bool on)
{
    arpButton.setToggleState (on, juce::dontSendNotification);
}

void SequenceRowComponent::setPlayheadStep (int step)
{
    if (step == playheadStep)
        return;

    playheadStep = step;
    placePlayhead();
}

void SequenceRowComponent::placePlayhead()
{
    if (! juce::isPositiveAndBelow (playheadStep, (int) stepButtons.size()))
    {
        playhead.setVisible (false);
        return;
    }

    playhead.setBounds (stepButtons[(size_t) playheadStep]->getBounds());
    playhead.setVisible (true);
    playhead.toFront (false);
}

void SequenceRowComponent::setSelected (bool nowSelected)
{
    if (selected == nowSelected)
        return;

    selected = nowSelected;
    repaint();
}

void SequenceRowComponent::rowClicked()
{
    if (onSelected)
        onSelected();
}

void SequenceRowComponent::setStepsTree (juce::ValueTree tree)
{
    stepsTree = tree;
    loadPatternFromTree();
}

int SequenceRowComponent::getPattern() const
{
    int pattern = 0;

    for (int i = 0; i < numSteps; ++i)
        if (stepButtons[(size_t) i]->getToggleState())
            pattern |= (1 << i);

    return pattern;
}

void SequenceRowComponent::setPattern (int pattern)
{
    for (int i = 0; i < numSteps; ++i)
        stepButtons[(size_t) i]->setToggleState ((pattern & (1 << i)) != 0,
                                                  juce::dontSendNotification);

    savePattern();
}

void SequenceRowComponent::loadPatternFromTree()
{
    if (! stepsTree.isValid())
        return;

    const int pattern = (int) stepsTree.getProperty ("p" + juce::String (seqIndex), 0);

    for (int i = 0; i < numSteps; ++i)
        stepButtons[(size_t) i]->setToggleState ((pattern & (1 << i)) != 0,
                                                  juce::dontSendNotification);
}

void SequenceRowComponent::savePattern()
{
    if (stepsTree.isValid())
        stepsTree.setProperty ("p" + juce::String (seqIndex), getPattern(), nullptr);
}
