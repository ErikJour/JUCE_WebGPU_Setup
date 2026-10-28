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
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    JuceWebGpuProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceWebGpuProcessorEditor)
};
