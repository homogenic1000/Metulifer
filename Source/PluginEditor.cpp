/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "UI/Skin.h"

//==============================================================================
MetuliferAudioProcessorEditor::MetuliferAudioProcessorEditor (MetuliferAudioProcessor& p)
    : AudioProcessorEditor (&p),
      audioProcessor (p),
      sequencerPanel (p.apvts)
{
    setLookAndFeel (&lookAndFeel);

    addAndMakeVisible (sequencerPanel);
    addAndMakeVisible (displayScreen);
    addAndMakeVisible (clockPanel);
    addAndMakeVisible (synthPanel);
    addAndMakeVisible (vcfPanel);
    addAndMakeVisible (lfoPanel1);
    addAndMakeVisible (lfoPanel2);
    addAndMakeVisible (cableLayer); // au-dessus de tout (hitTest limité aux jacks/câbles)

    sequencerPanel.onSelectionChanged = [this] (int index)
    {
        sequenceSelected (index);
    };

    vcfPanel.onFilterToggled = [this] (bool useLpf)
    {
        setFilterTypeParam (useLpf);
    };

    clockPanel.onPlayToggled = [this] (bool shouldPlay)
    {
        audioProcessor.setPlaying (shouldPlay);
        clearParamDisplay();
    };

    for (auto* lever : { &synthPanel.vco1Lever, &synthPanel.vco2Lever })
    {
        lever->onValueChanged = [this, lever] (int value)
        {
            showText (lever->getName() + " " + juce::String (value));
        };
    }

    // Knobs bound once and for all (one row = one sequence).
    auto track = [this] (juce::Slider& knob, const juce::String& paramName, const juce::String& display)
    {
        knob.setName (display);
        sliderParamIds[&knob] = paramName;
        knob.addListener (this);
    };

    for (int i = 0; i < MetuliferAudioProcessor::numSequences; ++i)
        if (auto* row = sequencerPanel.getRow (i))
        {
            track (row->getLengthKnob(), MetuliferAudioProcessor::seqParamId (i, "length"), "length");
            track (row->getVolumeKnob(), MetuliferAudioProcessor::seqParamId (i, "volume"), "volume");
        }

    track (clockPanel.tempoKnob, "tempo", "tempo");

    // LFO : bindings globaux (jamais re-bindés).
    {
        updatingBindings = true;

        auto bindLfoKnob = [this, &track] (juce::Slider& knob, const juce::String& paramId,
                                           const juce::String& display)
        {
            track (knob, paramId, display);
            lfoAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
                audioProcessor.apvts, paramId, knob));
        };

        struct { LFOPanel& panel; int index; } lfos[] = { { lfoPanel1, 0 }, { lfoPanel2, 1 } };

        for (auto& l : lfos)
        {
            const auto n = juce::String (l.index + 1);
            bindLfoKnob (l.panel.rateKnob,   "lfo" + n + "_rate",   "rate lfo" + n);
            bindLfoKnob (l.panel.offsetKnob, "lfo" + n + "_offset", "offset lfo" + n);
            bindLfoKnob (l.panel.depthKnob,  "lfo" + n + "_depth",  "depth lfo" + n);

            const auto idx = l.index;
            l.panel.onWaveClicked = [this, idx] (int wave)
            { setLfoWave (idx, wave); };
            l.panel.onPowerClicked = [this, idx] (bool on)
            { setLfoPower (idx, on); };
        }

        updatingBindings = false;
    }

    tempoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, "tempo", clockPanel.tempoKnob);

    cableLayer.setModTree (audioProcessor.getModTree(), &audioProcessor.undoManager);

    // Knobs re-bound on every sequence selection (ids/display set in bindSequence).
    for (auto* knob : { &synthPanel.vco1Note, &synthPanel.vco1Wave,
                        &synthPanel.vco2Note, &synthPanel.vco2Wave,
                        &synthPanel.adsr1Attack, &synthPanel.adsr1Decay,
                        &synthPanel.adsr1Sustain, &synthPanel.adsr1Release,
                        &synthPanel.adsr2Attack, &synthPanel.adsr2Decay,
                        &synthPanel.adsr2Sustain, &synthPanel.adsr2Release,
                        &synthPanel.mixVco1, &synthPanel.mixVco2,
                        &vcfPanel.cutoffKnob })
        knob->addListener (this);

    bindSequence (0);
    displayScreen.setSequenceNumber (1);

    sequencerPanel.setStepsTree (audioProcessor.getStepsTree());

    startTimerHz (30);
    setSize (1280, 720);
}

MetuliferAudioProcessorEditor::~MetuliferAudioProcessorEditor()
{
    setLookAndFeel (nullptr);

    if (filterParamId.isNotEmpty())
        audioProcessor.apvts.removeParameterListener (filterParamId, this);
}

