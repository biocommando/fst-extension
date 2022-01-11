#pragma once
#include "UnitTesting.h"
#include "TestingUtils.h"
#include "../src/audioeffectx.h"
#include <cstring>
#include <map>
#include <vector>
#include <sstream>

class MyAudioeffectx : public AudioEffectX
{
public:
    MyAudioeffectx() : AudioEffectX(masterDispatcher, 0, 1)
    {}

    int getFlags() { return flags; }
    int getNumInputs() { return numInputs; }
    int getNumOutputs() { return numOutputs; }
    int getUniqueID() { return uniqueID; }

    bool getEffectName(char *buf)
    {
        strcpy(buf, "effect name");
        return true;
    }

    bool getVendorString(char *buf)
    {
        strcpy(buf, "vendor string");
        return true;
    }

    std::string chunk;

    void getParameterName(int index, char *label)
    {
        sprintf(label, "parameter name %d", index);
    }

    void getParameterLabel(int index, char *label)
    {
        sprintf(label, "parameter label %d", index);
    }

    void getParameterDisplay(int index, char *label)
    {
        sprintf(label, "parameter display %d", index);
    }

    
    int setChunk(void *data, int sz, bool xx)
    {
        chunk = std::string(reinterpret_cast<char*>(data), sz);
        return 0;
    }

    int getChunk(void **data, bool xx)
    {
        *data = VOID_CAST(chunk.data());
        return chunk.size();
    }
};

TEST(TestAudioeffectx_AudioEffectX_compatibilityExtensionsHostSide)
{
    MyAudioeffectx aefx;

    auto fx = static_cast<FstAudioEffect*>(&aefx);

    EXPECT(fx->getEffectName() == "effect name");
    EXPECT(fx->getVendorName() == "vendor string");

    EXPECT(fx->getParamName(123) == "parameter name 123");
    EXPECT(fx->getParamLabel(123) == "parameter label 123");
    EXPECT(fx->getParamDisplay(123) == "parameter display 123");

    std::string chunk = "Hello world!";
    fx->setChunk(VOID_CAST(chunk.data()), chunk.size());
    EXPECT(chunk == aefx.chunk);
    void *chunkPtr = nullptr;
    fx->getChunk(&chunkPtr);
    EXPECT(chunkPtr == aefx.chunk.data());

    fx->setSampleRate(1234);
    EXPECT(NEAR(aefx.getSampleRate(), 1234, 0.001));
}

TEST(TestAudioeffectx_AudioEffectX_compatibilityExtensionsPluginSide)
{
    MyAudioeffectx aefx;
    aefx.setNumInputs(10);
    EXPECT(aefx.getNumInputs() == 10);
    aefx.setNumOutputs(20);
    EXPECT(aefx.getNumOutputs() == 20);
    aefx.setUniqueID(30);
    EXPECT(aefx.getUniqueID() == 30);

    aefx.isSynth(false);
    EXPECT(aefx.getFlags() == effFlagsCanReplacing);
    aefx.isSynth(true);
    EXPECT(aefx.getFlags() == effFlagsIsSynth | effFlagsCanReplacing);
    aefx.isSynth(false);
    EXPECT(aefx.getFlags() == effFlagsCanReplacing);
    aefx.programsAreChunks();
    EXPECT(aefx.getFlags() == effFlagsProgramChunks | effFlagsCanReplacing);
    aefx.hasEditor();
    EXPECT(aefx.getFlags() == effFlagsHasEditor | effFlagsProgramChunks | effFlagsCanReplacing);

    masterDispatcherCapture.clear();
    VstTimeInfo info;
    info.tempo = 120.005;
    masterDispatcherReturnValue = reinterpret_cast<void*>(&info);
    auto retInfo = aefx.getTimeInfo(123);
    EXPECT(NEAR(retInfo->tempo, 120.005, 1e-6));
    EXPECT(masterDispatcherCapture[0] == "7,0,123,0,0");
}

TEST(TestAudioeffectx_float2string)
{
    char buf[100] = "";
    float2string(5.6777, buf, 100);
    EXPECT(std::string(buf) == "5.678");
}

TEST_SUITE(TestAudioeffectx)
{
    RUN_TEST(TestAudioeffectx_AudioEffectX_compatibilityExtensionsHostSide);
    RUN_TEST(TestAudioeffectx_AudioEffectX_compatibilityExtensionsPluginSide);
    RUN_TEST(TestAudioeffectx_float2string);
}


