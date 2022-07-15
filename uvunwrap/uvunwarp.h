#pragma once

#include "vec2.h"
#include "vec3.h"
#include "vector"

namespace LFX {

	struct NLInMesh
	{
		std::vector<Float3> vertices;
		std::vector<Int3> triangles;
	};

	struct NLOutMesh
	{
		std::vector<Float2> uvs;
		std::vector<int> indices;
	};

	void Unwrap(NLOutMesh& out, const NLInMesh* mesh);

}