//==============================================================================
void MetuliferAudioProcessorEditor::paint (juce::Graphics& g)
{
    if (auto img = Skin::find ("bg_editor"); img.isValid())
        g.drawImage (img, getLocalBounds().toFloat(),
                     juce::RectanglePlacement::stretchToFit, false);
    else
        g.fillAll (juce::Colour (0xffe9e9e9));

    g.setColour (juce::Colours::black);
    g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    g.drawText ("METULIFER", 32, 8, 200, 20, juce::Justification::centredLeft);
}

void MetuliferAudioProcessorEditor::resized()
{
    sequencerPanel.setBounds (32, 32, 910, 417);
    displayScreen.setBounds (986, 32, 247, 132);
    clockPanel.setBounds (961, 164, 297, 284);
    synthPanel.setBounds (32, 458, 786, 237);
    vcfPanel.setBounds (684, 471, 105, 224);
    lfoPanel1.setBounds (874, 471, 105, 224);
    lfoPanel2.setBounds (1013, 471, 105, 224);
    cableLayer.setBounds (0, 0, getWidth(), getHeight());

    refreshCableTargets();
}

void MetuliferAudioProcessorEditor::refreshCableTargets()
{
    std::vector<CableLayer::Source> src;
    for (int i = 0; i < 2; ++i)
    {
        auto& panel = i == 0 ? lfoPanel1 : lfoPanel2;
        src.push_back ({ i, panel.getJackBounds()
                                + juce::Point<float> ((float) panel.getX(), (float) panel.getY()) });
    }
    cableLayer.setSources (std::move (src));

    std::vector<CableLayer::Target> tgt;

    struct Ref { juce::Slider* knob; const char* suffix; };

    const Ref refs[] = {
        { &synthPanel.adsr1Attack,  "a1"   }, { &synthPanel.adsr1Decay,   "d1"   },
        { &synthPanel.adsr1Sustain, "s1"   }, { &synthPanel.adsr1Release, "r1"   },
        { &synthPanel.adsr2Attack,  "a2"   }, { &synthPanel.adsr2Decay,   "d2"   },
        { &synthPanel.adsr2Sustain, "s2"   }, { &synthPanel.adsr2Release, "r2"   },
        { &synthPanel.mixVco1,      "mix1" }, { &synthPanel.mixVco2,      "mix2" },
        { &vcfPanel.cutoffKnob,     "cutoff" },
    };

    for (const auto& r : refs)
        tgt.push_back ({ r.suffix,
                         r.knob->getBounds().toFloat()
                             + juce::Point<float> ((float) r.knob->getParentComponent()->getX(),
                                                   (float) r.knob->getParentComponent()->getY()) });

    cableLayer.setTargets (std::move (tgt));
}

//==============================================================================
void MetuliferAudioProcessorEditor::sequenceSelected (int index)
{
    displayScreen.setSequenceNumber (index + 1);
    bindSequence (index);
    cableLayer.setSelectedSequence (index);
    clearParamDisplay();
}

void MetuliferAudioProcessorEditor::bindSequence (int index)
{
    currentSeq = index;
    updatingBindings = true;

    if (filterParamId.isNotEmpty())
        audioProcessor.apvts.removeParameterListener (filterParamId, this);

    synthAttachments.clear();
    vcfAttachments.clear();

    auto addKnob = [this] (std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>>& target,
                           juce::Slider& knob, const juce::String& paramName, const juce::String& display)
    {
        knob.setName (display);
        sliderParamIds[&knob] = paramName;
        target.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            audioProcessor.apvts, paramName, knob));
    };

    const auto id = [index] (const juce::String& n)
    {
        return MetuliferAudioProcessor::seqParamId (index, n);
    };

    synthPanel.vco1Lever.setParam (audioProcessor.apvts, id ("octave1"));
    synthPanel.vco1Lever.setName ("octave VCO1");
    synthPanel.vco2Lever.setParam (audioProcessor.apvts, id ("octave2"));
    synthPanel.vco2Lever.setName ("octave VCO2");

    addKnob (synthAttachments, synthPanel.vco1Note,     id ("note1"),  "note VCO1");
    addKnob (synthAttachments, synthPanel.vco1Wave,     id ("wave1"),  "wave VCO1");
    addKnob (synthAttachments, synthPanel.vco2Note,     id ("note2"),  "note VCO2");
    addKnob (synthAttachments, synthPanel.vco2Wave,     id ("wave2"),  "wave VCO2");

    addKnob (synthAttachments, synthPanel.adsr1Attack,  id ("a1"), "attack VCO1");
    addKnob (synthAttachments, synthPanel.adsr1Decay,   id ("d1"), "decay VCO1");
    addKnob (synthAttachments, synthPanel.adsr1Sustain, id ("s1"), "sustain VCO1");
    addKnob (synthAttachments, synthPanel.adsr1Release, id ("r1"), "release VCO1");

    addKnob (synthAttachments, synthPanel.adsr2Attack,  id ("a2"), "attack VCO2");
    addKnob (synthAttachments, synthPanel.adsr2Decay,   id ("d2"), "decay VCO2");
    addKnob (synthAttachments, synthPanel.adsr2Sustain, id ("s2"), "sustain VCO2");
    addKnob (synthAttachments, synthPanel.adsr2Release, id ("r2"), "release VCO2");

    addKnob (synthAttachments, synthPanel.mixVco1,      id ("mix1"),  "mix VCO1");
    addKnob (synthAttachments, synthPanel.mixVco2,      id ("mix2"),  "mix VCO2");

    addKnob (vcfAttachments,   vcfPanel.cutoffKnob,     id ("cutoff"), "cutoff");

    filterParamId = id ("filter");
    audioProcessor.apvts.addParameterListener (filterParamId, this);
    updateFilterButtons();

    updatingBindings = false;
}

