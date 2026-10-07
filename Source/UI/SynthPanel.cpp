#include "SynthPanel.h"
#include "Skin.h"

SynthPanel::SynthPanel()
{
    setupKnob (vco1Note, "VCO1_Note", 0.0, 11.0, 1.0, 0.0);
    setupKnob (vco1Wave, "VCO1_Wave",  0.0, 3.0, 1.0, 1.0);
    setupKnob (vco2Note, "VCO2_Note", 0.0, 11.0, 1.0, 0.0);
    setupKnob (vco2Wave, "VCO2_Wave",  0.0, 3.0, 1.0, 1.0);

    addAndMakeVisible (vco1Lever);
    addAndMakeVisible (vco2Lever);

    setupKnob (adsr1Attack,  "ADSR1_Attack",  0.0, 1.0, 0.01, 0.01);
    setupKnob (adsr1Decay,   "ADSR1_Decay",   0.0, 1.0, 0.01, 0.20);
    setupKnob (adsr1Sustain, "ADSR1_Sustain", 0.0, 1.0, 0.01, 0.70);
    setupKnob (adsr1Release, "ADSR1_Release", 0.0, 1.0, 0.01, 0.30);

    setupKnob (adsr2Attack,  "ADSR2_Attack",  0.0, 1.0, 0.01, 0.01);
    setupKnob (adsr2Decay,   "ADSR2_Decay",   0.0, 1.0, 0.01, 0.20);
    setupKnob (adsr2Sustain, "ADSR2_Sustain", 0.0, 1.0, 0.01, 0.70);
    setupKnob (adsr2Release, "ADSR2_Release", 0.0, 1.0, 0.01, 0.30);

    setupKnob (mixVco1, "Mix_VCO1", 0.0, 1.0, 0.01, 0.8);
    setupKnob (mixVco2, "Mix_VCO2", 0.0, 1.0, 0.01, 0.8);

    makeLabel (*this, "VCO1", 11.0f)->setName ("l_vco1");
    makeLabel (*this, "VCO2", 11.0f)->setName ("l_vco2");

    makeLabel (*this, "Octave", 11.0f)->setName ("l_oct");
    makeLabel (*this, "Note", 13.0f)->setName ("l_note");
    makeLabel (*this, "Wave", 13.0f)->setName ("l_wave");
    makeLabel (*this, "ADSR", 13.0f)->setName ("l_adsr");

    makeLabel (*this, "A", 10.0f)->setName ("l_a1");
    makeLabel (*this, "D", 10.0f)->setName ("l_d1");
    makeLabel (*this, "S", 10.0f)->setName ("l_s1");
    makeLabel (*this, "R", 10.0f)->setName ("l_r1");
    makeLabel (*this, "A", 10.0f)->setName ("l_a2");
    makeLabel (*this, "D", 10.0f)->setName ("l_d2");
    makeLabel (*this, "S", 10.0f)->setName ("l_s2");
    makeLabel (*this, "R", 10.0f)->setName ("l_r2");

    makeLabel (*this, "Mix out DB", 13.0f)->setName ("l_mix");
}

void SynthPanel::paint (juce::Graphics& g)
{
    Skin::drawPanel (g, "panel_synth", getLocalBounds().toFloat().reduced (0.5f),
                     juce::Colour (0xffdedede), 6.0f);
}

void SynthPanel::resized()
{
    const int big = 86;
    const int adsr = 31;

    auto labelFor = [this] (const juce::String& name) -> juce::Label*
    {
        for (auto& l : labels)
            if (l->getName() == name)
                return l.get();
        return nullptr;
    };

    auto place = [] (juce::Component* c, int x, int y, int w, int h)
    {
        if (c != nullptr)
            c->setBounds (x, y, w, h);
    };

    place (labelFor ("l_vco1"), 0, 76, 26, 14);
    place (labelFor ("l_vco2"), 0, 167, 26, 14);

    place (labelFor ("l_oct"),   16, 14, 48, 16);
    place (labelFor ("l_note"),  58, 14, 86, 16);
    place (labelFor ("l_wave"), 176, 14, 86, 16);
    place (labelFor ("l_adsr"), 298, 14, 148, 16);
    place (labelFor ("l_mix"),  499, 14, 93, 16);

    vco1Lever.setBounds (28, 46, 24, 74);
    vco1Note .setBounds (58, 40, big, big);
    vco1Wave .setBounds (176, 40, big, big);
    mixVco1  .setBounds (503, 40, big, big);

    juce::Slider* adsr1[4] = { &adsr1Attack, &adsr1Decay, &adsr1Sustain, &adsr1Release };
    const char* adsr1Names[4] = { "l_a1", "l_d1", "l_s1", "l_r1" };
    for (int i = 0; i < 4; ++i)
    {
        adsr1[i]->setBounds (298 + i * 39, 67, adsr, adsr);
        place (labelFor (adsr1Names[i]), 298 + i * 39, 99, adsr, 12);
    }

    vco2Lever.setBounds (28, 137, 24, 74);
    vco2Note .setBounds (58, 131, big, big);
    vco2Wave .setBounds (176, 131, big, big);
    mixVco2  .setBounds (503, 131, big, big);

    juce::Slider* adsr2[4] = { &adsr2Attack, &adsr2Decay, &adsr2Sustain, &adsr2Release };
    const char* adsr2Names[4] = { "l_a2", "l_d2", "l_s2", "l_r2" };
    for (int i = 0; i < 4; ++i)
    {
        adsr2[i]->setBounds (298 + i * 39, 158, adsr, adsr);
        place (labelFor (adsr2Names[i]), 298 + i * 39, 190, adsr, 12);
    }
}

void SynthPanel::setupKnob (juce::Slider& knob, const juce::String& name,
                            double min, double max, double step, double def)
{
    knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    knob.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    knob.setName (name);
    knob.setRange (min, max, step);
    knob.setValue (def, juce::dontSendNotification);
    addAndMakeVisible (knob);
}

juce::Label* SynthPanel::makeLabel (juce::Component& parent, const juce::String& text, float fontHeight)
{
    auto label = std::make_unique<juce::Label> ("", text);
    label->setFont (juce::FontOptions (fontHeight));
    label->setJustificationType (juce::Justification::centred);
    label->setColour (juce::Label::textColourId, juce::Colours::black);
    auto* ptr = label.get();
    parent.addAndMakeVisible (*label);
    labels.push_back (std::move (label));
    return ptr;
}
