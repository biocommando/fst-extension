// This file is licenced with GPL-3
#pragma once
#include "../FST-master/fst/fst.h"
#include "guibase.h"
#include <string>

#ifdef FST_DEBUG_OUTPUT
#include <fstream>
#ifndef FST_DEBUG_FILTER
#define FST_DEBUG_FILTER ""
#endif

#define FST_DEBUG_LOG(logger, logEvent)                                                  \
    do                                                                                   \
    {                                                                                    \
        std::string flt = FST_DEBUG_FILTER;                                              \
        bool posFlt = (flt != "" && flt[0] == '+');                                      \
        bool fltMatch = std::string(FST_DEBUG_FILTER).find(logger) != std::string::npos; \
        if ((fltMatch && !posFlt) || (!fltMatch && posFlt))                              \
            break;                                                                       \
        std::ofstream ofs;                                                               \
        ofs.open(FST_DEBUG_OUTPUT, std::ios_base::app);                                  \
        ofs << '[' << logger << "]: " << logEvent << '\n';                               \
    } while (0)

// static void FST_DEBUG_LOG_eyecatcher(const std::string &logger);
#else
#define FST_DEBUG_LOG(logger, logEvent)
#endif

class FstAudioEffect
{
protected:
    int numPrograms = 0;
    int numParams = 0;
    int numInputs = 2;
    int numOutputs = 2;
    int uniqueID = 0;
    int version = 0;
    int flags = effFlagsCanReplacing;
    float sampleRate = 0;
    AEffect *eff = nullptr;
    AEffectDispatcherProc audioMasterDispatcher;
    std::string serialized;

    // GUI stuff
    ERect guiBounds = {0, 0, 600, 600};
    void *guiWindow = nullptr;
    GuiBase *editor = nullptr;
    bool editorDirty = true;

    void openGui()
    {
        if (editor)
            editor->open(guiWindow);
    }

    void closeGui()
    {
        if (editor)
            editor->close();
    }

    void updateGuiBounds()
    {
        if (!editor)
            return;
        FST_DEBUG_LOG("AudioEffectX", "updateGuiBounds");
        ERect *ptr = &guiBounds;
        editor->getRect(&ptr);
        guiBounds = *ptr;
    }

public:
    virtual void processReplacing(float **indata, float **outdata, int sampleframes)
    {
    }
    virtual void process(float **indata, float **outdata, int sampleframes)
    {
    }
    virtual void processDoubleReplacing(double **indata, double **outdata, int sampleframes)
    {
    }

    virtual void open()
    {
    }

    virtual void close()
    {
    }

    virtual int getChunk(void **ptr);

    virtual void setChunk(void *ptr, int size);

    virtual int processEvents(VstEvents *events)
    {
        return false;
    }

    virtual void setParameter(int index, float value)
    {
    }

    virtual float getParameter(int index)
    {
        return 0;
    }

    void setAudioMasterDispatcher(AEffectDispatcherProc dispatch)
    {
        audioMasterDispatcher = dispatch;
    }

    AEffect *getAEffect();

    virtual std::string getEffectName()
    {
        return "FST Effect";
    }

    virtual std::string getVendorName()
    {
        return "FST Effect Vendor";
    }
    virtual std::string getProgramName()
    {
        return "FST Program";
    }
    virtual std::string getParamLabel(int index)
    {
        return "";
    }
    virtual std::string getParamName(int index)
    {
        return "Param" + std::to_string(index);
    }
    virtual std::string getParamDisplay(int index)
    {
        return std::to_string(getParameter(index)).substr(0, 7);
    }

    t_fstPtrInt audioMasterDispatch(t_fstInt32 opcode, int index, t_fstPtrInt ivalue, void *const ptr, float fvalue)
    {
        FST_DEBUG_LOG("FstAudioEffect", "audioMasterDispatch called op=" << opcode << ", idx=" << index << ", ival=" << ivalue << ", ptr set?" << (ptr != nullptr) << ", fval=" << fvalue);
        return audioMasterDispatcher(eff, opcode, index, ivalue, ptr, fvalue);
    }

    void setSampleRate(float sr)
    {
        FST_DEBUG_LOG("FstAudioEffect", "setSampleRate to " << sr);
        sampleRate = sr;
    }

    // GUI related

    void setGuiWindow(void *ptr)
    {
        FST_DEBUG_LOG("FstAudioEffect", "setGuiWindow to " << (ptr ? "VOID*" : "NULL"));
        guiWindow = ptr;
        if (ptr)
            openGui();
        else
            closeGui();
    }

    ERect *getGuiBounds()
    {
        updateGuiBounds();
        FST_DEBUG_LOG("FstAudioEffect",
                      "getGuiBounds: top "
                          << guiBounds.top << ", left " << guiBounds.left
                          << ", bottom " << guiBounds.bottom << ", right "
                          << guiBounds.right);
        return &guiBounds;
    }

    void setEditor(GuiBase *editor)
    {
        FST_DEBUG_LOG("FstAudioEffect", "setEditor");
        this->editor = editor;
        // hasEditor(); // It's too late to tell it here
    }

    virtual void setParameterAutomated(int index, float value)
    {
        FST_DEBUG_LOG("FstAudioEffect", "setParameterAutomated idx=" << index << ", val=" << value);
        setParameter(index, value);
        editorDirty = true;
    }

    virtual void notifyDraw()
    {
        if (editor && (editorDirty || editor->getFrameDirtyStatus()))
        {
            editor->draw(nullptr);
            editorDirty = false;
        }
    }

    virtual ~FstAudioEffect() {}
};