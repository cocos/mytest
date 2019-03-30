#include "LFX_Rasterizer.h"

namespace LFX {

	Rasterizer::Rasterizer(int width, int height)
		: _width(width)
		, _height(height)
	{
		assert(width > 16 && height > 16);

		_rmap.resize(width * height);
	}

	Rasterizer::~Rasterizer()
	{
	}

	void Rasterizer::DoBlur(Float3 * data, int xMapSize, int zMapSize, int stride)
	{
		Float3 * temp = new Float3[xMapSize * zMapSize];

		int index = 0;
		for (int v = 0; v < zMapSize; ++v)
		{
			for (int u = 0; u < xMapSize; ++u)
			{
				Float3 color(0, 0, 0);

				for (int y = -1; y <= 1; ++y)
				{
					for (int x = -1; x <= 1; ++x)
					{
						int s = Clamp<int>(u + x, 0, xMapSize - 1);
						int t = Clamp<int>(v + y, 0, zMapSize - 1);

						color += data[(t * stride) + s];
					}
				}

				color /= 9.0f;

				temp[index++] = color;
			}
		}

		for (int i = 0; i < zMapSize; ++i)
		{
			memcpy(data + i * stride, temp + i * xMapSize, xMapSize * sizeof(Float3));
		}

		
		delete[] temp;
	}

}