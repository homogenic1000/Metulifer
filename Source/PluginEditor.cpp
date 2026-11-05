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
    setOpaque (true);
    setLookAndFeel (&lookAndFeel);

    addAndMakeVisible (sequencerPanel);
    addAndMakeVisible (displayScreen);
    addAndMakeVisible (clockPanel);
    addAndMakeVisible (synthPanel);
    addAndMakeVisible (vcfPanel);
    addAndMakeVisible (lfoPanel1);
    addAndMakeVisible (lfoPanel2);
    addAndMakeVisible (reverbPanel);
    addAndMakeVisible (cableLayer); // au-dessus de tout (hitTest limité aux jacks/câbles)
    addAndMakeVisible (inspectButton);
    inspectButton.setTooltip ("Toggle UI Inspector (Cmd+I)");
    inspectButton.onClick = [this] { inspector.toggle(); };
    inspector.setUndoManager (&audioProcessor.undoManager);

    sequencerPanel.onSelectionChanged = [this] (int index)
    {
        sequenceSelected (index);
    };

    sequencerPanel.onStatusMessage = [this] (const juce::String& msg)
    {
        showText (msg);
    };

    vcfPanel.onFilterToggled = [this] (bool useLpf)
    {
        setFilterTypeParam (useLpf);
    };

    reverbPanel.onReverbPowerToggled = [this] (bool on)
    {
        setReverbPowerParam (on);
    };

    reverbPanel.onReverbModeToggled = [this] (int mode)
    {
        setReverbModeParam (mode);
    };

    reverbPanel.onDelayPowerToggled = [this] (bool on)
    {
        setDelayPowerParam (on);
    };

    reverbPanel.onDelayModeToggled = [this] (bool pp)
    {
        setDelayModeParam (pp);
    };

    reverbPanel.onReduxPowerToggled = [this] (bool on)
    {
        setReduxPowerParam (on);
    };

    synthPanel.onArpPowerToggled = [this] (bool on)
    {
        setArpPowerParam (on);
    };

    synthPanel.onArpModeToggled = [this] (int mode)
    {
        setArpModeParam (mode);
    };

    synthPanel.onArpChordToggled = [this] (int chord)
    {
        setArpChordParam (chord);
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
            track (row->getLengthKnob(), MetuliferAudioProcessor::seqParamId (i, "length"),
                   "seq " + juce::String (i + 1) + " length");
            track (row->getVolumeKnob(), MetuliferAudioProcessor::seqParamId (i, "volume"),
                   "seq " + juce::String (i + 1) + " volume");

            row->onArpToggled = [this, i] (bool on)
            {
                if (i == currentSeq)
                    setArpPowerParam (on);
                else
                {
                    auto* param = audioProcessor.apvts.getParameter (MetuliferAudioProcessor::seqParamId (i, "arp_enabled"));
                    if (param != nullptr)
                    {
                        param->beginChangeGesture();
                        param->setValueNotifyingHost (on ? 1.0f : 0.0f);
                        param->endChangeGesture();
                    }
                }
            };
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
    cableLayer.onCablesChanged = [this]
    {
        updateLfoDestinations();
    };
    cableLayer.onCableConnected = [this] (int lfo, const juce::String& suffix)
    {
        showText ("LFO " + juce::String (lfo + 1) + " -> " + suffix.toUpperCase());
        updateLfoDestinations();
    };
    cableLayer.onCableDisconnected = [this] (int lfo, const juce::String& suffix)
    {
        showText ("LFO " + juce::String (lfo + 1) + " UNPLUGGED");
        updateLfoDestinations();
    };

    for (int i = 0; i < 2; ++i)
    {
        const auto prefix = "lfo" + juce::String (i + 1) + "_";
        audioProcessor.apvts.addParameterListener (prefix + "wave", this);
        audioProcessor.apvts.addParameterListener (prefix + "enabled", this);
    }
    updateLfoButtons();

    // Knobs re-bound on every sequence selection (ids/display set in bindSequence).
    for (auto* knob : { &synthPanel.vco1Note, &synthPanel.vco1Wave,
                        &synthPanel.vco2Note, &synthPanel.vco2Wave,
                        &synthPanel.adsr1Attack, &synthPanel.adsr1Decay,
                        &synthPanel.adsr1Sustain, &synthPanel.adsr1Release,
                        &synthPanel.adsr2Attack, &synthPanel.adsr2Decay,
                        &synthPanel.adsr2Sustain, &synthPanel.adsr2Release,
                        &synthPanel.mixVco1, &synthPanel.mixVco2,
                        &synthPanel.arpOctavesKnob,
                        &vcfPanel.cutoffKnob,
                        &reverbPanel.revMixKnob,
                        &reverbPanel.dlyTimeKnob, &reverbPanel.dlyFeedbackKnob, &reverbPanel.dlyMixKnob,
                        &reverbPanel.rdxBitsKnob, &reverbPanel.rdxRateKnob, &reverbPanel.rdxMixKnob })
        knob->addListener (this);

    bindSequence (0);
    displayScreen.setSequenceNumber (1);

    sequencerPanel.setStepsTree (audioProcessor.getStepsTree());

    startTimerHz (60);
    setSize (1280, 720);
}

