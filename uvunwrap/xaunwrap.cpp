#/*
* Slay Engine
* Author: SiZhong.Wang(M-001)
* Copyright: Silvereye Information, Inc. All Rights Reserved.
*/
#include "xaunwrap.h"

#include <mutex>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "xatlas.h"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifdef _MSC_VER
#define FOPEN(_file, _filename, _mode) { if (fopen_s(&_file, _filename, _mode) != 0) _file = NULL; }
#define STRICMP _stricmp
#else
#define FOPEN(_file, _filename, _mode) _file = fopen(_filename, _mode)
#include <strings.h>
#define STRICMP strcasecmp
#endif

static bool s_verbose = false;

class Stopwatch
{
public:
	Stopwatch() { reset(); }
	void reset() { m_start = clock(); }
	double elapsed() const { return (clock() - m_start) * 1000.0 / CLOCKS_PER_SEC; }
private:
	clock_t m_start;
};

static int Print(const char* format, ...)
{
	va_list arg;
	va_start(arg, format);
	printf("\r"); // Clear progress text.
	const int result = vprintf(format, arg);
	va_end(arg);
	return result;
}

// May be called from any thread.
static bool ProgressCallback(xatlas::ProgressCategory category, int progress, void* userData)
{
	// Don't interupt verbose printing.
	if (s_verbose)
		return true;
	Stopwatch* stopwatch = (Stopwatch*)userData;
	static std::mutex progressMutex;
	std::unique_lock<std::mutex> lock(progressMutex);
	if (progress == 0)
		stopwatch->reset();
	printf("\r   %s [", xatlas::StringForEnum(category));
	for (int i = 0; i < 10; i++)
		printf(progress / ((i + 1) * 10) ? "*" : " ");
	printf("] %d%%", progress);
	fflush(stdout);
	if (progress == 100) {
		printf("\n      %.2f seconds (%g ms) elapsed\n", stopwatch->elapsed() / 1000.0, stopwatch->elapsed());
	}

	return true;
}

namespace LFX {

	bool XAUnwrap(XAOutMesh& out, const XAInMesh* mesh)
	{
		xatlas::SetPrint(Print, false);
		xatlas::Atlas* atlas = xatlas::Create();

		// Set progress callback.
		Stopwatch globalStopwatch, stopwatch;
		xatlas::SetProgressCallback(atlas, ProgressCallback, &stopwatch);

		xatlas::MeshDecl meshDecl;
		meshDecl.vertexCount = mesh->positions.size();
		meshDecl.vertexPositionData = mesh->positions.data();
		meshDecl.vertexPositionStride = sizeof(Float3);
		if (!mesh->normals.empty()) {
			meshDecl.vertexNormalData = mesh->normals.data();
			meshDecl.vertexNormalStride = sizeof(Float3);
		}
		meshDecl.indexCount = mesh->indices.size();
		meshDecl.indexData = mesh->indices.data();
		meshDecl.indexFormat = xatlas::IndexFormat::UInt32;
		if (!mesh->faceMaterials.empty()) {
			meshDecl.faceMaterialData = (const uint32_t*)mesh->faceMaterials.data();
		}

		xatlas::AddMeshError error = xatlas::AddMesh(atlas, meshDecl, 1);
		if (error != xatlas::AddMeshError::Success) {
			xatlas::Destroy(atlas);
			printf("\rError adding mesh: %s\n", xatlas::StringForEnum(error));
			return false;
		}

		xatlas::AddMeshJoin(atlas);
		printf("   %d total vertices\n", (int)mesh->positions.size());
		printf("   %d total faces\n", (int)mesh->indices.size() / 3);
		xatlas::ChartOptions chartOps;
		xatlas::PackOptions packOps;
		//packOps.resolution = 2048;
		xatlas::Generate(atlas, chartOps, packOps);
		printf("   %d charts\n", atlas->chartCount);
		printf("   %d atlases\n", atlas->atlasCount);
		for (uint32_t i = 0; i < atlas->atlasCount; i++) {
			printf("      %d: %0.2f%% utilization\n", i, atlas->utilization[i] * 100.0);
		}
		printf("   %ux%u resolution\n", atlas->width, atlas->height);
		printf("   %u total vertices\n", atlas->meshes[0].vertexCount);
		printf("%.2f seconds (%g ms) elapsed total\n", globalStopwatch.elapsed() / 1000.0, globalStopwatch.elapsed());

		bool successed = false;

		out.atlasWidth = atlas->width;
		out.atlasHeight = atlas->height;
		for (uint32_t i = 0; i < atlas->meshCount; i++) {
			const xatlas::Mesh& xmesh = atlas->meshes[i];
#if 0
			for (uint32_t j = 0; j < xmesh.indexCount; j += 3) {
				int atlasIndex = -1;
				bool skip = false;
				IVec2 uvs[3];
				for (int k = 0; k < 3; k++) {
					const xatlas::Vertex& v = xmesh.vertexArray[mesh.indexArray[j + k]];
					if (v.atlasIndex == -1) {
						skip = true;
						break;
					}
					atlasIndex = v.atlasIndex;
					uvs[k].x = int(v.uv[0]);
					uvs[k].y = int(v.uv[1]);
				}
				if (skip) {
					// Skip triangles that weren't atlased.
					continue;
				}
			}
#endif
			out.vertices.resize(xmesh.vertexCount);
			for (uint32_t j = 0; j < xmesh.vertexCount; ++j) {
				XAOutVertex& dst = out.vertices[j];
				const xatlas::Vertex& v = xmesh.vertexArray[j];
				dst.ref = v.xref;
				dst.uv.x = v.uv[0];
				dst.uv.y = v.uv[1];
				dst.chartIndex = v.chartIndex;
				dst.atlasIndex = v.atlasIndex;

				if (v.atlasIndex == -1) {
					// Skip triangles that weren't atlased.
					continue;
				}
			}

			out.indices.resize(xmesh.indexCount);
			for (uint32_t j = 0; j < xmesh.indexCount; ++j) {
				out.indices[j] = xmesh.indexArray[j];
			}

			successed = true;
		}

		xatlas::Destroy(atlas);

		return successed;
	}

}