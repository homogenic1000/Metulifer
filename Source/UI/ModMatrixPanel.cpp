#include "ModMatrixPanel.h"
#include "../PluginProcessor.h"

//==============================================================================
MatrixCell::MatrixCell (MetuliferAudioProcessor& p, int seqIndex, int dest, int src)
    : processor (p), seq (seqIndex), destIndex (dest), srcIndex (src)
{
    setRepaintsOnMouseActivity (true);
    updateValue();
}

void MatrixCell::setLocation (int seqIndex, int dest, int src)
{
    seq = seqIndex;
    destIndex = dest;
    srcIndex = src;
    updateValue();
}

void MatrixCell::updateValue()
{
    currentAmount = processor.getMatrixAmount (seq, destIndex, srcIndex);
    repaint();
}

void MatrixCell::applyDelta (float delta)
{
    const float newAmt = juce::jlimit (-1.0f, 1.0f, currentAmount + delta);
    if (std::abs (newAmt - currentAmount) > 0.0005f)
    {
        processor.setMatrixAmount (seq, destIndex, srcIndex, newAmt);
        currentAmount = newAmt;
        repaint();
        if (onValueChanged)
            onValueChanged();
    }
}

void MatrixCell::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat().reduced (1.0f);
    const bool isHover = isMouseOver();

    // Cell background
    juce::Colour bg = isDragging ? juce::Colour (0xff252530)
                                 : (isHover ? juce::Colour (0xff202028)
                                            : juce::Colour (0xff15151b));
    g.setColour (bg);
    g.fillRoundedRectangle (bounds, 3.0f);

    // Border
    g.setColour (isDragging ? juce::Colour (0xff38bdf8)
                            : (isHover ? juce::Colour (0xff3f3f4e)
                                       : juce::Colour (0xff262630)));
    g.drawRoundedRectangle (bounds, 3.0f, 1.0f);

    const float midX = bounds.getCentreX();
    const float halfW = bounds.getWidth() * 0.5f - 4.0f;

    // Center subtle zero notch
    g.setColour (juce::Colour (0x28ffffff));
    g.drawVerticalLine ((int) midX, bounds.getY() + 3.0f, bounds.getBottom() - 3.0f);

    // Active modulation fill
    if (std::abs (currentAmount) > 0.005f)
    {
        if (currentAmount > 0.0f)
        {
            const float fillW = halfW * currentAmount;
            const auto fillRect = juce::Rectangle<float> (midX, bounds.getY() + 2.0f, fillW, bounds.getHeight() - 4.0f);
            g.setColour (juce::Colour (0x3322d3ee));
            g.fillRoundedRectangle (fillRect, 2.0f);
        }
        else
        {
            const float fillW = halfW * (-currentAmount);
            const auto fillRect = juce::Rectangle<float> (midX - fillW, bounds.getY() + 2.0f, fillW, bounds.getHeight() - 4.0f);
            g.setColour (juce::Colour (0x33f59e0b));
            g.fillRoundedRectangle (fillRect, 2.0f);
        }
    }

    // Text display
    g.setFont (juce::FontOptions (10.5f).withStyle ("Bold"));

    if (std::abs (currentAmount) <= 0.005f)
    {
        g.setColour (juce::Colour (0xff52525b));
        g.drawText ("--", bounds, juce::Justification::centred, false);
    }
    else if (currentAmount > 0.0f)
    {
        const int pct = juce::roundToInt (currentAmount * 100.0f);
        g.setColour (juce::Colour (0xff22d3ee));
        g.drawText ("+" + juce::String (pct) + "%", bounds, juce::Justification::centred, false);
    }
    else
    {
        const int pct = juce::roundToInt (currentAmount * 100.0f);
        g.setColour (juce::Colour (0xfff59e0b));
        g.drawText (juce::String (pct) + "%", bounds, juce::Justification::centred, false);
    }
}