MetuliferAudioProcessorEditor::~MetuliferAudioProcessorEditor()
{
    setLookAndFeel (nullptr);

    if (filterParamId.isNotEmpty())
        audioProcessor.apvts.removeParameterListener (filterParamId, this);

    if (reverbEnabledParamId.isNotEmpty())
        audioProcessor.apvts.removeParameterListener (reverbEnabledParamId, this);
    if (reverbModeParamId.isNotEmpty())
        audioProcessor.apvts.removeParameterListener (reverbModeParamId, this);

    if (delayEnabledParamId.isNotEmpty())
        audioProcessor.apvts.removeParameterListener (delayEnabledParamId, this);
    if (delayPingpongParamId.isNotEmpty())
        audioProcessor.apvts.removeParameterListener (delayPingpongParamId, this);

    if (reduxEnabledParamId.isNotEmpty())
        audioProcessor.apvts.removeParameterListener (reduxEnabledParamId, this);

    if (arpEnabledParamId.isNotEmpty())
        audioProcessor.apvts.removeParameterListener (arpEnabledParamId, this);
    if (arpModeParamId.isNotEmpty())
        audioProcessor.apvts.removeParameterListener (arpModeParamId, this);
    if (arpChordParamId.isNotEmpty())
        audioProcessor.apvts.removeParameterListener (arpChordParamId, this);

    for (int i = 0; i < 2; ++i)
    {
        const auto prefix = "lfo" + juce::String (i + 1) + "_";
        audioProcessor.apvts.removeParameterListener (prefix + "wave", this);
        audioProcessor.apvts.removeParameterListener (prefix + "enabled", this);
    }
}

//==============================================================================
void MetuliferAudioProcessorEditor::paint (juce::Graphics& g)
{
    if (auto img = Skin::find ("bg_editor"); img.isValid())
    {
        g.drawImage (img, getLocalBounds().toFloat(),
                     juce::RectanglePlacement::stretchToFit, false);
    }
    else
    {
        g.fillAll (juce::Colour (0xffe2e5ea));
    }

    g.setColour (juce::Colour (0xff1e293b));
    g.setFont (juce::FontOptions (15.0f, juce::Font::bold));
    g.drawText ("METULIFER", 32, 7, 120, 20, juce::Justification::centredLeft);

    g.setColour (juce::Colour (0xff64748b));
    g.setFont (juce::FontOptions (10.0f, juce::Font::plain));
    g.drawText ("MODULAR STEP SEQUENCER & DUAL SYNTH", 145, 9, 320, 18, juce::Justification::centredLeft);
}

