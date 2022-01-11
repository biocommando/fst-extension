// Compatibility wrapper for VST plugins extending AudioEffectX.
// Code derived by black-box reverse-engineering a plugin that's stripped
// from VST SDK includes.
// This file is licenced with GPL-3
#pragma once
#include "FstAudioEffect.h"
#include "guibase.h"

#define FST_WRAP_CSTR_FN(bufferName, fn) \
    char bufferName[256];                \
    fn;                                  \
    return bufferName;

inline static void float2string(float f, char *text, int len)
{
    snprintf(text, len, "%.3f", f);
}

class AudioEffectX : public FstAudioEffect
{
public:
    AudioEffectX(AEffectDispatcherProc d, int unknown, int numParams)
    {
        this->audioMasterDispatcher = d;
        this->numParams = numParams;
    }

    void setNumInputs(int i)
    {
        numInputs = i;
    }

    void setNumOutputs(int i)
    {
        numOutputs = i;
    }

    void setUniqueID(int i)
    {
        uniqueID = i;
    }

    void programsAreChunks()
    {
        FST_DEBUG_LOG("AudioEffectX", "programsAreChunks");
        flags |= effFlagsProgramChunks;
        FST_DEBUG_LOG("AudioEffectX", "flags changed " << flags);
    }

    void isSynth(bool b)
    {
        FST_DEBUG_LOG("AudioEffectX", "isSynth = " << b);
        if (b)
            flags |= effFlagsIsSynth;
        else
            flags &= ~effFlagsIsSynth;
        FST_DEBUG_LOG("AudioEffectX", "flags changed " << flags);
    }

    void hasEditor()
    {
        FST_DEBUG_LOG("AudioEffectX", "hasEditor");
        flags |= effFlagsHasEditor;
        FST_DEBUG_LOG("AudioEffectX", "flags changed " << flags);
    }

    void setParameterAutomated(int idx, float value)
    {
        setParameter(idx, value);
    }

    std::string getEffectName()
    {
        FST_WRAP_CSTR_FN(buf, getEffectName(buf));
    }

    std::string getVendorName()
    {
        FST_WRAP_CSTR_FN(buf, getVendorString(buf));
    }

    std::string getProgramName()
    {
        return "";
    }

    std::string getParamLabel(int index)
    {
        FST_WRAP_CSTR_FN(buf, getParameterLabel(index, buf));
    }

    std::string getParamName(int index)
    {
        FST_WRAP_CSTR_FN(buf, getParameterName(index, buf));
    }

    std::string getParamDisplay(int index)
    {
        FST_WRAP_CSTR_FN(buf, getParameterDisplay(index, buf));
    }

    virtual void getParameterName(int index, char *label)
    {
        *label = 0;
    }
    virtual void getParameterLabel(int index, char *label)
    {
        *label = 0;
    }
    virtual void getParameterDisplay(int index, char *label)
    {
        *label = 0;
    }

    virtual bool getEffectName(char *name)
    {
        *name = 0;
        return true;
    }

    virtual bool getProductString(char *text)
    {
        *text = 0;
        return true;
    }

    virtual bool getVendorString(char *text)
    {
        *text = 0;
        return true;
    }

    virtual int setChunk(void *data, int sz, bool xx)
    {
        return 0;
    }

    virtual int getChunk(void **data, bool xx)
    {
        return 0;
    }

    int getChunk(void **data)
    {
        return getChunk(data, false);
    }

    void setChunk(void *data, int sz)
    {
        setChunk(data, sz, false);
    }

    float getSampleRate() const
    {
        return sampleRate;
    }

    VstTimeInfo *getTimeInfo(int flags)
    {
        return (VstTimeInfo *)audioMasterDispatch(audioMasterGetTime, 0, flags, nullptr, 0);
    }
};

#define createEffectInstance createFstInstance

typedef AudioEffectX AudioEffect;

typedef short VstInt16;
