// This file is licenced with GPL-3
#include "FstAudioEffect.h"
#include <cstdio>
#include <cstring>
#include <map>

static void strToCstr(void *dst, int sz, const std::string &src)
{
    strncpy((char *)dst, src.c_str(), sz);
}

#define FST_EFF ((FstAudioEffect *)eff->object)

static void fst_processReplacing(AEffect *eff, float **indata, float **outdata, int sampleframes)
{
    FST_DEBUG_LOG("process", "fst_processReplacing, frames " << sampleframes);
    FST_EFF->processReplacing(indata, outdata, sampleframes);
}
static void fst_process(AEffect *eff, float **indata, float **outdata, int sampleframes)
{
    FST_DEBUG_LOG("process", "fst_process, frames " << sampleframes);
    FST_EFF->process(indata, outdata, sampleframes);
}
static void fst_processDoubleReplacing(AEffect *eff, double **indata, double **outdata, int sampleframes)
{
    FST_DEBUG_LOG("process", "fst_processDoubleReplacing, frames " << sampleframes);
    FST_EFF->processDoubleReplacing(indata, outdata, sampleframes);
}

static void fst_setParameter(AEffect *eff, int index, float value)
{
    FST_DEBUG_LOG("params", "fst_setParameter: " << index << '=' << value);
    FST_EFF->setParameter(index, value);
}

static float fst_getParameter(AEffect *eff, int index)
{
    FST_DEBUG_LOG("params", "fst_getParameter: " << index);
    return FST_EFF->getParameter(index);
}

static inline t_fstPtrInt fst_dispatcher(AEffect *eff, t_fstInt32 opcode, int index, t_fstPtrInt ivalue, void *const ptr, float fvalue)
{
    if (opcode == 53)
        return 0;
    FST_DEBUG_LOG("dispatch" + std::to_string(opcode), "fst_dispatcher: op=" << opcode << ", idx=" << index << ", ival=" << ivalue << ", ptr set?=" << (ptr != nullptr) << ", fval=" << fvalue);
    auto aeff = FST_EFF;
    switch (opcode)
    {
    default:
        break;
    case effOpen:
        aeff->open();
        return 0;

    case effClose:
        aeff->close();
        return 0;

    case effGetVendorString:
        strToCstr(ptr, 64, aeff->getVendorName());
        return 1;
    case effGetEffectName:
        strToCstr(ptr, 64, aeff->getEffectName());
        return 1;
    case effProcessEvents:
        return aeff->processEvents((VstEvents *)ptr) ? 1 : 0;
    case effGetChunk:
        return aeff->getChunk((void **)ptr);
    case effSetChunk:
        aeff->setChunk((void *)ptr, ivalue);
        return 0;
    case effEditGetRect:
        *((ERect **)ptr) = aeff->getGuiBounds();
        return (t_fstPtrInt)aeff->getGuiBounds();
    case effEditOpen:
        aeff->setGuiWindow((void *)ptr);
        return 1;
    case effEditClose:
        aeff->setGuiWindow(nullptr);
        return 0;
    case effEditDraw:
        aeff->notifyDraw();
        return 0;
    /*case effGetChunk:
    {
        char **strptr = (char **)ptr;
        *strptr = chunk;
    }
        // printf("getChunk: %d bytes @ %p\n", sizeof(chunk), chunk);
        return sizeof(chunk);*/
    /*case effSetProgram:
        // printf("setting program to %d\n", ivalue);
        curProgram = ivalue;
        return 1;
    case effGetProgramName:
        snprintf((char *)ptr, 32, "FstProgram%d", curProgram);
        // printf("JMZ:setting program-name to %s\n", (char*)ptr);
        return 1;*/
    case effGetParamLabel:
        strToCstr(ptr, 32, aeff->getParamLabel(index));
        return 0;
    case effGetParamName:
        strToCstr(ptr, 32, aeff->getParamName(index));
        return 0;
    case effSetSampleRate:
        aeff->setSampleRate(fvalue);
        return 0;
    case effGetParamDisplay:
        strToCstr(ptr, 32, aeff->getParamDisplay(index));
        return 0;
    case effCanDo:
        do
        {
            //printf("canDo '%s'?\n", (char *)ptr);
            if (!strcmp((char *)ptr, "receiveVstEvents"))
                return 1;
            if (!strcmp((char *)ptr, "receiveVstMidiEvents"))
                return 1;
            if (!strcmp((char *)ptr, "sendVstEvents"))
                return 1;
            if (!strcmp((char *)ptr, "sendVstMidiEvents"))
                return 1;
            if (!strcmp((char *)ptr, "wantsChannelCountNotifications"))
                return 1;
        } while (0);
        return 0;
    case effMainsChanged:
        aeff->audioMasterDispatch(audioMasterGetCurrentProcessLevel, 0, 0, 0, 0.);
        aeff->audioMasterDispatch(audioMasterWantMidi, 0, 1, 0, 0.);
        break;
    }
    // printf("FstClient::dispatch(%p, %d, %d, %d, %p, %f)\n", eff, opcode, index, ivalue, ptr, fvalue);
    // printf("JMZ\n");

    return 0;
}

