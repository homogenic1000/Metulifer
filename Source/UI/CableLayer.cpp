#include "CableLayer.h"

void CableLayer::setModTree (juce::ValueTree tree, juce::UndoManager* um)
{
    modTree = tree;
    undoManager = um;
    refresh();
}

void CableLayer::setSources (std::vector<Source> newSources)
{
    sources = std::move (newSources);
    repaint();
}

void CableLayer::setTargets (std::vector<Target> newTargets)
{
    targets = std::move (newTargets);
    repaint();
}

void CableLayer::setSelectedSequence (int seqIndex)
{
    selectedSeq = juce::jlimit (0, 5, seqIndex);
    refresh();
    repaint();
}

void CableLayer::refresh()
{
    cables.clear();

    if (! modTree.isValid())
        return;

    for (int i = 0; i < modTree.getNumChildren(); ++i)
    {
        const auto child = modTree.getChild (i);

        if (! child.hasType ("cable"))
            continue;

        const juce::String dest = child.getProperty ("dest").toString();
        const auto seqNum = dest.upToFirstOccurrenceOf ("_", false, true).getIntValue() - 1;
        const auto suffix = dest.fromFirstOccurrenceOf ("_", false, false);

        if (seqNum == selectedSeq && suffix.isNotEmpty())
            cables.push_back ({ juce::jlimit (0, 1, (int) child.getProperty ("src", 0)),
                                suffix, dest });
    }
}

juce::Path CableLayer::cablePath (juce::Point<float> from, juce::Point<float> to)
{
    juce::Path p;
    p.startNewSubPath (from);
    const auto control = (from + to) * 0.5f + juce::Point<float> (0.0f, -60.0f);
    p.quadraticTo (control, to);
    return p;
}

int CableLayer::sourceNear (juce::Point<float> p) const
{
    for (const auto& s : sources)
        if (s.jack.expanded (6.0f).contains (p))
            return s.lfo;

    return -1;
}

const CableLayer::Target* CableLayer::targetAt (juce::Point<float> p) const
{
    for (const auto& t : targets)
        if (t.knob.expanded (4.0f).contains (p))
            return &t;

    return nullptr;
}

const CableLayer::Cable* CableLayer::cableNear (juce::Point<float> p) const
{
    auto bezier = [] (juce::Point<float> from, juce::Point<float> control,
                      juce::Point<float> to, float t)
    {
        const float u = 1.0f - t;
        return from * (u * u) + control * (2.0f * u * t) + to * (t * t);
    };

    for (const auto& c : cables)
    {
        for (const auto& s : sources)
        {
            if (s.lfo != c.lfo)
                continue;

            for (const auto& t : targets)
            {
                if (t.suffix != c.suffix)
                    continue;

                const auto from = s.jack.getCentre();
                const auto to = t.knob.getCentre();
                const auto control = (from + to) * 0.5f + juce::Point<float> (0.0f, -60.0f);

                for (int i = 0; i <= 24; ++i)
                    if (bezier (from, control, to, (float) i / 24.0f).getDistanceFrom (p) < 6.0f)
                        return &c;
            }
        }
    }

    return nullptr;
}

bool CableLayer::hitTest (int x, int y)
{
    if (dragSource >= 0)
        return true;

    const juce::Point<float> p ((float) x, (float) y);

    if (sourceNear (p) >= 0)
        return true;

    return cableNear (p) != nullptr;
}

void CableLayer::mouseDown (const juce::MouseEvent& e)
{
    const auto p = e.position;

    if (auto* cable = cableNear (p))
    {
        removeCable (*cable);
        return;
    }

    dragSource = sourceNear (p);
    mousePos = p;
}

void CableLayer::mouseDrag (const juce::MouseEvent& e)
{
    mousePos = e.position;
    repaint();
}

void CableLayer::mouseUp (const juce::MouseEvent& e)
{
    mousePos = e.position;

    if (dragSource >= 0)
    {
        if (auto* target = targetAt (mousePos))
            addCable (dragSource, "seq" + juce::String (selectedSeq + 1) + "_" + target->suffix);

        dragSource = -1;
        repaint();
    }
}

void CableLayer::addCable (int lfo, const juce::String& destId)
{
    if (! modTree.isValid())
        return;

    for (int i = 0; i < modTree.getNumChildren(); ++i)
        if (modTree.getChild (i).getProperty ("dest").toString() == destId)
            return; // un seul LFO par destination

    juce::ValueTree cable { "cable" };
    cable.setProperty ("src", lfo, nullptr);
    cable.setProperty ("dest", destId, nullptr);
    modTree.appendChild (cable, undoManager);
}

void CableLayer::removeCable (const Cable& cable)
{
    if (! modTree.isValid())
        return;

    for (int i = 0; i < modTree.getNumChildren(); ++i)
    {
        const auto child = modTree.getChild (i);

        if (child.getProperty ("dest").toString() == cable.destId)
        {
            modTree.removeChild (i, undoManager);
            return;
        }
    }
}

void CableLayer::paint (juce::Graphics& g)
{
    auto drawCable = [&g] (juce::Point<float> from, juce::Point<float> to,
                           const juce::Colour& colour, float width)
    {
        g.setColour (colour);
        g.strokePath (cablePath (from, to),
                      juce::PathStrokeType (width, juce::PathStrokeType::curved,
                                            juce::PathStrokeType::rounded));
    };

    for (const auto& c : cables)
    {
        for (const auto& s : sources)
        {
            if (s.lfo != c.lfo)
                continue;

            for (const auto& t : targets)
                if (t.suffix == c.suffix)
                    drawCable (s.jack.getCentre(), t.knob.getCentre(),
                               juce::Colour (0xffff7a1a), 2.0f);
        }
    }

    if (dragSource >= 0)
    {
        for (const auto& s : sources)
            if (s.lfo == dragSource)
            {
                if (auto* target = targetAt (mousePos))
                {
                    g.setColour (juce::Colour (0xff3aa63a));
                    g.drawRoundedRectangle (target->knob.expanded (2.0f), 5.0f, 2.0f);
                    drawCable (s.jack.getCentre(), target->knob.getCentre(),
                               juce::Colour (0xff3aa63a), 2.0f);
                }
                else
                {
                    drawCable (s.jack.getCentre(), mousePos,
                               juce::Colour (0x88888888), 2.0f);
                }
            }
    }
}
