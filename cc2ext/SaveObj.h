#pragma once

#include "LFX_Log.h"
#include "LFX_Mesh.h"

namespace LFX {

	struct OBJVert
	{
		Float3 p;
		Float3 n;
		Float2 uv;
	};

	bool SaveObj(const String& file, const std::vector<OBJVert>& verts, const std::vector<int>& indices);

}