//==============================================================================
void MetuliferAudioProcessorEditor::sliderDragStarted (juce::Slider* slider)
{
    if (! updatingBindings && slider != nullptr)
        showParamFor (*slider);
}

void MetuliferAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    if (! updatingBindings && slider != nullptr)
        showParamFor (*slider);
}

void MetuliferAudioProcessorEditor::showParamFor (juce::Slider& slider)
{
    auto it = sliderParamIds.find (&slider);
    if (it == sliderParamIds.end())
        return;

    auto* param = audioProcessor.apvts.getParameter (it->second);
    if (param == nullptr)
        return;

    const float normalised = param->getNormalisableRange().convertTo0to1 ((float) slider.getValue());

    juce::String text = slider.getName();
    text << " " << param->getText (normalised, 8);

    const auto label = param->getLabel();
    if (label.isNotEmpty())
        text << " " << label;

    showText (text);
}

void MetuliferAudioProcessorEditor::showText (const juce::String& text)
{
    displayScreen.setParamText (text);
    paramShown = true;
    paramShownUntilMs = juce::Time::getMillisecondCounter() + 2000;
}

void MetuliferAudioProcessorEditor::clearParamDisplay()
{
    paramShown = false;
    displayScreen.setParamText ({});
}

//==============================================================================
void MetuliferAudioProcessorEditor::setFilterTypeParam (bool useLpf)
{
    auto* param = audioProcessor.apvts.getParameter (filterParamId);
    if (param == nullptr)
        return;

    param->beginChangeGesture();
    param->setValueNotifyingHost (useLpf ? 0.0f : 1.0f);
    param->endChangeGesture();
}

void MetuliferAudioProcessorEditor::updateFilterButtons()
{
    auto* param = audioProcessor.apvts.getParameter (filterParamId);
    if (param == nullptr)
        return;

    vcfPanel.setFilterType (param->getValue() < 0.5f);
}

void MetuliferAudioProcessorEditor::parameterChanged (const juce::String& parameterID, float)
{
    if (parameterID != filterParamId)
        return;

    updateFilterButtons();

    auto* param = audioProcessor.apvts.getParameter (parameterID);
    if (param != nullptr)
        showText ("filter " + juce::String (param->getValue() < 0.5f ? "LPF" : "HPF"));
}

//==============================================================================
void MetuliferAudioProcessorEditor::setLfoWave (int lfoIndex, int waveIndex)
{
    auto* param = audioProcessor.apvts.getParameter ("lfo" + juce::String (lfoIndex + 1) + "_wave");
    if (param == nullptr)
        return;

    param->beginChangeGesture();
    param->setValueNotifyingHost ((float) waveIndex / 3.0f);
    param->endChangeGesture();

    static const char* names[] = { "sine", "saw", "square", "triangle" };
    showText ("wave lfo" + juce::String (lfoIndex + 1) + " " + names[juce::jlimit (0, 3, waveIndex)]);
}

void MetuliferAudioProcessorEditor::setLfoPower (int lfoIndex, bool on)
{
    auto* param = audioProcessor.apvts.getParameter ("lfo" + juce::String (lfoIndex + 1) + "_enabled");
    if (param == nullptr)
        return;

    param->beginChangeGesture();
    param->setValueNotifyingHost (on ? 1.0f : 0.0f);
    param->endChangeGesture();

    showText ("lfo" + juce::String (lfoIndex + 1) + " " + (on ? "on" : "off"));
}

//==============================================================================
void MetuliferAudioProcessorEditor::timerCallback()
{
    displayScreen.setRmsLevel (audioProcessor.getCurrentRms());

    const bool playing = audioProcessor.isPlaying();

    for (int i = 0; i < MetuliferAudioProcessor::numSequences; ++i)
    {
        const int step = playing ? audioProcessor.getPlayheadStep (i) : -1;

        if (step != lastPlayhead[i])
        {
            lastPlayhead[i] = step;
            sequencerPanel.setPlayhead (i, step);
        }
    }

    if (paramShown && juce::Time::getMillisecondCounter() >= paramShownUntilMs)
        clearParamDisplay();
}
