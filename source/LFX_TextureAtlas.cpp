#include "LFX_TextureAtlas.h"

namespace LFX {

	bool TextureAtlasPacker::Atlas::Extend(int w, int h, int channels)
	{
		if (Width > w && Height > h) {
			return false;
		}
		if (Width == w && Height == h) {
			return true;
		}

		int ow = Width;
		int oh = Height;

		// resize texture
		std::vector<unsigned char> newTex(w * h * channels);
		memset(&newTex[0], 0, newTex.size());
		Width = w; 
		Height = h;
		
		for (int y = 0; y < oh; ++y) {
			memcpy(&newTex[0], &Pixels[0], ow * channels);
		}

		Pixels = newTex;

		// recalc texcoord
		float scale = (float)w / (float)ow;
		for (auto& i : Items) {
			i.x *= scale;
			i.y *= scale;
			i.w *= scale;
			i.h *= scale;
		}

		return true;
	}

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

	int TextureAtlasPacker::Insert(unsigned char* pixels, int w, int h, Item& item)
	{
		const Rectangle<float>* pItem = NULL;
		for (size_t i = 0; i < mAtlasArray.size(); ++i)
		{
			pItem = _AtlasInsert(mAtlasArray[i], pixels, w, h, mOptions.Border, mOptions.Border);
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

		Atlas* pNewAtlas = new Atlas;
		pNewAtlas->Width = width;
		pNewAtlas->Height = height;
		pNewAtlas->Pixels.resize(pNewAtlas->Width * pNewAtlas->Height * mOptions.Channels);
		mAtlasArray.push_back(pNewAtlas);

		int uborder = mOptions.Border;
		int vborder = mOptions.Border;
		if (width < w + mOptions.Border * 2) {
			uborder = 0;
		}
		if (height < h + mOptions.Border * 2) {
			vborder = 0;
		}

		pItem = _AtlasInsert(pNewAtlas, pixels, w, h, uborder, vborder);

		item.index = mAtlasArray.size() - 1;
		item.offsetU = pItem->x;
		item.offsetV = pItem->y;
		item.scaleU = pItem->w;
		item.scaleV = pItem->h;

		return mAtlasArray.size() - 1;
	}

	const Rectangle<float> * TextureAtlasPacker::_AtlasInsert(Atlas * pAtlas, unsigned char * pixels, int w, int h, int uborder, int vborder)
	{
		int tw = w;
		int th = h;
		int aw = pAtlas->Width;
		int ah = pAtlas->Height;

		Rectangle<int> region;
		region.w = tw + uborder * 2;
		region.h = th + vborder * 2;

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
					return _AtlasAppend(pAtlas, pixels, region, uborder, vborder);
				}
			}
		}

		return NULL;
	}

	const Rectangle<int> * TextureAtlasPacker::_AtlasIntersect(Atlas * pAtlas, const Rectangle<int> & region)
	{
		for (size_t i = 0; i < pAtlas->Regions.size(); ++i)
		{
			const Rectangle<int> & ri = pAtlas->Regions[i];
			const Rectangle<int> & rect = ri & region;
			if (rect.w > 0 && rect.h > 0)
				return &pAtlas->Regions[i];
		}

		return NULL;
	}

	const Rectangle<float> * TextureAtlasPacker::_AtlasAppend(Atlas * pAtlas, unsigned char * pixels, const Rectangle<int> & region, int uborder, int vborder)
	{
		int width = region.w - uborder * 2;
		int height = region.h - vborder * 2;
		int channels = mOptions.Channels;

		for (int j = 0; j < region.w; ++j)
		{
			for (int i = 0; i < region.h; ++i)
			{
				int su = Clamp<int>(i - uborder, 0, width - 1);
				int sv = Clamp<int>(j - vborder, 0, height - 1);
				int srcIndex = j * width + i;

				int du = region.x + i;
				int dv = region.y + j;
				int dstIndex = (dv * pAtlas->Width + du);

				pAtlas->Pixels[dstIndex * channels + 0] = pixels[srcIndex * channels + 0];
				pAtlas->Pixels[dstIndex * channels + 1] = pixels[srcIndex * channels + 1];
				pAtlas->Pixels[dstIndex * channels + 2] = pixels[srcIndex * channels + 2];
				if (channels == 4) {
					pAtlas->Pixels[dstIndex * channels + 3] = pixels[srcIndex * channels + 3];
				}
			}
		}

		Rectangle<float> ai;
		ai.x = (region.x + uborder) / (float)pAtlas->Width;
		ai.y = (region.y + vborder) / (float)pAtlas->Height;
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