#pragma once

#include <JuceHeader.h>

class CableLayer : public juce::Component
{
public:
    CableLayer() = default;

    struct Source
    {
        int lfo = 0;
        juce::Rectangle<float> jack; // coordonnées éditeur
    };

    struct Target
    {
        juce::String suffix;          // ex. "cutoff"
        juce::Rectangle<float> knob;  // coordonnées éditeur
    };

    void setModTree (juce::ValueTree tree, juce::UndoManager* um);
    void setSources (std::vector<Source> newSources);
    void setTargets (std::vector<Target> newTargets);

    /** Recalcule les câbles visibles pour la séquence sélectionnée. */
    void setSelectedSequence (int seqIndex);

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    bool hitTest (int x, int y) override;

private:
    struct Cable
    {
        int lfo = 0;
        juce::String suffix;
        juce::String destId;
    };

    static juce::Path cablePath (juce::Point<float> from, juce::Point<float> to);
    const Cable* cableNear (juce::Point<float> p) const;
    int sourceNear (juce::Point<float> p) const;
    const Target* targetAt (juce::Point<float> p) const;
    void refresh();

    void addCable (int lfo, const juce::String& destId);
    void removeCable (const Cable& cable);

    juce::ValueTree modTree;
    juce::UndoManager* undoManager = nullptr;
    std::vector<Source> sources;
    std::vector<Target> targets;
    std::vector<Cable> cables;   // câbles de la séquence visible
    int selectedSeq = 0;

    int dragSource = -1;
    juce::Point<float> mousePos;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CableLayer)
};
