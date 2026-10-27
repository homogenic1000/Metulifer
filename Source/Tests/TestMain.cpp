#include <JuceHeader.h>

static void runTests()
{
    juce::UnitTestRunner runner;
    runner.runAllTests (true);
    juce::JUCEApplication::quit();
}

class TestApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "t"; }
    const juce::String getApplicationVersion() override { return "1"; }
    bool moreThanOneInstanceAllowed() override { return true; }
    void initialise (const juce::String&) override { runTests(); }
    void shutdown() override {}
    void systemRequestedQuit() override { quit(); }
};

START_JUCE_APPLICATION (TestApp)
