// Example UI for SawSynth.cpp using VST GUI.
// For compiling see make-synth-gui.bat
// This file is licenced with GPL-3
#include "../VSTGui/aeffguieditor.h"
#include <windows.h>
#include <map>

static inline std::string getWorkDir()
{
    // work out the resource directory
    // first we get the DLL path from windows API
    extern void *hInstance;
    wchar_t workDirWc[1024];
    GetModuleFileName((HMODULE)hInstance, workDirWc, 1024);
    char workDirC[1024];
    wcstombs(workDirC, workDirWc, 1024);

    std::string d = workDirC;

    // let's get rid of the DLL file name
    auto posBslash = d.find_last_of('\\');
    if (posBslash != std::string::npos)
    {
        d = d.substr(0, posBslash);
    }
    return d;
}

static inline CBitmap *loadBitmap(const std::string &relativePath)
{
    auto workDir = getWorkDir();
    std::string s = workDir + "\\" + relativePath;
    std::wstring ws(s.size(), L'#');
    mbstowcs(&ws[0], s.c_str(), s.size());
    auto bmp = Gdiplus::Bitmap::FromFile(ws.c_str(), false);
    auto cbmp = new CBitmap(bmp);
    delete bmp;
    return cbmp;
}

#define ADD_LABEL(rect, text)                                  \
    do                                                         \
    {                                                          \
        lbl = new CTextLabel(rect, std::string(text).c_str()); \
        lbl->setBackColor(cBg);                                \
        xframe->addView(lbl);                                  \
    } while (0)

class SawSynthGui : public AEffGUIEditorFst, public CControlListener
{
    std::map<long, CKnob *> knobs;
    std::map<long, CTextLabel *> knobLabels;

public:
    SawSynthGui(void *effectInstance) : AEffGUIEditorFst(effectInstance)
    {
        FST_DEBUG_LOG("SawSynthGui", "create GUI");
    }

    bool open(void *ptr)
    {
        FST_DEBUG_LOG("SawSynthGui", "open GUI");
        CRect frameSize(0, 0, 210, 200);
        CColor cBg = {127, 127, 127, 255}, cFg = kWhiteCColor;
        ERect *wSize;
        getRect(&wSize);

        wSize->top = wSize->left = 0;
        wSize->bottom = (short)frameSize.bottom;
        wSize->right = (short)frameSize.right;

        auto xframe = new CFrame(frameSize, ptr, this);
        xframe->setBackgroundColor(cBg);
        // 40 * 40 image
        CBitmap *knobBg = loadBitmap("knob-fst-SawSynthGui.bmp");

        CTextLabel *lbl;
        ADD_LABEL(CRect(10, 10, 200, 30), "FST with VSTGUI Synth Example");

        for (int paramIdx = 0; paramIdx <= 1; paramIdx++)
        {
            auto xOffset = paramIdx * 85 + 10;
            auto yOffset = 40;
            ADD_LABEL(CRect(xOffset, yOffset, xOffset + 80, yOffset + 20), fstEffect->getParamName(paramIdx));

            CRect knobRect(xOffset + 20, yOffset + 20, xOffset + 60, yOffset + 60);
            auto knob = new CKnob(knobRect, this, paramIdx, knobBg, nullptr);
            knob->setValue(fstEffect->getParameter(paramIdx));
            knobs[paramIdx] = knob;
            xframe->addView(knob);

            ADD_LABEL(CRect(xOffset, yOffset + 60, xOffset + 80, yOffset + 80), fstEffect->getParamDisplay(paramIdx));
            knobLabels[paramIdx] = lbl;
        }
        knobBg->forget();
        frame = xframe;
        return true;
    }

    void close()
    {
        auto xframe = frame;
        frame = nullptr;
        delete xframe;
    }

    void valueChanged(CControl *control)
    {
        auto tag = control->getTag();
        if (!frame || knobs.find(tag) == knobs.end())
            return;

        auto knob = (CKnob *)control;
        auto value = knob->getValue();
        fstEffect->setParameterAutomated(tag, value);
        setParameter(tag, value);
    }

    void setParameter(int id, float value)
    {
        if (!frame || knobs.find(id) == knobs.end())
            return;

        knobs[id]->setValue(value);
        knobLabels[id]->setText(fstEffect->getParamDisplay(id).c_str());
    }
};
