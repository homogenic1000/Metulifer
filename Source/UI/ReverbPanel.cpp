#include "ReverbPanel.h"
#include "Skin.h"
#include "Typography.h"

ReverbPanel::ReverbPanel()
{
    titleLabel.setFont (Typography::getSubheading (13.0f));
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setColour (juce::Label::textColourId, juce::Colour (0xff1e293b));
    addAndMakeVisible (titleLabel);

    // Tabs
    for (auto* tab : { &tabRev, &tabDly, &tabRdx })
    {
        tab->setClickingTogglesState (false);
        addAndMakeVisible (tab);
    }

    tabRev.onClick = [this] { setSelectedTab (0); };
    tabDly.onClick = [this] { setSelectedTab (1); };
    tabRdx.onClick = [this] { setSelectedTab (2); };

    auto setupKnob = [this] (juce::Slider& knob, const juce::String& name, double min, double max, double step, double def)
    {
        knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        knob.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
        knob.setName (name);
        knob.setRange (min, max, step);
        knob.setValue (def, juce::dontSendNotification);
        addAndMakeVisible (knob);
    };

    auto setupLabel = [this] (juce::Label& lbl, const juce::String& text)
    {
        lbl.setText (text, juce::dontSendNotification);
        lbl.setFont (Typography::getBoldLabel (10.0f));
        lbl.setJustificationType (juce::Justification::centred);
        lbl.setColour (juce::Label::textColourId, juce::Colour (0xff475569));
        addAndMakeVisible (lbl);
    };

    // 1. Reverb controls
    revOnOffButton.setClickingTogglesState (true);
    revOnOffButton.setToggleState (false, juce::dontSendNotification);
    revOnOffButton.setButtonText ("Off");
    revOnOffButton.onClick = [this]
    {
        isRevOn = revOnOffButton.getToggleState();
        revOnOffButton.setButtonText (isRevOn ? "On" : "Off");
        if (onReverbPowerToggled)
            onReverbPowerToggled (isRevOn);
        repaint();
    };
    addAndMakeVisible (revOnOffButton);

    revModeButton.setButtonText ("Room");
    revModeButton.onClick = [this]
    {
        revModeIndex = (revModeIndex + 1) % 2;
        revModeButton.setButtonText (revModeIndex == 0 ? "Room" : "Hall");
        if (onReverbModeToggled)
            onReverbModeToggled (revModeIndex);
    };
    addAndMakeVisible (revModeButton);

    setupKnob (revMixKnob, "Reverb_Mix", 0.0, 100.0, 1.0, 30.0);
    setupLabel (revMixLabel, "Mix");

    // 2. Delay controls
    dlyOnOffButton.setClickingTogglesState (true);
    dlyOnOffButton.setToggleState (false, juce::dontSendNotification);
    dlyOnOffButton.setButtonText ("Off");
    dlyOnOffButton.onClick = [this]
    {
        isDlyOn = dlyOnOffButton.getToggleState();
        dlyOnOffButton.setButtonText (isDlyOn ? "On" : "Off");
        if (onDelayPowerToggled)
            onDelayPowerToggled (isDlyOn);
        repaint();
    };
    addAndMakeVisible (dlyOnOffButton);

    dlyModeButton.setButtonText ("PingPong");
    dlyModeButton.onClick = [this]
    {
        isDlyPingpong = ! isDlyPingpong;
        dlyModeButton.setButtonText (isDlyPingpong ? "PingPong" : "Stereo");
        if (onDelayModeToggled)
            onDelayModeToggled (isDlyPingpong);
    };
    addAndMakeVisible (dlyModeButton);

    setupKnob (dlyTimeKnob, "Delay_Time", 20.0, 1000.0, 1.0, 250.0);
    dlyTimeKnob.setSkewFactorFromMidPoint (200.0);
    setupLabel (dlyTimeLabel, "Time");

    setupKnob (dlyFeedbackKnob, "Delay_Feedback", 0.0, 90.0, 1.0, 40.0);
    setupLabel (dlyFeedbackLabel, "Feedbk");

    setupKnob (dlyMixKnob, "Delay_Mix", 0.0, 100.0, 1.0, 35.0);
    setupLabel (dlyMixLabel, "Mix");

    // 3. Redux controls
    rdxOnOffButton.setClickingTogglesState (true);
    rdxOnOffButton.setToggleState (false, juce::dontSendNotification);
    rdxOnOffButton.setButtonText ("Off");
    rdxOnOffButton.onClick = [this]
    {
        isRdxOn = rdxOnOffButton.getToggleState();
        rdxOnOffButton.setButtonText (isRdxOn ? "On" : "Off");
        if (onReduxPowerToggled)
            onReduxPowerToggled (isRdxOn);
        repaint();
    };
    addAndMakeVisible (rdxOnOffButton);

    setupKnob (rdxBitsKnob, "Redux_Bits", 2.0, 16.0, 1.0, 8.0);
    setupLabel (rdxBitsLabel, "Bits");

    setupKnob (rdxRateKnob, "Redux_Rate", 1.0, 32.0, 1.0, 4.0);
    setupLabel (rdxRateLabel, "Rate");

    setupKnob (rdxMixKnob, "Redux_Mix", 0.0, 100.0, 1.0, 50.0);
    setupLabel (rdxMixLabel, "Mix");

    setSelectedTab (0);
}

