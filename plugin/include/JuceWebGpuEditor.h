#pragma once

#include "JuceWebGpuProcessor.h"

//==============================================================================
class JuceWebGpuProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit JuceWebGpuProcessorEditor (JuceWebGpuProcessor&);
    ~JuceWebGpuProcessorEditor() override;

    //==============================================================================
    void resized() override;

private:

    JuceWebGpuProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceWebGpuProcessorEditor)
};
