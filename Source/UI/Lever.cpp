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
    auto area = getLocalBounds().toFloat();

    g.setColour (juce::Colour (0xffc4c4c4));
    g.fillRoundedRectangle (area, 5.0f);

    g.setColour (juce::Colour (0xff9a9a9a));
    g.drawRoundedRectangle (area.reduced (0.5f), 5.0f, 1.0f);

    const float travel = area.getHeight() - 22.0f;
    const float t = (float) (value + 2) / 4.0f;
    const float handleY = area.getY() + (1.0f - t) * travel;

    g.setColour (juce::Colour (0xff3a7bd5));
    g.fillRoundedRectangle (area.getX() + 3.0f, handleY + 3.0f,
                            area.getWidth() - 6.0f, 16.0f, 4.0f);

    g.setColour (juce::Colour (0xffffffff));
    g.drawRoundedRectangle (juce::Rectangle<float> (area.getX() + 3.0f, handleY + 3.0f,
                                                    area.getWidth() - 6.0f, 16.0f),
                            4.0f, 1.0f);
}
