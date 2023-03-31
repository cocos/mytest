#pragma once

#include "LFX_Math.h"

namespace LFX {

	struct Texture
	{
		String name;
		int width;
		int height;
		int channels;
		std::vector<uint8_t> data;

		Texture()
		{
			channels = 0;
			width = 0;
			height = 0;
		}

		~Texture()
		{
		}

		int GetWidth() const
		{
			return width;
		}

		int GetHeight() const
		{
			return height;
		}

		bool GetColor(Float4& c, int x, int y) const;
		Float4 SampleColor(float u, float v, bool repeat = true);
		Float3 SampleColor3(float u, float v, bool repeat = true);
		static Float4 SampleCube(Texture* cubemap[6], const Float3& normal);
	};

}