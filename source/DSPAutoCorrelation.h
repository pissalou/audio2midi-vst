#pragma once

#include <juce_dsp/juce_dsp.h>

class AutoCorrelation
{
public:

    int LNL; //least note length
    int function = 0;   // function to find note [Origin, SIMD, FFT]
    int windowSizePower2 = 11;
    int hoppingSize = 1024;
    float correlationThres;
    float noiseThres;

    AutoCorrelation();
    void prepare(double SampleRate, int SampleSize);
    void process(const juce::dsp::ProcessContextReplacing<float> &context ,juce::MidiBuffer& midiMessages);

    // return note
    int findNote();
    int SIMDfindNote();
    int FFTfindNote();  //this FFT can process only with fixed window size (2048 here)

    // determine whether should we make noteOn message
    void addMidiMessage(int note, int notePos, juce::MidiBuffer& midiMessages);

private:
    double sampleRate;
    int sampleSize;

    int windowSize;
    float windowSamples[8192] = {0};
    int windowNextFill;
    int curSample;

    // for SIMD
    float sums[8192] = { 0 };

    // for FFT
    juce::dsp::FFT forwardFFT;
    std::array<float, 4096> FFTdata;

    // for building midi message
    int lastNote;   //lastNote == -1 means there's no note sustaining
    int lastNotePos;
};
