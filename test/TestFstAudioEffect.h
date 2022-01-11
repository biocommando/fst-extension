#pragma once
#include "UnitTesting.h"
#include "TestingUtils.h"
#include "../src/FstAudioEffect.h"
#include <map>
#include <vector>
#include <sstream>

// For the compilation to pass
FstAudioEffect *createFstInstance()
{
    return nullptr;
}

class MyEff1 : public FstAudioEffect
{
public:
    std::map<int, float> params;
    MyEff1(int numParams)
    {
        this->numParams = numParams;
    }
    float getSampleRate() { return sampleRate; }

    void setParameter(int idx, float val)
    {
        params[idx] = val;
    }
};

TEST(FstAudioEffect_publicApi_defaultImplementations)
{
    MyEff1 fx(2);
    EXPECT(fx.getEffectName() == "FST Effect");
    EXPECT(fx.getVendorName() == "FST Effect Vendor");
    EXPECT(fx.getProgramName() == "FST Program");
    EXPECT(fx.getParamLabel(123) == "");
    EXPECT(fx.getParamName(123) == "Param123");
    EXPECT(fx.getParamDisplay(123) == "0.00000");
    fx.setSampleRate(123.321);
    EXPECT(NEAR(fx.getSampleRate(), 123.321, 0.001));
    void *data = nullptr;
    fx.getChunk(&data);
    EXPECT(data != nullptr);
    EXPECT(std::string(reinterpret_cast<char *>(data)) == "Param0\1"
                                                          "0.000000\2"
                                                          "Param1\1"
                                                          "0.000000\2");
    std::string chunk = "Param0\1"
                        "0.1\2"
                        "Param1\1"
                        "0.5\2";
    data = const_cast<void *>(reinterpret_cast<const void *>(chunk.data()));
    fx.setChunk(data, chunk.size());
    EXPECT(NEAR(fx.params[0], 0.1, 0.001));
    EXPECT(NEAR(fx.params[1], 0.5, 0.001));
}

class MyEff2 : public MyEff1
{
public:
    std::string lastCalled;

    MyEff2(int numParams) : MyEff1(numParams)
    {
    }

    void open()
    {
        lastCalled = "open";
    }

    void close()
    {
        lastCalled = "close";
    }

    std::string getParamLabel(int idx)
    {
        return "lbl" + std::to_string(idx);
    }

    std::string getParamDisplay(int idx)
    {
        return "display" + std::to_string(idx);
    }

    int processEvents(VstEvents *ptr)
    {
        std::stringstream ss;
        auto evt = (VstMidiEvent *)(ptr->events[0]);
        ss << "effProcessEvents, count=" << ptr->numEvents;
        for (int i = 0; i < 4; i++)
        {
            ss << ';' << static_cast<int>(evt->midiData[i]);
        }
        ss << ";type=" << evt->type;
        lastCalled = ss.str();
        return true;
    }
};

// Copied from fst_utils.h
static VstEvents *create_vstevents(const unsigned char midi[4])
{
    VstEvents *ves = (VstEvents *)calloc(1, sizeof(VstEvents) + sizeof(VstEvent *));
    VstMidiEvent *ve = (VstMidiEvent *)calloc(1, sizeof(VstMidiEvent));
    ves->numEvents = 1;
    ves->events[0] = (VstEvent *)ve;
    ve->type = kVstMidiType;
    ve->byteSize = sizeof(VstMidiEvent);
    for (size_t i = 0; i < 4; i++)
        ve->midiData[i] = midi[i];

    return ves;
}

static std::string ERectToStr(const ERect *er)
{
    std::stringstream ss;
    ss << 'L' << er->left << 'R' << er->right << 'T' << er->top << 'B' << er->bottom;
    return ss.str();
}

