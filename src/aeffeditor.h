// Compatibility wrapper for FST plugins that use VSTGUI library.
// Code derived by inspecting BSD 3-clause licenced VSTGUI source code.
// This file is licenced with GPL-3
#pragma once
#include "windows.h"
#include "FstAudioEffect.h"
#include "guibase.h"

class AEffEditor : public GuiBase
{
protected:
    FstAudioEffect *fstEffect;
    void *systemWindow = nullptr;

public:
    AEffEditor(FstAudioEffect *effect) : fstEffect(effect) {}
    virtual void idle() {}

    virtual ~AEffEditor() {}
};