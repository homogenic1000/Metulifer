#include "CableLayer.h"

CableLayer::~CableLayer()
{
    if (modTree.isValid())
        modTree.removeListener (this);
}

void CableLayer::setModTree (juce::ValueTree tree, juce::UndoManager* um)
{
    if (modTree.isValid())
        modTree.removeListener (this);

    modTree = tree;
    undoManager = um;

    if (modTree.isValid())
        modTree.addListener (this);

    refresh();
    repaint();
    if (onCablesChanged)
        onCablesChanged();
}

void CableLayer::valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier&)
{
    refresh();
    repaint();
    if (onCablesChanged)
        onCablesChanged();
}

void CableLayer::valueTreeChildAdded (juce::ValueTree& parent, juce::ValueTree&)
{
    if (parent == modTree)
    {
        refresh();
        repaint();
        if (onCablesChanged)
            onCablesChanged();
    }
}

void CableLayer::valueTreeChildRemoved (juce::ValueTree& parent, juce::ValueTree&, int)
{
    if (parent == modTree)
    {
        refresh();
        repaint();
        if (onCablesChanged)
            onCablesChanged();
    }
}

void CableLayer::valueTreeChildOrderChanged (juce::ValueTree& parent, int, int)
{
    if (parent == modTree)
    {
        refresh();
        repaint();
        if (onCablesChanged)
            onCablesChanged();
    }
}

void CableLayer::valueTreeParentChanged (juce::ValueTree&) {}

void CableLayer::valueTreeRedirected (juce::ValueTree& tree)
{
    if (tree == modTree)
    {
        refresh();
        repaint();
        if (onCablesChanged)
            onCablesChanged();
    }
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
    if (onCablesChanged)
        onCablesChanged();
}

juce::StringArray CableLayer::getDestinationsForLfo (int lfoIndex) const
{
    juce::StringArray result;
    for (const auto& c : cables)
        if (c.lfo == lfoIndex)
            result.add (c.suffix);
    return result;
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
        int seqNum = -1;
        if (dest.startsWith ("seq"))
        {
            const auto rest = dest.substring (3);
            seqNum = rest.upToFirstOccurrenceOf ("_", false, false).getIntValue() - 1;
        }

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
        if (s.jack.expanded (7.0f).contains (p))
            return s.lfo;

    return -1;
}

const CableLayer::Target* CableLayer::targetAt (juce::Point<float> p) const
{
    for (const auto& t : targets)
        if (t.knob.expanded (5.0f).contains (p))
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
                    if (bezier (from, control, to, (float) i / 24.0f).getDistanceFrom (p) < 8.0f)
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

void CableLayer::mouseMove (const juce::MouseEvent& e)
{
    const auto p = e.position;
    if (sourceNear (p) >= 0 || cableNear (p) != nullptr)
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    else
        setMouseCursor (juce::MouseCursor::NormalCursor);
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
        {
            addCable (dragSource, "seq" + juce::String (selectedSeq + 1) + "_" + target->suffix);
            if (onCableConnected)
                onCableConnected (dragSource, target->suffix);
        }

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
            const int lfo = cable.lfo;
            const auto suffix = cable.suffix;
            modTree.removeChild (i, undoManager);
            if (onCableDisconnected)
                onCableDisconnected (lfo, suffix);
            return;
        }
    }
}

void CableLayer::paint (juce::Graphics& g)
{
    auto drawCable = [&g, this] (juce::Point<float> from, juce::Point<float> to,
                                 const juce::Colour& colour, float width)
    {
        const auto path = cablePath (from, to);

        // Drop shadow
        g.setColour (juce::Colour (0x55000000));
        g.strokePath (path, juce::PathStrokeType (width + 2.0f,
                                                 juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

        // Outer sheath
        g.setColour (colour.darker (0.25f));
        g.strokePath (path, juce::PathStrokeType (width + 0.5f,
                                                 juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

        // Core cable
        g.setColour (colour);
        g.strokePath (path, juce::PathStrokeType (width,
                                                 juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

        // Specular highlight line
        g.setColour (juce::Colours::white.withAlpha (0.4f));
        g.strokePath (path, juce::PathStrokeType (juce::jmax (0.8f, width * 0.25f),
                                                 juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

        // Plug heads at endpoints
        for (const auto& p : { from, to })
        {
            g.setColour (juce::Colour (0xff1f242d));
            g.fillEllipse (p.x - 5.0f, p.y - 5.0f, 10.0f, 10.0f);
            g.setColour (colour);
            g.drawEllipse (p.x - 5.0f, p.y - 5.0f, 10.0f, 10.0f, 1.5f);
            g.setColour (juce::Colours::white);
            g.fillEllipse (p.x - 1.5f, p.y - 1.5f, 3.0f, 3.0f);
        }
    };

    for (const auto& c : cables)
    {
        const juce::Colour cableCol = (c.lfo == 0) ? juce::Colour (0xffff8c24) : juce::Colour (0xff00d4ff);

        for (const auto& s : sources)
        {
            if (s.lfo != c.lfo)
                continue;

            for (const auto& t : targets)
            {
                if (t.suffix == c.suffix)
                {
                    drawCable (s.jack.getCentre(), t.knob.getCentre(), cableCol, 2.5f);

                    // Destination knob halo & badge
                    g.setColour (cableCol.withAlpha (0.45f));
                    g.drawRoundedRectangle (t.knob.expanded (3.0f), 6.0f, 1.8f);

                    // Badge pill L1 or L2
                    const juce::Rectangle<float> badge (t.knob.getRight() - 16.0f, t.knob.getY() - 3.0f, 17.0f, 11.0f);
                    g.setColour (cableCol);
                    g.fillRoundedRectangle (badge, 3.0f);
                    g.setColour (juce::Colours::black);
                    g.setFont (juce::FontOptions (8.5f, juce::Font::bold));
                    g.drawText (c.lfo == 0 ? "L1" : "L2", badge, juce::Justification::centred, false);
                }
            }
        }
    }

    if (dragSource >= 0)
    {
        const juce::Colour dragCol = (dragSource == 0) ? juce::Colour (0xffff8c24) : juce::Colour (0xff00d4ff);

        for (const auto& s : sources)
        {
            if (s.lfo == dragSource)
            {
                if (auto* target = targetAt (mousePos))
                {
                    // Target highlight
                    g.setColour (juce::Colour (0xff10b981));
                    g.fillRoundedRectangle (target->knob.expanded (4.0f), 6.0f);
                    g.setColour (juce::Colours::white);
                    g.drawRoundedRectangle (target->knob.expanded (4.0f), 6.0f, 2.0f);

                    drawCable (s.jack.getCentre(), target->knob.getCentre(), juce::Colour (0xff10b981), 2.8f);
                }
                else
                {
                    drawCable (s.jack.getCentre(), mousePos, dragCol.withAlpha (0.75f), 2.2f);
                }
            }
        }
    }
}