TEST(FstAudioEffect_dispatcherApi)
{
    MyEff2 fx(1);
    auto aeff = fx.getAEffect();
    aeff->dispatcher(aeff, effOpen, 0, 0, nullptr, 0);
    EXPECT(fx.lastCalled == "open");

    aeff->dispatcher(aeff, effClose, 0, 0, nullptr, 0);
    EXPECT(fx.lastCalled == "close");

    char buf[256];
    aeff->dispatcher(aeff, effGetVendorString, 0, 0, reinterpret_cast<void *>(buf), 0);
    EXPECT(std::string(buf) == "FST Effect Vendor");

    aeff->dispatcher(aeff, effGetEffectName, 0, 0, reinterpret_cast<void *>(buf), 0);
    EXPECT(std::string(buf) == "FST Effect");

    unsigned char midi[] = {1, 2, 3, 4};
    auto evts = create_vstevents(midi);
    auto res = aeff->dispatcher(aeff, effProcessEvents, 0, 0, evts, 0);
    EXPECT(res == 1);
    EXPECT(fx.lastCalled == "effProcessEvents, count=1;1;2;3;4;type=1");
    free(evts);

    void *chunk = nullptr;
    res = aeff->dispatcher(aeff, effGetChunk, 0, 0, &chunk, 0);
    const auto expectedChunk = "Param0\1"
                               "0.000000\2";
    EXPECT(sizeof(expectedChunk) - 1);
    EXPECT(chunk != nullptr);
    EXPECT(std::string(reinterpret_cast<char *>(chunk)) == expectedChunk);

    char setChunk[] = "Param0\1"
                      "100\2";
    aeff->dispatcher(aeff, effSetChunk, 0, sizeof(setChunk) - 1, &setChunk, 0);
    EXPECT(NEAR(fx.params[0], 100, 0.001));

    // GUI stuff tested separately

    aeff->dispatcher(aeff, effGetParamLabel, 123, 0, reinterpret_cast<void *>(buf), 0);
    EXPECT(std::string(buf) == "lbl123");

    aeff->dispatcher(aeff, effGetParamName, 123, 0, reinterpret_cast<void *>(buf), 0);
    EXPECT(std::string(buf) == "Param123");

    aeff->dispatcher(aeff, effGetParamDisplay, 123, 0, reinterpret_cast<void *>(buf), 0);
    EXPECT(std::string(buf) == "display123");

    aeff->dispatcher(aeff, effSetSampleRate, 0, 0, nullptr, 96000);
    EXPECT(NEAR(fx.getSampleRate(), 96000, 0.1));

    // CanDo
    res = aeff->dispatcher(aeff, effCanDo, 0, 0, VOID_CAST("receiveVstEvents"), 0);
    EXPECT(res == 1);

    res = aeff->dispatcher(aeff, effCanDo, 0, 0, VOID_CAST("receiveVstMidiEvents"), 0);
    EXPECT(res == 1);

    res = aeff->dispatcher(aeff, effCanDo, 0, 0, VOID_CAST("sendVstEvents"), 0);
    EXPECT(res == 1);

    res = aeff->dispatcher(aeff, effCanDo, 0, 0, VOID_CAST("sendVstMidiEvents"), 0);
    EXPECT(res == 1);

    res = aeff->dispatcher(aeff, effCanDo, 0, 0, VOID_CAST("wantsChannelCountNotifications"), 0);
    EXPECT(res == 1);

    res = aeff->dispatcher(aeff, effCanDo, 0, 0, VOID_CAST("other stuff"), 0);
    EXPECT(res == 0);

    masterDispatcherCapture.clear();
    fx.setAudioMasterDispatcher(masterDispatcher);
    aeff->dispatcher(aeff, effMainsChanged, 0, 0, nullptr, 0);
    EXPECT(masterDispatcherCapture.size() == 2);
    EXPECT(masterDispatcherCapture[0] == "23,0,0,0,0");
    EXPECT(masterDispatcherCapture[1] == "6,0,1,0,0");

    // Return 0 for unknown
    res = aeff->dispatcher(aeff, fst_effLast, 0, 0, nullptr, 0);
    EXPECT(res == 0);
}

class MyEff3 : public MyEff1
{
public:
    MyEff3() : MyEff1(1)
    {
        numPrograms = 1;
        numParams = 2;
        numInputs = 3;
        numOutputs = 4;
        uniqueID = 5;
        version = 6;
        flags = 7;
    }

