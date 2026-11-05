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

        // Only close and minimise buttons for fixed-size window
        window->setTitleBarButtonsRequired (juce::DocumentWindow::closeButton | juce::DocumentWindow::minimiseButton, false);

        if (! window->isUsingNativeTitleBar())
            window->setUsingNativeTitleBar (true);

        constexpr int fixedWidth = 1280;
        constexpr int fixedHeight = 720;

        window->setResizable (false, false);

        if (editor != nullptr)
        {
            editor->setSize (fixedWidth, fixedHeight);
        }

        window->setContentComponentSize (fixedWidth, fixedHeight);

        auto& desktop = juce::Desktop::getInstance();
        auto* primaryScreen = desktop.getDisplays().getPrimaryDisplay();
        auto screenBounds = primaryScreen != nullptr ? primaryScreen->userBounds.toNearestInt()
                                                     : juce::Rectangle<int> (0, 0, 1920, 1080);

        if (window->getWidth() != fixedWidth || window->getHeight() < fixedHeight || ! screenBounds.intersects (window->getBounds()))
        {
            window->setContentComponentSize (fixedWidth, fixedHeight);
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

                    // Strictly lock window size on macOS Cocoa
                    nsWindow.styleMask &= ~NSWindowStyleMaskResizable;
                    [nsWindow setShowsResizeIndicator:NO];

                    NSButton* zoomButton = [nsWindow standardWindowButton:NSWindowZoomButton];
                    if (zoomButton != nil)
                        zoomButton.enabled = NO;

                    [nsWindow setContentSize:NSMakeSize (fixedWidth, fixedHeight)];
                    [nsWindow setMinSize:nsWindow.frame.size];
                    [nsWindow setMaxSize:nsWindow.frame.size];

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
