#include "SequenceRowComponent.h"

SequenceRowComponent::SequenceRowComponent (int sequenceIndex)
    : seqIndex (sequenceIndex)
{
    addAndMakeVisible (copyButton);
    addAndMakeVisible (pasteButton);

    for (int i = 0; i < numSteps; ++i)
    {
        auto step = std::make_unique<juce::TextButton> ("step " + juce::String (i + 1));
        step->setClickingTogglesState (true);
        step->onClick = [this] { rowClicked(); };
        addAndMakeVisible (*step);
        stepButtons.push_back (std::move (step));
    }

    auto setupKnob = [this] (juce::Slider& knob, const juce::String& name)
    {
        knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        knob.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
        knob.setName (name);
        addAndMakeVisible (knob);
    };

    setupKnob (lengthKnob, "Length");
    setupKnob (volumeKnob, "Volume");
    lengthKnob.setRange (0.0, 32.0, 1.0);
    lengthKnob.setValue (32.0, juce::dontSendNotification);
    volumeKnob.setRange (0.0, 1.0, 0.01);
    volumeKnob.setValue (0.8, juce::dontSendNotification);

    addMouseListener (&mouseListener, true);
}

SequenceRowComponent::~SequenceRowComponent()
{
    removeMouseListener (&mouseListener);
}

void SequenceRowComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (0.5f);

    g.setColour (juce::Colours::white);
    g.fillRoundedRectangle (bounds, 5.0f);

    if (selected)
    {
        g.setColour (juce::Colour (0xff3a7bd5));
        g.drawRoundedRectangle (bounds, 5.0f, 2.0f);
    }
}

void SequenceRowComponent::resized()
{
    const float pad = 5.6f;
    const float gap = 3.5f;
    auto area = getLocalBounds().toFloat().reduced (pad);

    auto buttonsArea = area.removeFromLeft (76.0f);
    auto copyArea = buttonsArea.removeFromLeft (33.0f);
    copyButton.setBounds (copyArea.getX(), (getHeight() - 31) / 2, 33, 31);
    buttonsArea.removeFromLeft (10.0f);
    pasteButton.setBounds (buttonsArea.getX(), (getHeight() - 31) / 2, 33, 31);

    area.removeFromLeft (gap);

    auto volumeArea = area.removeFromRight (50.0f);
    volumeKnob.setBounds (volumeArea.getX(), (getHeight() - 50) / 2, 50, 50);
    area.removeFromRight (gap);
    auto lengthArea = area.removeFromRight (50.0f);
    lengthKnob.setBounds (lengthArea.getX(), (getHeight() - 50) / 2, 50, 50);
    area.removeFromRight (gap);

    const int n = (int) stepButtons.size();
    const float stepW = juce::jmin (26.0f, (area.getWidth() - gap * (float) (n - 1)) / (float) n);
    const float startX = area.getX() + (area.getWidth() - (stepW * n + gap * (float) (n - 1))) / 2.0f;
    const float y = (getHeight() - stepW) / 2.0f;

    for (int i = 0; i < n; ++i)
        stepButtons[(size_t) i]->setBounds (juce::roundToInt (startX + i * (stepW + gap)),
                                            juce::roundToInt (y),
                                            juce::roundToInt (stepW),
                                            juce::roundToInt (stepW));
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
