#include "LFX_ILBakerRandom.h"

namespace LFX { namespace ILBaker {

	void Random::SetSeed(unsigned int seed)
	{
		engine.seed(seed);
	}

	void Random::SeedWithRandomValue()
	{
		std::random_device device;
		engine.seed(device());
	}

	unsigned int Random::RandomUint()
	{
		return engine();
	}

	float Random::RandomFloat()
	{
		// return distribution(engine);
		return (RandomUint() & 0xFFFFFF) / float(1 << 24);
	}

	Float2 Random::RandomFloat2()
	{
		return Float2(RandomFloat(), RandomFloat());
	}

}}