#include "ClockPanel.h"

ClockPanel::ClockPanel()
{
    titleLabel.setFont (juce::FontOptions (15.0f, juce::Font::bold));
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::black);
    addAndMakeVisible (titleLabel);

    tempoKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    tempoKnob.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    tempoKnob.setName ("Tempo");
    tempoKnob.setRange (20.0, 300.0, 1.0);
    tempoKnob.setValue (120.0, juce::dontSendNotification);
    addAndMakeVisible (tempoKnob);

    playStopButton.setClickingTogglesState (true);
    playStopButton.onClick = [this]
    {
        playStopButton.setButtonText (playStopButton.getToggleState() ? "Stop" : "Play");
    };
    addAndMakeVisible (playStopButton);
}

void ClockPanel::paint (juce::Graphics& g)
{
    g.setColour (juce::Colour (0xffdedede));
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (0.5f), 6.0f);
}

void ClockPanel::resized()
{
    auto area = getLocalBounds().reduced (6);

    titleLabel.setBounds (area.removeFromTop (24));
    area.removeFromTop (10);

    auto knobArea = area.removeFromTop (175);
    tempoKnob.setBounds (knobArea.getX() + (knobArea.getWidth() - 175) / 2,
                         knobArea.getY(), 175, 175);

    area.removeFromTop (12);
    playStopButton.setBounds (area.getX() + (area.getWidth() - 90) / 2,
                              area.getY(), 90, 40);
}
