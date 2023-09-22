#include "SaveObj.h"

namespace LFX {

	bool SaveObj(const String& file, const std::vector<OBJVert>& verts, const std::vector<int>& indices)
	{
		FILE* fp = fopen(file.c_str(), "rb");
		if (fp == nullptr) {
			LOGE("Write obj file '%s' failed", file.c_str());
			return false;
		}

		// v
		for (const auto& v : verts) {
			fprintf(fp, "v %f %f %f\n", v.p.x, v.p.y, v.p.z);
		}

		// vn
		for (const auto& v : verts) {
			fprintf(fp, "vn %f %f %f\n", v.n.x, v.n.y, v.n.z);
		}

		// vt
		for (const auto& v : verts) {
			fprintf(fp, "vt %f %f\n", v.uv.x, v.uv.y);
		}

		fclose(fp);
		return true;
	}

}