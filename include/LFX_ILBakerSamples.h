#pragma once

#include "LFX_Types.h"
#include "LFX_ILBakerMath.h"
#include "LFX_ILBakerRandom.h"

namespace LFX { namespace ILBaker {

	struct IntegrationSamples
	{
		std::vector<Float2> Samples;
		int NumPixels = 0;
		int NumTypes = 0;
		int NumSamples = 0;

		void Init(int numPixels, int numTypes, int numSamples)
		{
			NumPixels = numPixels;
			NumSamples = numSamples;
			NumTypes = numTypes;
			Samples.resize(numPixels * numTypes * numSamples);
		}

		int ArrayIndex(int pixelIdx, int typeIdx, int sampleIdx) const
		{
			assert(pixelIdx < NumPixels);
			assert(typeIdx < NumTypes);
			assert(sampleIdx < NumSamples);
			return pixelIdx * (NumSamples * NumTypes) + typeIdx * NumSamples + sampleIdx;
		}

		Float2 GetSample(int pixelIdx, int typeIdx, int sampleIdx) const
		{
			const int idx = ArrayIndex(pixelIdx, typeIdx, sampleIdx);
			return Samples[idx];
		}

		Float2* GetSamplesForType(int pixelIdx, int typeIdx)
		{
			const int startIdx = ArrayIndex(pixelIdx, typeIdx, 0);
			return &Samples[startIdx];
		}

		const Float2* GetSamplesForType(int pixelIdx, int typeIdx) const
		{
			const int startIdx = ArrayIndex(pixelIdx, typeIdx, 0);
			return &Samples[startIdx];
		}

		void GetSampleSet(int pixelIdx, int sampleIdx, Float2* sampleSet) const
		{
			assert(pixelIdx < NumPixels);
			assert(sampleIdx < NumSamples);
			assert(sampleSet != nullptr);
			const int typeStride = NumSamples;
			int idx = pixelIdx * (NumSamples * NumTypes) + sampleIdx;
			for (int typeIdx = 0; typeIdx < NumTypes; ++typeIdx)
			{
				sampleSet[typeIdx] = Samples[idx];
				idx += typeStride;
			}
		}
	};

	void GenerateIntegrationSamples(IntegrationSamples& samples, int sqrtNumSamples,
		int tileSizeX, int tileSizeY, int numIntegrationTypes, Random & rdm);

}}