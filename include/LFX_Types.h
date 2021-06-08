#pragma once

#ifdef _MSC_VER
#include <windows.h>
#undef min
#undef max
#endif

#include <cassert>
#include <cmath>
#include <string>
#include <array>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <random>

#if 0
#ifdef LFX_EXPORT
#define LFX_ENTRY __declspec(dllexport)
#else
#define LFX_ENTRY __declspec(dllimport)
#endif
#endif

#define LFX_ENTRY

#define LFX_VERSION 31

// Features(too slow)
//#define LFX_FEATURE_EDGA_AA
//#define LFX_FEATURE_GI_MSAA

// Debug
//#define LFX_DEBUG_LUV

namespace LFX {

#ifndef FLT_MIN
#define FLT_MIN 1.175494351e-38F
#endif

#ifndef FLT_MAX
#define FLT_MAX 3.402823466e+38F
#endif

#define LFX_BVH_LEVELS 8

#define UNIT_LEN 1.0f
#define LMAP_BORDER 1
#define LMAP_OPTIMIZE_PX 1
#define LMAP_RGBE_EXPONENT_MAX 8.0f

#define d_assert assert
#define SAFE_DELETE(p) if (p) { delete p; p = NULL; }
#define SAFE_DELETE_ARRAY(p) if (p) { delete[] p; p = NULL; }

#ifdef _WIN32
#define LFX_USE_EMBREE_SCENE
#endif

	typedef std::string String;

	template <class T>
	struct Singleton
	{
		static T * msInstance;

	public:
		Singleton()
		{
			d_assert(msInstance == NULL);

			msInstance = static_cast<T *>(this);
		}

		~Singleton()
		{
			msInstance = NULL;
		}

		static T * Instance()
		{
			return msInstance;
		}
	};

#define ImplementSingleton(classname) \
	template<> classname* Singleton<classname>::msInstance = NULL;

	/**
	* @zh Ëæ»úÉú³ÉÆ÷
	*/
	struct LFX_ENTRY RandomEngine
	{
	public:
		RandomEngine(unsigned int seed = 0);

		unsigned int RandomUint();
		float RandomFloat();
		float UniformDistribution();

	public:
		std::mt19937 engine;
		std::uniform_real_distribution<float> distribution;
	};
}