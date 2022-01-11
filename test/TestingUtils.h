#pragma once

#include "../src/FstAudioEffect.h"
#include <string>
#include <sstream>
#include <vector>

static std::vector<std::string> masterDispatcherCapture;
static void *masterDispatcherReturnValue = nullptr;

t_fstPtrInt masterDispatcher(struct _fstEffect *a, int b, int c, t_fstPtrInt d, void *const e, float f)
{
    std::stringstream ss;
    ss << b << ',' << c << ',' << d << ',' << reinterpret_cast<long long>(e) << ',' << f;
    masterDispatcherCapture.push_back(ss.str());
    return reinterpret_cast<t_fstPtrInt>(masterDispatcherReturnValue);
}

#define VOID_CAST(x) const_cast<void*>(reinterpret_cast<const void*>(x))