AEffect *FstAudioEffect::getAEffect()
{
    FST_DEBUG_LOG("newfx", "getAEffect");
    if (eff)
        return eff;
    eff = new AEffect();
    // FST_EFF = this;
    memset(eff, 0, sizeof(AEffect));
    eff->magic = 0x56737450;
    eff->dispatcher = fst_dispatcher;
    eff->process = fst_process;
    eff->getParameter = fst_getParameter;
    eff->setParameter = fst_setParameter;

    eff->numPrograms = numPrograms;
    eff->numParams = numParams;
    eff->numInputs = numInputs;
    eff->numOutputs = numOutputs;
    eff->float1 = 1;
    eff->object = this;
    eff->uniqueID = uniqueID;
    eff->version = version;
    eff->flags = flags;
    eff->processReplacing = fst_processReplacing;
    eff->processDoubleReplacing = fst_processDoubleReplacing;

    /*//probably not needed
    for (size_t i = 0; i < (sizeof(canDos) / sizeof(*canDos)); i++)
    {
        char buf[512] = {0};
        char hostcode[512] = {0};
        snprintf(buf, 511, canDos[i]);
        buf[511] = 0;
        t_fstPtrInt res = dispatch(0, audioMasterCanDo, 0, 0, buf, 0);
    }*/
    return eff;
}

int FstAudioEffect::getChunk(void **ptr)
{
    FST_DEBUG_LOG("FstAudioEffect", "getChunk default implementation");
    serialized = "";
    for (int i = 0; i < numParams; i++)
    {
        serialized.append(getParamName(i) + "\1" + std::to_string(getParameter(i)) + "\2");
    }
    *ptr = (void *)serialized.data();
    FST_DEBUG_LOG("FstAudioEffect", "get chunk=" << serialized);
    return serialized.size();
}

void FstAudioEffect::setChunk(void *ptr, int size)
{
    FST_DEBUG_LOG("FstAudioEffect", "setChunk default implementation, size " << size);
    std::string saveData((char *)ptr, size);
    FST_DEBUG_LOG("FstAudioEffect", "set chunk=" << saveData);
    size_t pos = 0;
    while (pos < size)
    {
        const auto origpos = pos;
        pos = saveData.find('\2', pos);
        if (pos == std::string::npos)
            break;
        const auto pos2 = saveData.find('\1', origpos);
        if (pos2 == std::string::npos)
            break;
        const auto name = saveData.substr(origpos, pos2 - origpos);
        const auto svalue = saveData.substr(pos2 + 1, pos - pos2);
        const auto value = std::stof(svalue);
        for (int i = 0; i < numParams; i++)
        {
            if (getParamName(i) == name)
            {
                FST_DEBUG_LOG("FstAudioEffect", "set parameter " << name << '=' << value);
                setParameter(i, value);
                break;
            }
        }
        pos = pos + 1;
    }
}

extern FstAudioEffect *createFstInstance(
#ifdef VST_COMPAT
    AEffectDispatcherProc
#endif
);

extern "C" AEffect *VSTPluginMain(AEffectDispatcherProc dispatch4host)
{
    FST_DEBUG_LOG("newfx", "Call createFstInstance..."
#ifdef VST_COMPAT
                           "note: VST_COMPAT is enabled!"
#endif
    );
    auto plugin = createFstInstance(
#ifdef VST_COMPAT
        dispatch4host
#endif
    );
#ifndef VST_COMPAT
    FST_DEBUG_LOG("newfx", "Call setAudioMasterDispatcher...");
    plugin->setAudioMasterDispatcher(dispatch4host);
#endif
    FST_DEBUG_LOG("newfx", "Create AEffect...");
    return plugin->getAEffect();
}

#ifdef WINDOWS_GUI
#include <windows.h>

void *hInstance; // Required by VSTGui

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL, // handle to DLL module
    DWORD fdwReason,    // reason for calling function
    LPVOID lpReserved)  // reserved
{
    hInstance = (void *)hinstDLL;
    return TRUE;
}

#endif
