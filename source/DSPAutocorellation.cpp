#include "DSPAutoCorrelation.h"

int midiChannel = 0;

//FFT size = 2048 (11 power 2)
AutoCorrelation::AutoCorrelation() : forwardFFT(11)
{
    lastNote = -1;
    lastNotePos = 0;
    LNL = 3000;
}

void AutoCorrelation::prepare(double SampleRate, int SampleSize)
{
    sampleRate = SampleRate;
    sampleSize = SampleSize;
    windowNextFill = 0;
    curSample = 0;
}

void AutoCorrelation::process (const juce::dsp::ProcessContextReplacing<float>& context,
                               juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    // --- Input binding (const-correct)
    const auto& inBlock = context.getInputBlock();
    const float* src    = inBlock.getChannelPointer (0); // mono; downmix if needed
    // --- Derived window size (ensure allocation/resize elsewhere as needed)
    windowSize = 1 << windowSizePower2;
    jassert (hoppingSize > 0 && hoppingSize <= windowSize);
    // Carry block-relative time safely
    lastNotePos = std::max (0, lastNotePos - sampleSize);
    // Process samples in the current block
    while (curSample < sampleSize)
    {
        // 1) Note-off if sustained beyond LNL
        if (lastNote != -1)
        {
            const int ts = std::max (0, curSample - 1);
            if ((ts - lastNotePos) > LNL)
            {
                midiMessages.addEvent (juce::MidiMessage::allNotesOff (midiChannel), ts);
                lastNote = -1;
            }
        }
        // 2) If window is full, run note detection and emit MIDI
        if (windowNextFill >= windowSize)
        {
            int note = -1;
            switch (function)
            {
                case 0: note = findNote();     break;
                case 1: note = SIMDfindNote(); break;
                default: note = FFTfindNote(); break;
            }
            const int ts = std::max (0, curSample - 1);
            if (note != -1)
            {
                juce::Logger::writeToLog ("Found note: " + juce::String (note));
                addMidiMessage (note, ts, midiMessages);
            }
            // Slide window forward by 'hoppingSize'
            // (Consider replacing with a circular buffer to avoid O(N) copy)
            const int keep = windowSize - hoppingSize;
            for (int i = 0; i < keep; ++i)
                windowSamples[i] = windowSamples[i + hoppingSize];
            windowNextFill -= hoppingSize;
            continue; // Try to fill more without advancing curSample
        }
        // 3) Fill window from input
        windowSamples[windowNextFill++] = src[curSample++];
    }

    // Reset block-local cursor for next call
    curSample = 0;
}

int AutoCorrelation::findNote(){
    int T = 1;  //period represented in number of samples
    float ACF_PREV, ACF;    //ACF means correlation function value
    float thres;    //to determine whether we're in the second local peak region
    bool flag = false;  //flag be true while we get into second local peak region

    for (int k = 0; k < windowSize; ++k)
    {
        ACF_PREV = ACF;
        ACF = 0;

        // calculate correlation values
        for (int n = 0; n < windowSize - k; ++n)
            ACF += windowSamples[n] * windowSamples[n + k];

        //determine thres while k == 0
        if (!k) {
            thres = ACF * correlationThres;
            continue;
        }

        // already find the local peak
        if (flag && ACF <= ACF_PREV) {
            T = k - 1;
            break;
        }

        // already get into second local peak region
        if (ACF > ACF_PREV && ACF > thres) {
            flag = true;
        }
    }

    //calculating note
    double freq = sampleRate / T;
    int note = round(log(freq / 440.0) / log(2) * 12 + 69);
    if (note > 127 || note < 0 || thres <= noiseThres )
        return -1;
    return note;
}

int AutoCorrelation::SIMDfindNote(){
    //calculating correlation values and fill in sums[]
    for(int k = 0; k < windowSize; k++)
        sums[k] = 0;
    for(int k = 0; k < windowSize; k++){
        juce::FloatVectorOperationsBase<float, int>::addWithMultiply(sums, &windowSamples[k], windowSamples[k], windowSize - k);
    }

    int T = 1;  //period represented in number of samples
    float thres = correlationThres * sums[0];   //determine thres
    bool flag = false;  //flag be true while we get into second local peak region

    //find second local peak
    for (int k = 1; k < windowSize; ++k){
        // already get the peak
        if (flag && sums[k] <= sums[k-1]){
            T = k - 1;
            break;
        }

        // already get into second local peak region
        if (sums[k] > sums[k-1] && sums[k] > thres){
            flag = true;
        }
    }

    //calculating note
    double freq = sampleRate / T;
    int note = round(log(freq / 440.0) / log(2) * 12 + 69);
    if (note > 127 || note < 0 || thres <= noiseThres )
        return -1;
    return note;
}

int AutoCorrelation::FFTfindNote(){
    // copy samples from window to FFTdata
    std::fill (FFTdata.begin(), FFTdata.end(), 0);
    std::copy (&windowSamples[0], &windowSamples[2047], FFTdata.begin());

    // do FFT
    forwardFFT.performFrequencyOnlyForwardTransform(FFTdata.data());

    // calculate note
    auto k = std::distance(FFTdata.begin(), std::max_element(FFTdata.begin(), FFTdata.end()));
    double freq = (double)k / 2048 * sampleRate;
    int note = round(log(freq / 440.0) / log(2) * 12 + 69);
    if (note > 127 || note < 0 || FFTdata[k] <= noiseThres )
        return -1;
    return note;
}

void AutoCorrelation::addMidiMessage(int note, int notePos, juce::MidiBuffer& midiMessages){
    // illegal note
    if (note == -1)
        return;

    // lastNote == -1 means there's no ongoing note, we can make noteOn message now
    if (lastNote == -1){
        auto message = juce::MidiMessage::noteOn(1, note, (juce::uint8) 100);
        midiMessages.addEvent(message, notePos);
        lastNote = note;
        lastNotePos = notePos;
        return;
    }

    // we detect the same note again, so update its note position to make it sustain
    if (note == lastNote){
        lastNotePos = notePos;
        return;
    }
}