void MetuliferAudioProcessorEditor::resized()
{
    const int marginX = 32;
    const int rightColWidth = 288;
    const int rightColX = getWidth() - marginX - rightColWidth; // 1280 - 32 - 288 = 960

    inspectButton.setBounds (getWidth() - marginX - 60, 8, 60, 18);

    // Top Section: Sequencer (Left) | DisplayScreen + ClockPanel (Right)
    sequencerPanel.setBounds (marginX, 32, rightColX - marginX - 16, 417);
    displayScreen.setBounds (rightColX, 32, rightColWidth, 128);
    clockPanel.setBounds (rightColX, 172, rightColWidth, 277);

    // Bottom Section: SynthPanel + 4 modular slices (VCF, LFO1, LFO2, Reverb/Multi-FX)
    const int botY = 468;
    const int botH = 224;
    const int moduleW = 105;
    const int gap = 14;

    const int revX  = getWidth() - marginX - moduleW; // 1143
    const int lfo2X = revX - gap - moduleW;           // 1024
    const int lfo1X = lfo2X - gap - moduleW;          // 905
    const int vcfX  = lfo1X - gap - moduleW;          // 786
    const int synthW = vcfX - gap - marginX;          // 740

    synthPanel.setBounds (marginX, botY, synthW, botH);
    vcfPanel.setBounds (vcfX, botY, moduleW, botH);
    lfoPanel1.setBounds (lfo1X, botY, moduleW, botH);
    lfoPanel2.setBounds (lfo2X, botY, moduleW, botH);
    reverbPanel.setBounds (revX, botY, moduleW, botH);

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
        { &reverbPanel.revMixKnob,  "reverb_mix" },
        { &reverbPanel.dlyTimeKnob, "delay_time" },
        { &reverbPanel.dlyFeedbackKnob, "delay_feedback" },
        { &reverbPanel.dlyMixKnob,  "delay_mix" },
        { &reverbPanel.rdxBitsKnob, "redux_bits" },
        { &reverbPanel.rdxRateKnob, "redux_rate" },
        { &reverbPanel.rdxMixKnob,  "redux_mix" },
    };

    targetKnobs.clear();
    for (const auto& r : refs)
    {
        targetKnobs.push_back ({ r.knob, r.suffix });
        tgt.push_back ({ r.suffix,
                         r.knob->getBounds().toFloat()
                             + juce::Point<float> ((float) r.knob->getParentComponent()->getX(),
                                                   (float) r.knob->getParentComponent()->getY()) });
    }

    cableLayer.setTargets (std::move (tgt));
    updateLfoDestinations();
}

//==============================================================================
void MetuliferAudioProcessorEditor::sequenceSelected (int index)
{
    displayScreen.setSequenceNumber (index + 1);
    bindSequence (index);
    cableLayer.setSelectedSequence (index);
    updateLfoDestinations();
    clearParamDisplay();
}

