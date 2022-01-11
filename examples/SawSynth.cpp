// Example synth plugin: polyphonic saw / square wave oscillator with
// parameter for saw / square wave mixing. Has 2 parameters: mix and gain.
// Can be compiled with or without GUI support. GUI support has been developed
// only for Windows.
// For compiling see make-synth.bat
// For compiling with UI see make-synth-gui.bat
// This file is licenced with GPL-3
#include "../src/FstAudioEffect.h"
#include <cmath>
#include <mutex>
#include <vector>
#ifdef WINDOWS_GUI
#include "SawSynthGui.h"
#endif

struct Voice
{
    float increment;
    unsigned char key;
    float phase;
};

class SawSynth : public FstAudioEffect
{
    float mix = 0;
    float gain = 0.5;
    std::mutex voiceLock;
    std::vector<Voice> voices;

    float noteToIncrement(int note)
    {
        auto freq = pow(2, note / 12.0) * 16.352;
        return freq / sampleRate;
    }

public:
    SawSynth()
    {
        numParams = 2;
        numPrograms = 0;
        uniqueID = 12345;
        flags |= effFlagsIsSynth | effFlagsProgramChunks;
#ifdef WINDOWS_GUI
        flags |= effFlagsHasEditor;
#endif
    }

    void open()
    {
#ifdef WINDOWS_GUI
        setEditor(new SawSynthGui(this));
#endif
    }

    void setParameter(int index, float value)
    {
        if (index == 0)
            mix = value;
        if (index == 1)
            gain = value;
#ifdef WINDOWS_GUI
        if (editor)
        {
            ((SawSynthGui *)editor)->setParameter(index, value);
        }
#endif
    }

    float getParameter(int index)
    {
        if (index == 0)
            return mix;
        if (index == 1)
            return gain;
        return 0;
    }

    std::string getParamName(int index)
    {
        if (index == 0)
            return "Sqr/Saw Mix";
        if (index == 1)
            return "Gain";
        return "";
    }

    void processReplacing(float **indata, float **outdata, int sampleframes)
    {
        voiceLock.lock();
        for (int i = 0; i < sampleframes; i++)
        {
            float value = 0;
            for (auto &voice : voices)
            {
                auto sawValue = (voice.phase - 0.5) * 2;
                auto sqrValue = voice.phase > 0.5 ? 1 : -1;
                value += mix * sqrValue + (1 - mix) * sawValue;
                voice.phase += voice.increment;
                if (voice.phase >= 1)
                    voice.phase -= 1;
            }
            for (int c = 0; c < numOutputs; c++)
                outdata[c][i] = value * gain;
        }
        voiceLock.unlock();
    }

    int processEvents(VstEvents *events)
    {
        bool handled = false;
        for (int i = 0; i < events->numEvents; i++)
        {
            if (events->events[i]->type == kVstMidiType)
            {
                auto midiMessage = ((VstMidiEvent *)events->events[i])->midiData;
                if ((midiMessage[0] & 0xF0) == 0b10000000) // Note off
                {
                    char offkey = midiMessage[1];
                    voiceLock.lock();
                    auto it = voices.begin();
                    while (it != voices.end())
                    {
                        if (offkey == it->key)
                            it = voices.erase(it);
                        else
                            it++;
                    }
                    voiceLock.unlock();
                }
                else if ((midiMessage[0] & 0xF0) == 0b10010000) // Note on
                {
                    // Note on, key/velocity=midievent->midiData[1]/midievent->midiData[2]
                    voiceLock.lock();
                    voices.push_back({noteToIncrement(midiMessage[1]),  midiMessage[1], 0});
                    voiceLock.unlock();
                }
                handled = true;
            }
        }
        return handled ? 1 : 0;
    }
};

FstAudioEffect *createFstInstance()
{
    return new SawSynth();
}