    void processReplacing(float **indata, float **outdata, int sampleframes)
    {
        for (int i = 0; i < sampleframes; i++)
        {
            outdata[0][i] = indata[0][i];
        }
    }

    void process(float **indata, float **outdata, int sampleframes)
    {
        for (int i = 0; i < sampleframes; i++)
        {
            outdata[0][i] = indata[0][i] + 1;
        }
    }

    void processDoubleReplacing(double **indata, double **outdata, int sampleframes)
    {
        for (int i = 0; i < sampleframes; i++)
        {
            outdata[0][i] = -indata[0][i];
        }
    }
};

TEST(FstAudioEffect_aeffectImplementation)
{
    MyEff3 fx;
    auto aeff = fx.getAEffect();

    EXPECT(aeff->magic == 0x56737450);
    EXPECT(aeff->numPrograms == 1);
    EXPECT(aeff->numParams == 2);
    EXPECT(aeff->numInputs == 3);
    EXPECT(aeff->numOutputs == 4);
    EXPECT(aeff->uniqueID == 5);
    EXPECT(aeff->version == 6);
    EXPECT(aeff->flags == 7);
    EXPECT(aeff->float1 == 1);
    EXPECT(aeff->object == &fx);

    float findata = 1;
    float *findatap = &findata;
    float foutdata = 0;
    float *foutdatap = &foutdata;

    aeff->process(aeff, &findatap, &foutdatap, 1);
    EXPECT(NEAR(2, foutdata, 0.0001));
    
    aeff->processReplacing(aeff, &findatap, &foutdatap, 1);
    EXPECT(NEAR(1, foutdata, 0.0001));
    
    double dindata = 1;
    double *dindatap = &dindata;
    double doutdata = 0;
    double *doutdatap = &doutdata;
    aeff->processDoubleReplacing(aeff, &dindatap, &doutdatap, 1);
    EXPECT(NEAR(-1, doutdata, 0.0001));
}

class EffGui : public GuiBase
{
    ERect myRect{1, 2, 3, 4};

public:
    std::string lastCalled;

    bool open(void *ptr)
    {
        lastCalled = "open;" + std::to_string(reinterpret_cast<t_fstPtrInt>(ptr));
    }

    void close()
    {
        lastCalled = "close";
    }

    bool getRect(ERect **ppRect)
    {
        lastCalled = "getRect";
        *ppRect = &myRect;
        return true;
    }

    void draw(ERect *pRect)
    {
        lastCalled = "draw;" + std::string(pRect == nullptr ? "NULL" : "NOT NULL");
    }
};

TEST(FstAudioEffect_gui)
{
    MyEff1 fx(1);
    EffGui gui;
    fx.setEditor(&gui);
    auto aeff = fx.getAEffect();

    void *erect = nullptr;
    auto res = aeff->dispatcher(aeff, effEditGetRect, 0, 0, &erect, 0);
    EXPECT(erect != nullptr);
    EXPECT(res == reinterpret_cast<t_fstPtrInt>(erect));
    EXPECT(ERectToStr(reinterpret_cast<ERect *>(erect)) == "L2R4T1B3");

    aeff->dispatcher(aeff, effEditOpen, 0, 0, erect, 0);
    EXPECT(gui.lastCalled == "open;" + std::to_string(reinterpret_cast<t_fstPtrInt>(erect)));

    aeff->dispatcher(aeff, effEditClose, 0, 0, erect, 0);
    EXPECT(gui.lastCalled == "close");

    aeff->dispatcher(aeff, effEditDraw, 0, 0, nullptr, 0);
    EXPECT(gui.lastCalled == "draw;NULL");
}

TEST_SUITE(TestFstAudioEffect)
{
    RUN_TEST(FstAudioEffect_publicApi_defaultImplementations);
    RUN_TEST(FstAudioEffect_dispatcherApi);
    RUN_TEST(FstAudioEffect_aeffectImplementation);
    RUN_TEST(FstAudioEffect_gui);
}