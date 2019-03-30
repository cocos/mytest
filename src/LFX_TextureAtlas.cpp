#include "LFX_TextureAtlas.h"

namespace LFX {

	TextureAtlasPacker::TextureAtlasPacker(const Options& ops)
		: mOptions(ops)
	{
	}

	TextureAtlasPacker::~TextureAtlasPacker()
	{
		for (size_t i = 0; i < mAtlasArray.size(); ++i)
		{
			delete mAtlasArray[i];
		}
	}

	int TextureAtlasPacker::Insert(unsigned char * pixels, int w, int h, Item & item)
	{
		const Rectangle<float> * pItem = NULL;
		for (size_t i = 0; i < mAtlasArray.size(); ++i)
		{
			pItem = _AtlasInsert(mAtlasArray[i], pixels, w, h);
			if (pItem != NULL)
			{
				item.index = i;
				item.offsetU = pItem->x;
				item.offsetV = pItem->y;
				item.scaleU = pItem->w;
				item.scaleV = pItem->h;
				return i;
			}
		}

		//
		int width = std::max(w, mOptions.Width);
		int height = std::max(h, mOptions.Height);

		Atlas * pNewAtlas = new Atlas;
		pNewAtlas->Width = width;
		pNewAtlas->Height = height;
		pNewAtlas->Pixels.resize(pNewAtlas->Width * pNewAtlas->Height * 3);
		pItem = _AtlasInsert(pNewAtlas, pixels, w, h);
		mAtlasArray.push_back(pNewAtlas);

		item.index = mAtlasArray.size() - 1;
		item.offsetU = pItem->x;
		item.offsetV = pItem->y;
		item.scaleU = pItem->w;
		item.scaleV = pItem->h;

		return mAtlasArray.size() - 1;
	}

	const Rectangle<float> * TextureAtlasPacker::_AtlasInsert(Atlas * pAtlas, unsigned char * pixels, int w, int h)
	{
		int tw = w;
		int th =h;
		int aw = pAtlas->Width;
		int ah = pAtlas->Height;

		Rectangle<int> region;
		region.w = tw + mOptions.Border * 2;
		region.h = th + mOptions.Border * 2;

		for (int v = 0; v < ah; ++v) {
			region.y = v;
			if (region.bottom() > ah)
				break;

			for (int u = 0; u < aw; ++u) {
				region.x = u;
				if (region.right() > aw)
					break;

				const Rectangle<int> * test = _AtlasIntersect(pAtlas, region);
				if (test != NULL) {
					u = std::max(u, test->right() - 1);
				}
				else {
					return _AtlasAppend(pAtlas, pixels, region);
				}
			}
		}

		return NULL;
	}

	const Rectangle<int> * TextureAtlasPacker::_AtlasIntersect(Atlas * pAtlas, const Rectangle<int> & region)
	{
		for (size_t i = 0; i < pAtlas->Regions.size(); ++i)
		{
			Rectangle<int> & rect = pAtlas->Regions[i] & region;
			if (rect.w > 0 && rect.h > 0)
				return &pAtlas->Regions[i];
		}

		return NULL;
	}

	const Rectangle<float> * TextureAtlasPacker::_AtlasAppend(Atlas * pAtlas, unsigned char * pixels, const Rectangle<int> & region)
	{
		int width = region.w - mOptions.Border * 2;
		int height = region.h - mOptions.Border * 2;

		for (int j = 0; j < region.w; ++j)
		{
			for (int i = 0; i < region.h; ++i)
			{
				int su = i - mOptions.Border;
				int sv = j - mOptions.Border;
				su = Clamp<int>(su, 0, width - 1);
				sv = Clamp<int>(sv, 0, height - 1);
				int srcIndex = j * width + i;

				int du = region.x + i;
				int dv = region.y + j;
				int dstIndex = (dv * pAtlas->Width + du);

				pAtlas->Pixels[dstIndex * 3 + 0] = pixels[srcIndex * 3 + 0];
				pAtlas->Pixels[dstIndex * 3 + 1] = pixels[srcIndex * 3 + 1];
				pAtlas->Pixels[dstIndex * 3 + 2] = pixels[srcIndex * 3 + 2];
			}
		}

		Rectangle<float> ai;
		ai.x = (region.x + mOptions.Border) / (float)pAtlas->Width;
		ai.y = (region.y + mOptions.Border) / (float)pAtlas->Height;
		ai.w = width / (float)pAtlas->Width;
		ai.h = height / (float)pAtlas->Height;
		pAtlas->Items.push_back(ai);

		Rectangle<int> rcWithSpace = region;
		rcWithSpace.w += mOptions.Space;
		rcWithSpace.h += mOptions.Space;
		pAtlas->Regions.push_back(rcWithSpace);

		return &pAtlas->Items[pAtlas->Items.size() - 1];
	}

	std::vector<TextureAtlasPacker::Atlas *> & TextureAtlasPacker::GetAtlasArray()
	{
		return mAtlasArray;
	}

}