void MatrixCell::mouseDown (const juce::MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        juce::PopupMenu menu;
        menu.addItem (1, "Reset to 0%");
        menu.addSeparator();
        menu.addItem (2, "Set to +100%");
        menu.addItem (3, "Set to +50%");
        menu.addItem (4, "Set to -50%");
        menu.addItem (5, "Set to -100%");

        menu.showMenuAsync (juce::PopupMenu::Options(), [this] (int result)
        {
            if (result == 1) applyDelta (-currentAmount);
            else if (result == 2) applyDelta (1.0f - currentAmount);
            else if (result == 3) applyDelta (0.5f - currentAmount);
            else if (result == 4) applyDelta (-0.5f - currentAmount);
            else if (result == 5) applyDelta (-1.0f - currentAmount);
        });
        return;
    }

    dragStartPos = e.getPosition();
    dragStartAmount = currentAmount;
    isDragging = true;
    repaint();
}

void MatrixCell::mouseDrag (const juce::MouseEvent& e)
{
    if (! isDragging)
        return;

    const float dy = (float) (dragStartPos.y - e.y);
    const float sens = e.mods.isShiftDown() ? 0.002f : 0.008f;
    const float newAmt = juce::jlimit (-1.0f, 1.0f, dragStartAmount + dy * sens);

    if (std::abs (newAmt - currentAmount) > 0.001f)
    {
        processor.setMatrixAmount (seq, destIndex, srcIndex, newAmt);
        currentAmount = newAmt;
        repaint();
        if (onValueChanged)
            onValueChanged();
    }
}

void MatrixCell::mouseUp (const juce::MouseEvent&)
{
    if (isDragging)
    {
        isDragging = false;
        repaint();
    }
}

void MatrixCell::mouseDoubleClick (const juce::MouseEvent&)
{
    processor.setMatrixAmount (seq, destIndex, srcIndex, 0.0f);
    currentAmount = 0.0f;
    repaint();
    if (onValueChanged)
        onValueChanged();
}

void MatrixCell::mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    const float delta = wheel.deltaY * (e.mods.isShiftDown() ? 0.02f : 0.05f);
    applyDelta (delta);
}

//==============================================================================
ModMatrixPanel::ModMatrixPanel (MetuliferAudioProcessor& p)
    : processor (p)
{
    // Title
    titleLabel.setFont (juce::FontOptions (12.0f).withStyle ("Bold"));
    titleLabel.setColour (juce::Label::textColourId, juce::Colour (0xffe4e4e7));
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (titleLabel);

    // Sequence tabs S1..S6
    for (int i = 0; i < 6; ++i)
    {
        auto& btn = seqButtons[i];
        btn.setButtonText ("S" + juce::String (i + 1));
        btn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1f1f26));
        btn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffa1a1aa));
        btn.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff0284c7));
        btn.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
        btn.setClickingTogglesState (false);
        btn.onClick = [this, i]
        {
            setSelectedSequence (i);
            if (onSequenceChanged)
                onSequenceChanged (i);
        };
        addAndMakeVisible (btn);
    }

    // Filter mode tabs
    const char* filterNames[3] = { "ALL", "SYNTH", "FX" };
    for (int i = 0; i < 3; ++i)
    {
        auto& btn = filterButtons[i];
        btn.setButtonText (filterNames[i]);
        btn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1c1c24));
        btn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff71717a));
        btn.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff334155));
        btn.setColour (juce::TextButton::textColourOnId, juce::Colour (0xff38bdf8));
        btn.setClickingTogglesState (false);
        btn.onClick = [this, i] { setFilterMode ((FilterMode) i); };
        addAndMakeVisible (btn);
    }

    // Clear Sequence button
    clearButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff272730));
    clearButton.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff94a3b8));
    clearButton.onClick = [this] { clearCurrentSequenceMatrix(); };
    addAndMakeVisible (clearButton);

    // Close button
    closeButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff272730));
    closeButton.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffe4e4e7));
    closeButton.onClick = [this]
    {
        if (onCloseRequested)
            onCloseRequested();
    };
    addAndMakeVisible (closeButton);

    // Viewport setup
    viewport.setViewedComponent (&gridContent, false);
    viewport.setScrollBarsShown (true, false, false, false);
    viewport.setScrollBarThickness (8);
    addAndMakeVisible (viewport);

    // Instantiate all 15 x 6 = 90 cells
    cells.reserve (ModMatrix::numDestinations * ModMatrix::numSources);
    for (int d = 0; d < ModMatrix::numDestinations; ++d)
    {
        for (int s = 0; s < ModMatrix::numSources; ++s)
        {
            auto cell = std::make_unique<MatrixCell> (processor, currentSeq, d, s);
            gridContent.addAndMakeVisible (cell.get());
            cells.push_back (std::move (cell));
        }
    }

    setSelectedSequence (0);
    setFilterMode (filterAll);
}

