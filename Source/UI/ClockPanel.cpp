#include "ClockPanel.h"
#include "Skin.h"
#include "Typography.h"

ClockPanel::ClockPanel()
{
    titleLabel.setFont (Typography::getHeading (15.0f));
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
        const bool playing = playStopButton.getToggleState();
        playStopButton.setButtonText (playing ? "Stop" : "Play");

        if (onPlayToggled)
            onPlayToggled (playing);
    };
    addAndMakeVisible (playStopButton);

    syncButton.setClickingTogglesState (true);
    syncButton.setTooltip ("Sync playback and tempo with DAW host");
    addAndMakeVisible (syncButton);
}

void ClockPanel::paint (juce::Graphics& g)
{
    Skin::drawPanel (g, "panel_clock", getLocalBounds().toFloat().reduced (0.5f),
                     juce::Colour (0xffdedede), 6.0f);
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
    const int totalBtnWidth = 90 + 10 + 90;
    const int startX = area.getX() + (area.getWidth() - totalBtnWidth) / 2;
    playStopButton.setBounds (startX, area.getY(), 90, 36);
    syncButton.setBounds (startX + 100, area.getY(), 90, 36);
}