void MetuliferAudioProcessorEditor::bindSequence (int index)
{
    currentSeq = index;
    updatingBindings = true;

    for (const auto& tk : targetKnobs)
    {
        if (tk.knob != nullptr && tk.knob->getProperties().contains ("mod_pos"))
        {
            tk.knob->getProperties().remove ("mod_pos");
            tk.knob->getProperties().remove ("mod_lfo");
            tk.knob->getProperties().remove ("base_pos");
            tk.knob->repaint();
        }
    }

    if (filterParamId.isNotEmpty())
        audioProcessor.apvts.removeParameterListener (filterParamId, this);
    if (reverbEnabledParamId.isNotEmpty())
        audioProcessor.apvts.removeParameterListener (reverbEnabledParamId, this);
    if (reverbModeParamId.isNotEmpty())
        audioProcessor.apvts.removeParameterListener (reverbModeParamId, this);
    if (delayEnabledParamId.isNotEmpty())
        audioProcessor.apvts.removeParameterListener (delayEnabledParamId, this);
    if (delayPingpongParamId.isNotEmpty())
        audioProcessor.apvts.removeParameterListener (delayPingpongParamId, this);
    if (reduxEnabledParamId.isNotEmpty())
        audioProcessor.apvts.removeParameterListener (reduxEnabledParamId, this);
    if (arpEnabledParamId.isNotEmpty())
        audioProcessor.apvts.removeParameterListener (arpEnabledParamId, this);
    if (arpModeParamId.isNotEmpty())
        audioProcessor.apvts.removeParameterListener (arpModeParamId, this);
    if (arpChordParamId.isNotEmpty())
        audioProcessor.apvts.removeParameterListener (arpChordParamId, this);

    synthAttachments.clear();
    vcfAttachments.clear();
    fxAttachments.clear();
    arpAttachments.clear();

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

    // FX: Reverb, Delay, Redux
    addKnob (fxAttachments, reverbPanel.revMixKnob,      id ("reverb_mix"),     "reverb mix");
    addKnob (fxAttachments, reverbPanel.dlyTimeKnob,     id ("delay_time"),     "delay time");
    addKnob (fxAttachments, reverbPanel.dlyFeedbackKnob, id ("delay_feedback"), "delay feedback");
    addKnob (fxAttachments, reverbPanel.dlyMixKnob,      id ("delay_mix"),      "delay mix");
    addKnob (fxAttachments, reverbPanel.rdxBitsKnob,     id ("redux_bits"),     "redux bits");
    addKnob (fxAttachments, reverbPanel.rdxRateKnob,     id ("redux_rate"),     "redux rate");
    addKnob (fxAttachments, reverbPanel.rdxMixKnob,      id ("redux_mix"),      "redux mix");

    // Arp: Octaves
    addKnob (arpAttachments, synthPanel.arpOctavesKnob,  id ("arp_octaves"),    "arp octaves");

    filterParamId = id ("filter");
    audioProcessor.apvts.addParameterListener (filterParamId, this);
    updateFilterButtons();

    reverbEnabledParamId = id ("reverb_enabled");
    reverbModeParamId    = id ("reverb_mode");
    audioProcessor.apvts.addParameterListener (reverbEnabledParamId, this);
    audioProcessor.apvts.addParameterListener (reverbModeParamId, this);

    delayEnabledParamId  = id ("delay_enabled");
    delayPingpongParamId = id ("delay_pingpong");
    audioProcessor.apvts.addParameterListener (delayEnabledParamId, this);
    audioProcessor.apvts.addParameterListener (delayPingpongParamId, this);

    reduxEnabledParamId  = id ("redux_enabled");
    audioProcessor.apvts.addParameterListener (reduxEnabledParamId, this);

    arpEnabledParamId    = id ("arp_enabled");
    arpModeParamId       = id ("arp_mode");
    arpChordParamId      = id ("arp_chord");
    audioProcessor.apvts.addParameterListener (arpEnabledParamId, this);
    audioProcessor.apvts.addParameterListener (arpModeParamId, this);
    audioProcessor.apvts.addParameterListener (arpChordParamId, this);

    updateFxButtons();
    updateArpButtons();

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

void MetuliferAudioProcessorEditor::setReverbPowerParam (bool on)
{
    auto* param = audioProcessor.apvts.getParameter (reverbEnabledParamId);
    if (param == nullptr)
        return;

    param->beginChangeGesture();
    param->setValueNotifyingHost (on ? 1.0f : 0.0f);
    param->endChangeGesture();

    showText ("reverb " + juce::String (on ? "on" : "off"));
}

void MetuliferAudioProcessorEditor::setReverbModeParam (int mode)
{
    auto* param = audioProcessor.apvts.getParameter (reverbModeParamId);
    if (param == nullptr)
        return;

    param->beginChangeGesture();
    param->setValueNotifyingHost (mode == 0 ? 0.0f : 1.0f);
    param->endChangeGesture();

    showText ("reverb " + juce::String (mode == 0 ? "Room" : "Hall"));
}

void MetuliferAudioProcessorEditor::setDelayPowerParam (bool on)
{
    auto* param = audioProcessor.apvts.getParameter (delayEnabledParamId);
    if (param == nullptr)
        return;

    param->beginChangeGesture();
    param->setValueNotifyingHost (on ? 1.0f : 0.0f);
    param->endChangeGesture();

    showText ("delay " + juce::String (on ? "on" : "off"));
}

void MetuliferAudioProcessorEditor::setDelayModeParam (bool pp)
{
    auto* param = audioProcessor.apvts.getParameter (delayPingpongParamId);
    if (param == nullptr)
        return;

    param->beginChangeGesture();
    param->setValueNotifyingHost (pp ? 1.0f : 0.0f);
    param->endChangeGesture();

    showText ("delay " + juce::String (pp ? "PingPong" : "Stereo"));
}

void MetuliferAudioProcessorEditor::setReduxPowerParam (bool on)
{
    auto* param = audioProcessor.apvts.getParameter (reduxEnabledParamId);
    if (param == nullptr)
        return;

    param->beginChangeGesture();
    param->setValueNotifyingHost (on ? 1.0f : 0.0f);
    param->endChangeGesture();

    showText ("redux " + juce::String (on ? "on" : "off"));
}

void MetuliferAudioProcessorEditor::updateFxButtons()
{
    if (auto* p = audioProcessor.apvts.getParameter (reverbEnabledParamId))
        reverbPanel.setReverbEnabled (p->getValue() > 0.5f);
    if (auto* p = audioProcessor.apvts.getParameter (reverbModeParamId))
        reverbPanel.setReverbMode (p->getValue() > 0.5f ? 1 : 0);

    if (auto* p = audioProcessor.apvts.getParameter (delayEnabledParamId))
        reverbPanel.setDelayEnabled (p->getValue() > 0.5f);
    if (auto* p = audioProcessor.apvts.getParameter (delayPingpongParamId))
        reverbPanel.setDelayPingpong (p->getValue() > 0.5f);

    if (auto* p = audioProcessor.apvts.getParameter (reduxEnabledParamId))
        reverbPanel.setReduxEnabled (p->getValue() > 0.5f);
}

void MetuliferAudioProcessorEditor::setArpPowerParam (bool on)
{
    auto* param = audioProcessor.apvts.getParameter (arpEnabledParamId);
    if (param == nullptr)
        return;

    param->beginChangeGesture();
    param->setValueNotifyingHost (on ? 1.0f : 0.0f);
    param->endChangeGesture();

    showText ("arp " + juce::String (on ? "on" : "off"));
}

void MetuliferAudioProcessorEditor::setArpModeParam (int mode)
{
    auto* param = audioProcessor.apvts.getParameter (arpModeParamId);
    if (param == nullptr)
        return;

    const float norm = (float) mode / 10.0f;
    param->beginChangeGesture();
    param->setValueNotifyingHost (norm);
    param->endChangeGesture();

    static const char* names[] = {
        "Up", "Down", "Up&Down", "Down&Up", "Pinky Up", "Pinky Down",
        "Thumb-Pinky", "Converge", "Diverge", "Random", "Order"
    };
    showText ("arp mode " + juce::String (names[juce::jlimit (0, 10, mode)]));
}

void MetuliferAudioProcessorEditor::setArpChordParam (int chord)
{
    auto* param = audioProcessor.apvts.getParameter (arpChordParamId);
    if (param == nullptr)
        return;

    const float norm = (float) chord / 9.0f;
    param->beginChangeGesture();
    param->setValueNotifyingHost (norm);
    param->endChangeGesture();

    static const char* names[] = {
        "Major", "Minor", "Penta", "Octaves", "Maj7", "Min7",
        "Dom7", "Sus4", "Fifths", "Chrom"
    };
    showText ("arp chord " + juce::String (names[juce::jlimit (0, 9, chord)]));
}

void MetuliferAudioProcessorEditor::updateArpButtons()
{
    if (auto* p = audioProcessor.apvts.getParameter (arpEnabledParamId))
    {
        const bool on = p->getValue() > 0.5f;
        synthPanel.setArpEnabled (on);
        if (auto* row = sequencerPanel.getRow (currentSeq))
            row->setArpEnabled (on);
    }

    if (auto* p = audioProcessor.apvts.getParameter (arpModeParamId))
        synthPanel.setArpModeIndex (juce::roundToInt (p->getValue() * 10.0f));

    if (auto* p = audioProcessor.apvts.getParameter (arpChordParamId))
        synthPanel.setArpChordIndex (juce::roundToInt (p->getValue() * 9.0f));
}

void MetuliferAudioProcessorEditor::parameterChanged (const juce::String& parameterID, float)
{
    if (parameterID == filterParamId)
    {
        updateFilterButtons();

        auto* param = audioProcessor.apvts.getParameter (parameterID);
        if (param != nullptr)
            showText ("filter " + juce::String (param->getValue() < 0.5f ? "LPF" : "HPF"));
    }
    else if (parameterID == reverbEnabledParamId || parameterID == reverbModeParamId
          || parameterID == delayEnabledParamId || parameterID == delayPingpongParamId
          || parameterID == reduxEnabledParamId)
    {
        updateFxButtons();
    }
    else if (parameterID == arpEnabledParamId || parameterID == arpModeParamId
          || parameterID == arpChordParamId)
    {
        updateArpButtons();
    }
    else if (parameterID.startsWith ("lfo"))
    {
        juce::MessageManager::callAsync ([this] { updateLfoButtons(); });
    }
}

//==============================================================================
void MetuliferAudioProcessorEditor::updateLfoButtons()
{
    for (int i = 0; i < 2; ++i)
    {
        auto& panel = (i == 0 ? lfoPanel1 : lfoPanel2);
        const auto prefix = "lfo" + juce::String (i + 1) + "_";
        if (auto* w = audioProcessor.apvts.getRawParameterValue (prefix + "wave"))
            panel.setWaveIndex ((int) std::round (w->load()));
        if (auto* e = audioProcessor.apvts.getRawParameterValue (prefix + "enabled"))
            panel.setEnabled (e->load() > 0.5f);
    }
}

void MetuliferAudioProcessorEditor::updateLfoDestinations()
{
    for (int i = 0; i < 2; ++i)
    {
        auto& panel = (i == 0 ? lfoPanel1 : lfoPanel2);
        const auto dests = cableLayer.getDestinationsForLfo (i);

        if (dests.isEmpty())
        {
            panel.setDestinationSummary ("NO TARGET");
        }
        else
        {
            juce::String summary;
            for (int d = 0; d < dests.size(); ++d)
            {
                if (d > 0) summary += ", ";
                summary += dests[d].replaceCharacter ('_', ' ').toUpperCase();
            }
            panel.setDestinationSummary (summary);
        }
    }
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

    // Update studio-grade accurate stereo dB meter with ballistics
    displayScreen.updateMeterLevels (audioProcessor.getPeakDbL(),
                                     audioProcessor.getPeakDbR(),
                                     audioProcessor.getRmsDbL(),
                                     audioProcessor.getRmsDbR());

    // Push real-time oscilloscope samples to the OLED display
    float scopeData[512];
    audioProcessor.getScopeBuffer (scopeData, 512);
    displayScreen.pushScopeSamples (scopeData, 512);

    // Update LFO real-time activity and rate readouts
    const float lfo0 = audioProcessor.getLfoValue (0);
    const float lfo1 = audioProcessor.getLfoValue (1);
    lfoPanel1.setLfoActivity (lfo0);
    lfoPanel2.setLfoActivity (lfo1);
    lfoPanel1.setRateHz ((float) lfoPanel1.rateKnob.getValue());
    lfoPanel2.setRateHz ((float) lfoPanel2.rateKnob.getValue());

    // Visually animate assigned knobs in time with the LFO
    const auto& activeCables = cableLayer.getVisibleCables();
    const bool lfo1On = lfoPanel1.isEnabled();
    const bool lfo2On = lfoPanel2.isEnabled();

    for (const auto& tk : targetKnobs)
    {
        if (tk.knob == nullptr)
            continue;

        int modulatingLfo = -1;
        for (const auto& c : activeCables)
        {
            if (c.suffix == tk.suffix)
            {
                modulatingLfo = c.lfo;
                break;
            }
        }

        const bool isModulated = (modulatingLfo == 0 && lfo1On) || (modulatingLfo == 1 && lfo2On);

        if (isModulated)
        {
            const float lfoVal = (modulatingLfo == 0 ? lfo0 : lfo1);
            const float baseNorm = (float) tk.knob->valueToProportionOfLength (tk.knob->getValue());
            const float modNorm = juce::jlimit (0.0f, 1.0f, baseNorm + lfoVal * 0.5f);

            const float prevMod = tk.knob->getProperties().contains ("mod_pos")
                                    ? (float) tk.knob->getProperties()["mod_pos"]
                                    : -999.0f;

            if (std::abs (modNorm - prevMod) >= 0.005f)
            {
                tk.knob->getProperties().set ("mod_pos", modNorm);
                tk.knob->getProperties().set ("mod_lfo", modulatingLfo);
                tk.knob->getProperties().set ("base_pos", baseNorm);
                tk.knob->repaint();
            }
        }
        else if (tk.knob->getProperties().contains ("mod_pos"))
        {
            tk.knob->getProperties().remove ("mod_pos");
            tk.knob->getProperties().remove ("mod_lfo");
            tk.knob->getProperties().remove ("base_pos");
            tk.knob->repaint();
        }
    }

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

bool MetuliferAudioProcessorEditor::keyPressed (const juce::KeyPress& key)
{
    if (key.getModifiers().isCommandDown() && (key.getKeyCode() == 'I' || key.getKeyCode() == 'i'))
    {
        inspector.toggle();
        return true;
    }

    return AudioProcessorEditor::keyPressed (key);
}
