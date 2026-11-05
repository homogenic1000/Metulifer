#include "NativeTitleBar_mac.h"

#if JUCE_MAC
#import <AppKit/AppKit.h>
#endif

namespace NativeTitleBar
{
    void setupNativeTitleBar (juce::DocumentWindow* window)
    {
        if (window == nullptr)
            return;

        window->setTitleBarButtonsRequired (juce::DocumentWindow::allButtons, false);

        if (! window->isUsingNativeTitleBar())
            window->setUsingNativeTitleBar (true);

       #if JUCE_MAC
        auto applyCocoaStyling = [](juce::DocumentWindow* win)
        {
            if (win == nullptr)
                return;

            if (auto* peer = win->getPeer())
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
                        [nsWindow display];
                    }
                }
            }
        };

        applyCocoaStyling (window);

        // StandaloneFilterWindow calls setVisible(true) after child editor is added.
        // Queue an async call to ensure NSWindow styling is applied once the peer is fully attached.
        juce::MessageManager::callAsync ([safeDw = juce::Component::SafePointer<juce::DocumentWindow> (window), applyCocoaStyling]
        {
            if (safeDw != nullptr)
                applyCocoaStyling (safeDw.getComponent());
        });
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
