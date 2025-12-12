#pragma once

#include "AudioPlayer.h"
#include "PluginProcessor.h"
#include <melatonin_inspector/melatonin_inspector.h>

struct IIRFilterDemoDSP
{
    void prepare (const dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;

        iir.state = dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, 440.0);
        iir.prepare (spec);
    }

    void process (const dsp::ProcessContextReplacing<float>& context)
    {
        iir.process (context);
    }

    void reset()
    {
        iir.reset();
    }

    void updateParameters() const
    {
        if (! approximatelyEqual (sampleRate, 0.0))
        {
            auto cutoff = static_cast<float> (cutoffParam.getCurrentValue());
            auto qVal   = static_cast<float> (qParam.getCurrentValue());

            switch (typeParam.getCurrentSelectedID())
            {
                case 1:     *iir.state = dsp::IIR::ArrayCoefficients<float>::makeLowPass  (sampleRate, cutoff, qVal); break;
                case 2:     *iir.state = dsp::IIR::ArrayCoefficients<float>::makeHighPass (sampleRate, cutoff, qVal); break;
                case 3:     *iir.state = dsp::IIR::ArrayCoefficients<float>::makeBandPass (sampleRate, cutoff, qVal); break;
                default:    break;
            }
        }
    }

    //==============================================================================
    dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>> iir;

    ChoiceParameter typeParam { { "Low-pass", "High-pass", "Band-pass" }, 1, "Type" };
    SliderParameter cutoffParam { { 20.0, 20000.0 }, 0.5, 440.0f, "Cutoff", "Hz" };
    SliderParameter qParam { { 0.3, 20.0 }, 0.5, 1.0 / std::sqrt (2.0), "Q" };

    std::vector<DSPDemoParameterBase*> parameters { &typeParam, &cutoffParam, &qParam };
    double sampleRate = 0.0;
};

//==============================================================================
class PluginEditor final : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginProcessor& processorRef;
    AudioFileReaderComponent<IIRFilterDemoDSP> fileReaderComponent;
    std::unique_ptr<melatonin::Inspector> inspector;
    juce::TextButton inspectButton { "Inspect the UI" };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
