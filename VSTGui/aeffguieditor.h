//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 3.6
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2008, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------
#ifndef __aeffguieditor__
#define __aeffguieditor__

#ifndef __aeffeditor__
#include "../src/aeffeditor.h"
#endif

/*#ifndef __audioeffectx__
#include "audioeffectx.h"
#endif*/

#include "FstAudioEffect.h"

#ifndef __vstgui__
#include "vstgui.h"
#endif

//-----------------------------------------------------------------------------
// AEffGUIEditor Declaration
//-----------------------------------------------------------------------------
class AEffGUIEditorFst : public AEffEditor, public VSTGUIEditorInterface
{
public :

	AEffGUIEditorFst (void* pEffect);

	virtual ~AEffGUIEditorFst ();

	virtual void setParameter (VstInt32 index, float value) {} 
	virtual bool getRect (ERect** ppRect);
	virtual bool open (void* ptr);
	virtual void idle ();
	virtual void draw (ERect* pRect);

	#if VST_2_1_EXTENSIONS
	virtual bool onKeyDown (VstKeyCode& keyCode);
	virtual bool onKeyUp (VstKeyCode& keyCode);
	#endif

	#if MAC
	virtual long mouse (long x, long y);
	#endif

	// wait (in ms)
	void wait (unsigned long ms);

	// get the current time (in ms)
	unsigned long getTicks ();

	// feedback to appli.
	virtual void doIdleStuff ();

	// get the effect attached to this editor
	//AudioEffect* getEffect () { return effect; }
	FstAudioEffect* getEffect () { return fstEffect; }

	// get version of this VSTGUI
	long getVstGuiVersion () { return (VSTGUI_VERSION_MAJOR << 16) + VSTGUI_VERSION_MINOR; }

	// set/get the knob mode
	virtual bool setKnobMode (int val);
	virtual long getKnobMode () const { return knobMode; }

	virtual bool onWheel (float distance);

	// get the CFrame object
	#if USE_NAMESPACE
	VSTGUI::CFrame* getFrame () { return frame; }
	#else
	CFrame* getFrame () { return frame; }
	#endif

#if VST_2_1_EXTENSIONS
	virtual void beginEdit (long index) { ((AudioEffectX*)effect)->beginEdit (index); }
	virtual void endEdit (long index)   { ((AudioEffectX*)effect)->endEdit (index); }
#endif

	bool getFrameDirtyStatus()
	{
		return frame != nullptr && frame->isDirty();
	}

//---------------------------------------
protected:
	ERect   rect;
private:
	unsigned long lLastTicks;
	bool inIdleStuff;

	static long knobMode;
};
#ifdef VST_COMPAT
#include "audioeffectx.h"

class AEffGUIEditor : public AEffGUIEditorFst
{
	public:
	AEffGUIEditor(void *effect) : AEffGUIEditorFst(effect), effect((AudioEffectX*)effect)
	{
	}
	virtual ~AEffGUIEditor() {}
	protected:
	AudioEffectX *effect;
};

#endif
#endif
