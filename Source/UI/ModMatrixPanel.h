#pragma once

#include <JuceHeader.h>
#include "../DSP/ModMatrix.h"

class MetuliferAudioProcessor;

//==============================================================================
/**
 * Interactive modulation matrix cell displaying bipolar amount (-100% to +100%).
 * Supports vertical drag, shift for fine control, double-click to reset, right-click menu,
 * and mouse-wheel tweaking.
 */
class MatrixCell : public juce::Component
{
public:
    MatrixCell (MetuliferAudioProcessor& processor, int seqIndex, int dest, int src);

    void setLocation (int seqIndex, int dest, int src);
    void updateValue();

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    void mouseDoubleClick (const juce::MouseEvent& e) override;
    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

    std::function<void()> onValueChanged;

private:
    void applyDelta (float delta);

    MetuliferAudioProcessor& processor;
    int seq = 0;
    int destIndex = 0;
    int srcIndex = 0;
    float currentAmount = 0.0f;
    float dragStartAmount = 0.0f;
    juce::Point<int> dragStartPos;
    bool isDragging = false;
};

//==============================================================================
/**
 * Ableton-style Modulation Matrix Panel.
 * Allows routing any of the 6 modulation sources (LFO1, LFO2, Env1, Env2, Key, Velocity)
 * to 15 synthesis/FX destinations with bipolar depths (-100% to +100%).
 */
class ModMatrixPanel : public juce::Component
{
public:
    explicit ModMatrixPanel (MetuliferAudioProcessor& processor);
    ~ModMatrixPanel() override = default;

    void setSelectedSequence (int seqIndex);
    int getSelectedSequence() const noexcept { return currentSeq; }

    void refreshCells();

    void paint (juce::Graphics& g) override;
    void resized() override;

    std::function<void()> onCloseRequested;
    std::function<void(int)> onSequenceChanged;

private:
    enum FilterMode
    {
        filterAll = 0,
        filterSynth,
        filterFx
    };

    void setFilterMode (FilterMode mode);
    bool isDestVisible (int dest) const;
    void clearCurrentSequenceMatrix();

    MetuliferAudioProcessor& processor;
    int currentSeq = 0;
    FilterMode currentFilter = filterAll;

    // Header controls
    juce::Label titleLabel { "title", "MODULATION MATRIX" };
    juce::TextButton seqButtons[6];
    juce::TextButton filterButtons[3];
    juce::TextButton clearButton { "CLEAR" };
    juce::TextButton closeButton { juce::CharPointer_UTF8 ("\xc3\x97") }; // "×"

    // Grid components
    struct GridContent : public juce::Component
    {
        ModMatrixPanel& owner;
        explicit GridContent (ModMatrixPanel& p) : owner (p) {}
        void paint (juce::Graphics& g) override;
        void resized() override;
    };

    GridContent gridContent { *this };
    juce::Viewport viewport;

    std::vector<std::unique_ptr<MatrixCell>> cells;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModMatrixPanel)
};
