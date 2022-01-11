// Example effect plugin: a stereo-gain that multiplies the gain
// with the value of the single gain parameter.
// For compiling see make-effect.bat
// This file is licenced with GPL-3
#include "../src/FstAudioEffect.h"

class GainPlugin : public FstAudioEffect
{
    float gain = 0;

public:
    GainPlugin()
    {
        numParams = 1;
        numPrograms = 0;
        uniqueID = 12345;
    }

    void setParameter(int index, float value)
    {
        gain = value;
    }

    float getParameter(int index)
    {
        if (index != 0)
            return 0;
        return gain;
    }

    std::string getParamName(int index)
    {
        if (index == 0)
            return "Gain";
        return "";
    }

    void processReplacing(float **indata, float **outdata, int sampleframes)
    {
        for (int c = 0; c < numOutputs; c++)
            for (int i = 0; i < sampleframes; i++)
                outdata[c][i] = indata[c][i] * gain;
    }
};

FstAudioEffect *createFstInstance()
{
    return new GainPlugin();
}