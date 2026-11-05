#include "NativeTitleBar_mac.h"

#if JUCE_MAC
#import <AppKit/AppKit.h>
#endif

namespace NativeTitleBar
{
    void setupNativeTitleBar (juce::DocumentWindow* window, juce::Component* editor)
    {
        if (window == nullptr)
            return;

        window->setTitleBarButtonsRequired (juce::DocumentWindow::allButtons, false);

        if (! window->isUsingNativeTitleBar())
            window->setUsingNativeTitleBar (true);

        constexpr int defaultWidth = 1280;
        constexpr int defaultHeight = 720;

        window->setResizable (true, false);
        window->setResizeLimits (1060, 680, 2560, 1440);

        if (editor != nullptr)
        {
            if (editor->getWidth() < 1000 || editor->getHeight() < 600)
                editor->setSize (defaultWidth, defaultHeight);
        }

        auto& desktop = juce::Desktop::getInstance();
        auto* primaryScreen = desktop.getDisplays().getPrimaryDisplay();
        auto screenBounds = primaryScreen != nullptr ? primaryScreen->userBounds.toNearestInt()
                                                     : juce::Rectangle<int> (0, 0, 1920, 1080);

        if (window->getWidth() < 1200 || window->getHeight() < 680 || ! screenBounds.intersects (window->getBounds()))
        {
            window->setContentComponentSize (defaultWidth, defaultHeight);
            window->centreWithSize (window->getWidth(), window->getHeight());
        }

       #if JUCE_MAC
        if (auto* peer = window->getPeer())
        {
            if (auto* view = (NSView*) peer->getNativeHandle())
            {
                if (NSWindow* nsWindow = [view window])
                {
                    if (@available (macOS 10.14, *))
                        nsWindow.appearance = [NSAppearance appearanceNamed:NSAppearanceNameDarkAqua];

                    nsWindow.title = @"Metulifer";
                    nsWindow.titleVisibility = NSWindowTitleVisible;
                    nsWindow.titlebarAppearsTransparent = NO;

                    // Enable zoom and resizing on Cocoa NSWindow
                    nsWindow.styleMask |= NSWindowStyleMaskResizable;
                    NSButton* zoomButton = [nsWindow standardWindowButton:NSWindowZoomButton];
                    if (zoomButton != nil)
                        zoomButton.enabled = YES;

                    [nsWindow display];
                }
            }
        }
       #endif
    }

    void openSettingsMenu (juce::DocumentWindow* window, juce::Component* targetComponent)
    {
        if (window == nullptr)
            return;

        for (auto* child : window->getChildren())
        {
            if (auto* btn = dynamic_cast<juce::Button*> (child))
            {
                if (btn->getButtonText() == "Options")
                {
                    if (targetComponent != nullptr)
                    {
                        auto targetBoundsInWindow = window->getLocalArea (targetComponent, targetComponent->getLocalBounds());
                        btn->setBounds (targetBoundsInWindow);
                    }
                    btn->triggerClick();
                    return;
                }
            }
        }
    }
}
