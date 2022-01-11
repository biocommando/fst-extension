// Compatibility wrapper for FST plugins that use VSTGUI library.
// Code derived by inspecting BSD 3-clause licenced VSTGUI source code.
// This file is licenced with GPL-3
#pragma once

#include "../FST-master/fst/fst.h"

class GuiBase
{
public:
    virtual bool open(void *ptr) { return false; }
    virtual void close() {}
    virtual bool getRect(ERect **ppRect) { return false; }
    virtual void draw(ERect *pRect) {}
    virtual bool getFrameDirtyStatus() { return false; }
    virtual ~GuiBase() {}
};