void ModMatrixPanel::setSelectedSequence (int seqIndex)
{
    currentSeq = juce::jlimit (0, 5, seqIndex);

    for (int i = 0; i < 6; ++i)
    {
        seqButtons[i].setColour (juce::TextButton::buttonColourId,
                                 i == currentSeq ? juce::Colour (0xff0284c7) : juce::Colour (0xff1f1f26));
        seqButtons[i].setColour (juce::TextButton::textColourOffId,
                                 i == currentSeq ? juce::Colours::white : juce::Colour (0xffa1a1aa));
        seqButtons[i].repaint();
    }

    refreshCells();
}

void ModMatrixPanel::setFilterMode (FilterMode mode)
{
    currentFilter = mode;

    for (int i = 0; i < 3; ++i)
    {
        const bool active = (i == (int) currentFilter);
        filterButtons[i].setColour (juce::TextButton::buttonColourId,
                                    active ? juce::Colour (0xff334155) : juce::Colour (0xff1c1c24));
        filterButtons[i].setColour (juce::TextButton::textColourOffId,
                                    active ? juce::Colour (0xff38bdf8) : juce::Colour (0xff71717a));
        filterButtons[i].repaint();
    }

    resized();
}

bool ModMatrixPanel::isDestVisible (int dest) const
{
    switch (currentFilter)
    {
        case filterSynth: return dest <= ModMatrix::dstMix2;
        case filterFx:    return dest >= ModMatrix::dstDecay1;
        case filterAll:
        default:          return true;
    }
}

void ModMatrixPanel::clearCurrentSequenceMatrix()
{
    for (int d = 0; d < ModMatrix::numDestinations; ++d)
        for (int s = 0; s < ModMatrix::numSources; ++s)
            processor.setMatrixAmount (currentSeq, d, s, 0.0f);

    refreshCells();
}

void ModMatrixPanel::refreshCells()
{
    for (auto& cell : cells)
    {
        if (cell != nullptr)
        {
            // Update cell sequence pointer
            cell->setLocation (currentSeq, cell->getProperties()["dest"], cell->getProperties()["src"]);
            cell->updateValue();
        }
    }
}

void ModMatrixPanel::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    // Dark sleek background
    g.setColour (juce::Colour (0xf00e0e12));
    g.fillRoundedRectangle (bounds, 6.0f);

    // Accent header line
    g.setColour (juce::Colour (0xff0284c7));
    g.fillRect (bounds.getX() + 6.0f, bounds.getY() + 31.0f, bounds.getWidth() - 12.0f, 1.5f);

    // Border
    g.setColour (juce::Colour (0xff272730));
    g.drawRoundedRectangle (bounds, 6.0f, 1.0f);
}