void ReverbPanel::setSelectedTab (int tab)
{
    currentTab = juce::jlimit (0, 2, tab);
    updateTabVisibility();
    repaint();
}

void ReverbPanel::updateTabVisibility()
{
    const bool rev = (currentTab == 0);
    const bool dly = (currentTab == 1);
    const bool rdx = (currentTab == 2);

    revOnOffButton.setVisible (rev);
    revModeButton.setVisible (rev);
    revMixKnob.setVisible (rev);
    revMixLabel.setVisible (rev);

    dlyOnOffButton.setVisible (dly);
    dlyModeButton.setVisible (dly);
    dlyTimeKnob.setVisible (dly);
    dlyTimeLabel.setVisible (dly);
    dlyFeedbackKnob.setVisible (dly);
    dlyFeedbackLabel.setVisible (dly);
    dlyMixKnob.setVisible (dly);
    dlyMixLabel.setVisible (dly);

    rdxOnOffButton.setVisible (rdx);
    rdxBitsKnob.setVisible (rdx);
    rdxBitsLabel.setVisible (rdx);
    rdxRateKnob.setVisible (rdx);
    rdxRateLabel.setVisible (rdx);
    rdxMixKnob.setVisible (rdx);
    rdxMixLabel.setVisible (rdx);
}

void ReverbPanel::paint (juce::Graphics& g)
{
    Skin::drawPanel (g, "panel_reverb", getLocalBounds().toFloat().reduced (0.5f),
                     juce::Colour (0xffdedede), 6.0f);

    // Draw active indicator LEDs on the 3 tabs
    auto drawLed = [&g] (const juce::Component& c, bool active)
    {
        const auto b = c.getBounds().toFloat();
        const float dotSize = 4.0f;
        const float dotX = b.getRight() - 6.0f;
        const float dotY = b.getY() + 4.0f;
        g.setColour (active ? juce::Colour (0xff22c55e) : juce::Colour (0xff475569));
        g.fillEllipse (dotX, dotY, dotSize, dotSize);
    };

    drawLed (tabRev, isRevOn);
    drawLed (tabDly, isDlyOn);
    drawLed (tabRdx, isRdxOn);

    // Active tab indicator highlight line
    const juce::Component* activeComp = (currentTab == 0 ? &tabRev : (currentTab == 1 ? &tabDly : &tabRdx));
    auto ab = activeComp->getBounds().toFloat();
    g.setColour (juce::Colour (0xff2563eb));
    g.fillRect (ab.getX() + 2.0f, ab.getBottom() + 1.0f, ab.getWidth() - 4.0f, 2.0f);
}

void ReverbPanel::resized()
{
    titleLabel.setBounds (0, 3, getWidth(), 18);

    // Tabs row
    tabRev.setBounds (4, 23, 30, 20);
    tabDly.setBounds (37, 23, 31, 20);
    tabRdx.setBounds (71, 23, 30, 20);

    // Reverb controls
    revOnOffButton.setBounds (6, 50, 44, 32);
    revModeButton.setBounds (55, 50, 44, 32);
    revMixKnob.setBounds (20, 96, 65, 65);
    revMixLabel.setBounds (0, 163, getWidth(), 14);

    // Delay controls
    dlyOnOffButton.setBounds (6, 48, 44, 28);
    dlyModeButton.setBounds (54, 48, 46, 28);
    dlyTimeKnob.setBounds (8, 80, 42, 42);
    dlyTimeLabel.setBounds (4, 122, 50, 12);
    dlyFeedbackKnob.setBounds (55, 80, 42, 42);
    dlyFeedbackLabel.setBounds (51, 122, 50, 12);
    dlyMixKnob.setBounds (28, 140, 48, 48);
    dlyMixLabel.setBounds (0, 188, getWidth(), 14);

    // Redux controls
    rdxOnOffButton.setBounds (15, 48, 75, 28);
    rdxBitsKnob.setBounds (8, 80, 42, 42);
    rdxBitsLabel.setBounds (4, 122, 50, 12);
    rdxRateKnob.setBounds (55, 80, 42, 42);
    rdxRateLabel.setBounds (51, 122, 50, 12);
    rdxMixKnob.setBounds (28, 140, 48, 48);
    rdxMixLabel.setBounds (0, 188, getWidth(), 14);

    updateTabVisibility();
}

void ReverbPanel::setReverbEnabled (bool on)
{
    isRevOn = on;
    revOnOffButton.setToggleState (on, juce::dontSendNotification);
    revOnOffButton.setButtonText (on ? "On" : "Off");
    repaint();
}

void ReverbPanel::setReverbMode (int mode)
{
    revModeIndex = juce::jlimit (0, 1, mode);
    revModeButton.setButtonText (revModeIndex == 0 ? "Room" : "Hall");
}

void ReverbPanel::setDelayEnabled (bool on)
{
    isDlyOn = on;
    dlyOnOffButton.setToggleState (on, juce::dontSendNotification);
    dlyOnOffButton.setButtonText (on ? "On" : "Off");
    repaint();
}

void ReverbPanel::setDelayPingpong (bool pp)
{
    isDlyPingpong = pp;
    dlyModeButton.setButtonText (isDlyPingpong ? "PingPong" : "Stereo");
}

void ReverbPanel::setReduxEnabled (bool on)
{
    isRdxOn = on;
    rdxOnOffButton.setToggleState (on, juce::dontSendNotification);
    rdxOnOffButton.setButtonText (on ? "On" : "Off");
    repaint();
}
