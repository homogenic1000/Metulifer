#include "Lever.h"

Lever::Lever()
{
    setOpaque (false);
}

void Lever::setParam (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId)
{
    attachment.reset();

    auto* param = apvts.getParameter (paramId);

    if (param == nullptr)
        return;

    attachment = std::make_unique<juce::ParameterAttachment> (
        *param,
        [this] (float denormalised)
        {
            setValue (juce::roundToInt (denormalised));
        },
        apvts.undoManager);

    attachment->sendInitialUpdate();
}

int Lever::positionFromY (float y) const
{
    const float rel = juce::jlimit (0.0f, 1.0f, y / (float) juce::jmax (1, getHeight()));
    const int fromTop = juce::roundToInt (rel * (float) (numPositions - 1));
    return 2 - fromTop;
}

void Lever::setValue (int newValue)
{
    const int clamped = juce::jlimit (-2, 2, newValue);

    if (value == clamped)
        return;

    value = clamped;
    repaint();
}

void Lever::applyFromMouse (const juce::MouseEvent& e)
{
    const int newPos = positionFromY ((float) e.y);

    if (newPos == value)
        return;

    if (attachment != nullptr)
        attachment->setValueAsCompleteGesture ((float) newPos);
    else
        setValue (newPos);

    if (onValueChanged)
        onValueChanged (newPos);
}

void Lever::mouseDown (const juce::MouseEvent& e)
{
    applyFromMouse (e);
}

void Lever::mouseDrag (const juce::MouseEvent& e)
{
    applyFromMouse (e);
}

void Lever::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat().reduced (0.5f);

    // Frame with drop shadow
    g.setColour (juce::Colour (0x15000000));
    g.fillRoundedRectangle (area.translated (0.0f, 1.0f), 5.0f);

    // Slot recess
    g.setColour (juce::Colour (0xff20242c));
    g.fillRoundedRectangle (area, 4.0f);

    g.setColour (juce::Colour (0xff475569));
    g.drawRoundedRectangle (area, 4.0f, 1.0f);

    // Vertical center groove
    const float trackW = 4.0f;
    const float trackX = area.getCentreX() - trackW * 0.5f;
    g.setColour (juce::Colour (0xff14171d));
    g.fillRoundedRectangle (trackX, area.getY() + 6.0f, trackW, area.getHeight() - 12.0f, 2.0f);

    // 5 Position tick marks
    const float travel = area.getHeight() - 20.0f;
    for (int i = 0; i < numPositions; ++i)
    {
        const float t = (float) i / 4.0f;
        const float tickY = area.getY() + 10.0f + (1.0f - t) * travel;
        const bool isCurrent = (i == (value + 2));
        g.setColour (isCurrent ? juce::Colour (0xff60a5fa) : juce::Colour (0xff475569));
        g.fillRect (area.getX() + 2.0f, tickY - 0.5f, 3.5f, 1.0f);
        g.fillRect (area.getRight() - 5.5f, tickY - 0.5f, 3.5f, 1.0f);
    }

    // Handle
    const float t = (float) (value + 2) / 4.0f;
    const float handleH = 16.0f;
    const float handleY = area.getY() + 2.0f + (1.0f - t) * travel;
    auto handleRect = juce::Rectangle<float> (area.getX() + 2.0f, handleY,
                                              area.getWidth() - 4.0f, handleH);

    juce::ColourGradient handleGrad (juce::Colour (0xff475569), handleRect.getCentreX(), handleRect.getY(),
                                     juce::Colour (0xff1e293b), handleRect.getCentreX(), handleRect.getBottom(), false);
    g.setGradientFill (handleGrad);
    g.fillRoundedRectangle (handleRect, 3.0f);

    // Handle center accent indicator
    g.setColour (juce::Colour (0xff3b82f6));
    g.fillRect (handleRect.getX() + 3.0f, handleRect.getCentreY() - 1.0f,
                handleRect.getWidth() - 6.0f, 2.0f);

    g.setColour (juce::Colour (0xff94a3b8));
    g.drawRoundedRectangle (handleRect, 3.0f, 1.0f);
}
