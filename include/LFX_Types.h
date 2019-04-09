#pragma once

#ifdef _MSC_VER
#include <windows.h>
#undef min
#undef max
#endif

#include <cassert>
#include <cmath>
#include <array>
#include <vector>
#include <list>
#include <map>
#include <algorithm>

#if 0
#ifdef LFX_EXPORT
#define LFX_ENTRY __declspec(dllexport)
#else
#define LFX_ENTRY __declspec(dllimport)
#endif
#endif

#define LFX_ENTRY

// Features(too slow)
//#define LFX_FEATURE_EDGA_AA
//#define LFX_FEATURE_GI_MSAA

namespace LFX {

	typedef std::string String;

#ifndef FLT_MIN
#define FLT_MIN 1.175494351e-38F
#endif

#ifndef FLT_MAX
#define FLT_MAX 3.402823466e+38F
#endif

#define UNIT_LEN 1.0f

#define LOGI(fmt, ...)
#define LOGD(fmt, ...)
#define LOGW(fmt, ...)
#define LOGE(fmt, ...)

#define d_assert assert
#define SAFE_DELETE(p) if (p) { delete p; p = NULL; }

}