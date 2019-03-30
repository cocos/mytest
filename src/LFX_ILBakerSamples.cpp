#include "LFX_ILBakerSamples.h"

namespace LFX { namespace ILBaker {

	template<typename T> void Shuffle(std::vector<T>& values, Random& randomGenerator)
	{
		const int count = values.size();
		for (int i = 0; i < count; ++i)
		{
			int other = i + (randomGenerator.RandomUint() % (count - i));
			Swap(values[i], values[other]);
		}
	}

	template<typename T> void Shuffle(T* values, int count, Random& randomGenerator)
	{
		for (int i = 0; i < count; ++i)
		{
			int other = i + (randomGenerator.RandomUint() % (count - i));
			Swap(values[i], values[other]);
		}
	}


	static unsigned int CMJPermute(unsigned int i, unsigned int l, unsigned int p)
	{
		unsigned int w = l - 1;
		w |= w >> 1;
		w |= w >> 2;
		w |= w >> 4;
		w |= w >> 8;
		w |= w >> 16;
		do
		{
			i ^= p; i *= 0xe170893d;
			i ^= p >> 16;
			i ^= (i & w) >> 4;
			i ^= p >> 8; i *= 0x0929eb3f;
			i ^= p >> 23;
			i ^= (i & w) >> 1; i *= 1 | p >> 27;
			i *= 0x6935fa69;
			i ^= (i & w) >> 11; i *= 0x74dcb303;
			i ^= (i & w) >> 2; i *= 0x9e501cc3;
			i ^= (i & w) >> 2; i *= 0xc860a3df;
			i &= w;
			i ^= i >> 5;
		} while (i >= l);
		return (i + p) % l;
	}

	static float CMJRandFloat(unsigned int i, unsigned int p)
	{
		i ^= p;
		i ^= i >> 17;
		i ^= i >> 10; i *= 0xb36534e5;
		i ^= i >> 12;
		i ^= i >> 21; i *= 0x93fc4795;
		i ^= 0xdf6e307f;
		i ^= i >> 17; i *= 1 | p >> 18;
		return i * (1.0f / 4294967808.0f);
	}

	// Returns a 2D sample from a particular pattern using correlated multi-jittered sampling [Kensler 2013]
	Float2 SampleCMJ2D(int sampleIdx, int numSamplesX, int numSamplesY, int pattern)
	{
		int N = numSamplesX * numSamplesY;
		sampleIdx = CMJPermute(sampleIdx, N, pattern * 0x51633e2d);
		int sx = CMJPermute(sampleIdx % numSamplesX, numSamplesX, pattern * 0x68bc21eb);
		int sy = CMJPermute(sampleIdx / numSamplesX, numSamplesY, pattern * 0x02e5be93);
		float jx = CMJRandFloat(sampleIdx, pattern * 0x967a889b);
		float jy = CMJRandFloat(sampleIdx, pattern * 0x368cc8b7);
		return Float2((sx + (sy + jx) / numSamplesY) / numSamplesX, (sampleIdx + jy) / N);
	}

	void GenerateCMJSamples2D(Float2* samples, int numSamplesX, int numSamplesY, int pattern)
	{
		const int numSamples = numSamplesX * numSamplesY;
		for (int i = 0; i < numSamples; ++i)
			samples[i] = SampleCMJ2D(i, numSamplesX, numSamplesY, pattern);
	}

	void GenerateIntegrationSamples(IntegrationSamples& samples,
		int sqrtNumSamples, int tileSizeX, int tileSizeY, int numIntegrationTypes, Random & rdm)
	{
		const int numSamplesPerPixel = sqrtNumSamples * sqrtNumSamples;
		const int numTilePixels = tileSizeX * tileSizeY;
		const int numSamplesPerTile = numSamplesPerPixel * numTilePixels;
		samples.Init(numTilePixels, numIntegrationTypes, numSamplesPerPixel);

		for (int pixelIdx = 0; pixelIdx < numTilePixels; ++pixelIdx)
		{
			for (int typeIdx = 0; typeIdx < numIntegrationTypes; ++typeIdx)
			{
				Float2* typeSamples = samples.GetSamplesForType(pixelIdx, typeIdx);

				GenerateCMJSamples2D(typeSamples, sqrtNumSamples, sqrtNumSamples, rdm.RandomUint());
				Shuffle(typeSamples, numSamplesPerPixel, rdm);
			}
		}
	}

}}