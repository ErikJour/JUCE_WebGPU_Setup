#include "../include/JuceWebGpuProcessor.h"
#include "../include/JuceWebGpuEditor.h"

//==============================================================================
JuceWebGpuProcessorEditor::JuceWebGpuProcessorEditor (JuceWebGpuProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);
    setSize (WIDTH, HEIGHT);
    mWebGpuWindow.initialize();
}

JuceWebGpuProcessorEditor::~JuceWebGpuProcessorEditor()
{
    stopTimer();
    mWebGpuWindow.terminate();
}

void JuceWebGpuProcessorEditor::parentHierarchyChanged()
{
    AudioProcessorEditor::parentHierarchyChanged();

    if (mWebGpuWindow.hasSurface() || getPeer() == nullptr)
        return;

    const auto* webGpuDisplay = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
    const double scale        = webGpuDisplay ? webGpuDisplay->scale : 1.0;
    const auto width          = static_cast<uint32_t>(getWidth() * scale);
    const auto height         = static_cast<uint32_t>(getHeight() * scale);

    if (!mWebGpuWindow.initSurface(scale, width, height))
        return;
#if JUCE_MAC
    mMetalView.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(&mMetalView);
    mMetalView.setView(mWebGpuWindow.getNativeView());
    mMetalView.setBounds(getLocalBounds());
#endif
    mStartTimeMs      = juce::Time::getMillisecondCounterHiRes();
    mStartTimeSet     = true;
    mConfiguredWidth  = width;
    mConfiguredHeight = height;
    startTimerHz(60);
    setResizable (false, false);
}

void JuceWebGpuProcessorEditor::timerCallback()
{
    if (mStartTimeSet) {
        mWebGpuWindow.getScene().renderFrame();
    }
}

void JuceWebGpuProcessorEditor::resized()
{

}
