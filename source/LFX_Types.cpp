#include "LFX_Types.h"

namespace LFX {

	RandomEngine::RandomEngine(unsigned int seed)
		: distribution(0.0f, 1.0f)
	{
		if (seed != 0) {
			engine.seed(seed);
		}
		else {
			std::random_device device;
			engine.seed(device());
		}
	}

	unsigned int RandomEngine::RandomUint()
	{
		return engine();
	}

	float RandomEngine::RandomFloat()
	{
		// return distribution(engine);
		return (RandomUint() & 0xFFFFFF) / float(1 << 24);
	}

	float RandomEngine::UniformDistribution()
	{
		return distribution(engine);
	}

}