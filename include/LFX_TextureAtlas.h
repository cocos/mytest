#pragma once

#include "LFX_Math.h"
#include "LFX_Geom.h"

namespace LFX {

	class LFX_ENTRY TextureAtlasPacker
	{
	public:
		struct Options
		{
			int Width, Height;
			int Channels;
			int Space;
			int Border;

			Options()
			{
				Width = Height = 1024;
				Channels = 3;
				Space = 0;
				Border = 0;
			}
		};

		struct Item
		{
			int index;
			float offsetU;
			float offsetV;
			float scaleU;
			float scaleV;
		};

		struct Atlas
		{
			int Width, Height;
			std::vector<unsigned char> Pixels;
			std::vector<Rectangle<int> > Regions;
			std::vector<Rectangle<float> > Items;
		};

	public:
		TextureAtlasPacker(const Options& ops);
		~TextureAtlasPacker();

		int
			Insert(unsigned char * pixels, int w, int h, Item & item);

		std::vector<Atlas *> &
			GetAtlasArray();

	protected:
		const Rectangle<float> *
			_AtlasInsert(Atlas * pAtlas, unsigned char * pixels, int w, int h, int uborder, int vborder);
		const Rectangle<int> *
			_AtlasIntersect(Atlas * pAtlas, const Rectangle<int> & region);
		const Rectangle<float> *
			_AtlasAppend(Atlas * pAtlas, unsigned char * pixels, const Rectangle<int> & region, int uborder, int vborder);

	protected:
		Options mOptions;
		std::vector<Atlas *> mAtlasArray;
	};

}