void ModMatrixPanel::resized()
{
    const int margin = 8;
    const int headerH = 26;

    // Header layout
    titleLabel.setBounds (margin + 4, margin, 170, headerH);

    int tabX = titleLabel.getRight() + 10;
    for (int i = 0; i < 6; ++i)
    {
        seqButtons[i].setBounds (tabX, margin, 32, headerH);
        tabX += 34;
    }

    tabX += 16;
    for (int i = 0; i < 3; ++i)
    {
        const int w = (i == 0 ? 40 : 54);
        filterButtons[i].setBounds (tabX, margin, w, headerH);
        tabX += w + 2;
    }

    closeButton.setBounds (getWidth() - margin - 26, margin, 26, headerH);
    clearButton.setBounds (closeButton.getX() - 8 - 74, margin, 74, headerH);

    // Viewport layout below header
    const int vpY = margin + headerH + 6;
    const int vpH = getHeight() - vpY - margin;
    const int vpW = getWidth() - margin * 2;
    viewport.setBounds (margin, vpY, vpW, vpH);

    // Calculate content height
    int visibleRows = 0;
    for (int d = 0; d < ModMatrix::numDestinations; ++d)
        if (isDestVisible (d))
            ++visibleRows;

    const int rowH = 23;
    const int colHeaderH = 22;
    const int contentH = colHeaderH + visibleRows * rowH + 6;
    const int contentW = viewport.getMaximumVisibleWidth();

    gridContent.setBounds (0, 0, contentW, juce::jmax (vpH, contentH));
    gridContent.resized();
}

//==============================================================================
void ModMatrixPanel::GridContent::paint (juce::Graphics& g)
{
    const int labelColW = 140;
    const int colHeaderH = 22;
    const int numSrc = ModMatrix::numSources;
    const float cellW = (float) (getWidth() - labelColW - 10) / (float) numSrc;

    // Header background
    g.setColour (juce::Colour (0xff181820));
    g.fillRect (0, 0, getWidth(), colHeaderH);

    // Column headers
    g.setFont (juce::FontOptions (10.0f).withStyle ("Bold"));
    g.setColour (juce::Colour (0xff71717a));
    g.drawText ("DESTINATION", 8, 0, labelColW - 8, colHeaderH, juce::Justification::centredLeft, false);

    for (int s = 0; s < numSrc; ++s)
    {
        const float x = (float) labelColW + (float) s * cellW;
        g.setColour (juce::Colour (0xff38bdf8));
        g.drawText (ModMatrix::getSourceName (s), (int) x, 0, (int) cellW, colHeaderH, juce::Justification::centred, false);
    }

    // Row destination labels and alternating stripes
    const int rowH = 23;
    int currentY = colHeaderH + 2;

    for (int d = 0; d < ModMatrix::numDestinations; ++d)
    {
        if (! owner.isDestVisible (d))
            continue;

        // Alternating background stripe
        if ((d % 2) == 1)
        {
            g.setColour (juce::Colour (0x10ffffff));
            g.fillRect (0, currentY, getWidth(), rowH);
        }

        g.setFont (juce::FontOptions (10.5f));
        g.setColour (juce::Colour (0xffe4e4e7));
        g.drawText (ModMatrix::getDestName (d), 8, currentY, labelColW - 8, rowH, juce::Justification::centredLeft, false);

        currentY += rowH;
    }
}

void ModMatrixPanel::GridContent::resized()
{
    const int labelColW = 140;
    const int colHeaderH = 22;
    const int numSrc = ModMatrix::numSources;
    const float cellW = (float) (getWidth() - labelColW - 10) / (float) numSrc;
    const int rowH = 23;

    int cellIdx = 0;
    int currentY = colHeaderH + 2;

    for (int d = 0; d < ModMatrix::numDestinations; ++d)
    {
        const bool visible = owner.isDestVisible (d);

        for (int s = 0; s < numSrc; ++s)
        {
            if (cellIdx < (int) owner.cells.size())
            {
                auto* cell = owner.cells[(size_t) cellIdx].get();
                cell->getProperties().set ("dest", d);
                cell->getProperties().set ("src", s);
                cell->setLocation (owner.getSelectedSequence(), d, s);

                if (visible)
                {
                    const float x = (float) labelColW + (float) s * cellW;
                    cell->setBounds ((int) x + 1, currentY + 1, (int) cellW - 2, rowH - 2);
                    cell->setVisible (true);
                }
                else
                {
                    cell->setVisible (false);
                }
            }
            ++cellIdx;
        }

        if (visible)
            currentY += rowH;
    }
}
