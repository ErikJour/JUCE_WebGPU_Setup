#pragma once

#include "JuceWebGpuProcessor.h"
#include "WebGpuWindow.h"
#define WIDTH 800
#define HEIGHT 450

//==============================================================================
class JuceWebGpuProcessorEditor final : public juce::AudioProcessorEditor,
                                        private juce::Timer
{
public:
    explicit JuceWebGpuProcessorEditor (JuceWebGpuProcessor&);
    ~JuceWebGpuProcessorEditor() override;

    //==============================================================================
    void parentHierarchyChanged() override;
    void resized() override;
    void timerCallback() override;


private:

    JuceWebGpuProcessor& processorRef;
    WebGpuWindow                mWebGpuWindow;
#if JUCE_MAC
    juce::NSViewComponent       mMetalView;
#endif
    double   mStartTimeMs         = 0.0;
    bool     mStartTimeSet        = false;
    uint32_t mConfiguredWidth     = 0;
    uint32_t mConfiguredHeight    = 0;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceWebGpuProcessorEditor)
};
