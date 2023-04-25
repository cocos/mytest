#pragma once

#include "vec2.h"
#include "vec3.h"
#include <vector>

namespace LFX {

	struct XAOutVertex
	{
		int ref; // Index of input vertex from which this output vertex originated.
		Float2 uv; // Not normalized - values are in Atlas width and height range.
		int atlasIndex; // Sub-atlas index. -1 if the vertex doesn't exist in any atlas.
		int chartIndex; // -1 if the vertex doesn't exist in any chart.
	};

	struct XAInMesh
	{
		std::vector<Float3> positions;
		std::vector<Float3> normals;
		std::vector<int> indices;
		std::vector<int> faceMaterials;
	};

	struct XAOutMesh
	{
		int atlasWidth;
		int atlasHeight;
		std::vector<XAOutVertex> vertices;
		std::vector<int> indices;
	};

	bool XAUnwrap(XAOutMesh& out, const XAInMesh* mesh);

}