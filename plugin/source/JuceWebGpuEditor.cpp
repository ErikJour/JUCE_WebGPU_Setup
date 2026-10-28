#include "../include/JuceWebGpuProcessor.h"
#include "../include/JuceWebGpuEditor.h"

//==============================================================================
JuceWebGpuProcessorEditor::JuceWebGpuProcessorEditor (JuceWebGpuProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);
    setSize (400, 300);
}

JuceWebGpuProcessorEditor::~JuceWebGpuProcessorEditor()
{
}


void JuceWebGpuProcessorEditor::resized()
{

}
