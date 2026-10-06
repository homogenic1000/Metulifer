#include "VCFPanel.h"

VCFPanel::VCFPanel()
{
    titleLabel.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::black);
    addAndMakeVisible (titleLabel);

    lpfButton.setClickingTogglesState (true);
    hpfButton.setClickingTogglesState (true);
    lpfButton.setToggleState (true, juce::dontSendNotification);

    lpfButton.onClick = [this]
    {
        hpfButton.setToggleState (false, juce::dontSendNotification);
        updateFilterType();
    };
    hpfButton.onClick = [this]
    {
        lpfButton.setToggleState (false, juce::dontSendNotification);
        updateFilterType();
    };

    addAndMakeVisible (lpfButton);
    addAndMakeVisible (hpfButton);

    cutoffLabel.setFont (juce::FontOptions (11.0f));
    cutoffLabel.setJustificationType (juce::Justification::centred);
    cutoffLabel.setColour (juce::Label::textColourId, juce::Colours::black);
    addAndMakeVisible (cutoffLabel);

    cutoffKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    cutoffKnob.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    cutoffKnob.setName ("VCF_Cutoff");
    cutoffKnob.setRange (20.0, 20000.0, 1.0);
    cutoffKnob.setSkewFactorFromMidPoint (1000.0);
    cutoffKnob.setValue (1000.0, juce::dontSendNotification);
    addAndMakeVisible (cutoffKnob);
}

void VCFPanel::paint (juce::Graphics& g)
{
    g.setColour (juce::Colour (0xffdedede));
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (0.5f), 6.0f);
}

void VCFPanel::resized()
{
    auto area = getLocalBounds().reduced (4);

    titleLabel.setBounds (area.removeFromTop (22));

    auto buttons = area.removeFromTop (49);
    lpfButton.setBounds (buttons.getX(), buttons.getY(), 50, 49);
    hpfButton.setBounds (buttons.getX() + 55, buttons.getY(), 50, 49);

    area.removeFromTop (16);
    auto knobArea = area.removeFromTop (100);
    cutoffKnob.setBounds (knobArea.getX() + 10, knobArea.getY(), 86, 86);
    cutoffLabel.setBounds (knobArea.getX(), knobArea.getBottom() - 4, 105, 14);
}

void VCFPanel::updateFilterType()
{
    if (! lpfButton.getToggleState() && ! hpfButton.getToggleState())
        lpfButton.setToggleState (true, juce::dontSendNotification);
}
