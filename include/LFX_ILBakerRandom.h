#pragma once

#include <random>
#include "LFX_Math.h"

namespace LFX { namespace ILBaker {

	class Random
	{
	public:
		void SetSeed(unsigned int seed);
		void SeedWithRandomValue();

		unsigned int RandomUint();
		float RandomFloat();
		Float2 RandomFloat2();

	private:
		std::mt19937 engine;
		std::uniform_real_distribution<float> distribution;
	};

}}