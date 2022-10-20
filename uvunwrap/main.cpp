#include "uvunwarp.h"

using namespace LFX;

const char* GInputFile = nullptr;
const char* GOutputFile = nullptr;

/**
* File Struct
* numVertices (int)
* numIndices (int)
* vertexData (Float3 * numVertices)
* indexData (int * numIndices)
*/
bool ReadInMesh(FILE* fp, NLInMesh& mesh)
{
	int numVertices = 0;
	fread(&numVertices, sizeof(int), 1, fp);
	if (numVertices <= 0) {
		printf("Error: Invalid number vertex %d!!", numVertices);
		return false;
	}

	int numIndices = 0;
	fread(&numIndices, sizeof(int), 1, fp);
	if (numIndices <= 0) {
		printf("Error: Invalid number index %d!!", numIndices);
		return false;
	}

	mesh.vertices.resize(numVertices);
	mesh.triangles.resize(numIndices / 3);
	fread(mesh.vertices.data(), sizeof(Float3), mesh.vertices.size(), fp);
	fread(mesh.triangles.data(), sizeof(Int3), mesh.triangles.size(), fp);

	printf("Info: Input mesh %d vertices, %d indices", numVertices, numIndices);

	return true;
}

/**
* File Struct
* numVertices (int)
* vertexData (Float2 * numVertices)
* numIndices (int)
* indexData (int * numIndices)
*/
void SaveOutMesh(FILE* fp, NLOutMesh& mesh)
{
	int numVertices = mesh.uvs.size();
	fwrite(&numVertices, sizeof(int), 1, fp);
	fwrite(mesh.uvs.data(), sizeof(Float2), mesh.uvs.size(), fp);

	int numIndices = mesh.indices.size();
	fwrite(&numIndices, sizeof(int), 1, fp);
	fwrite(mesh.indices.data(), sizeof(int), mesh.indices.size(), fp);

	printf("Info: Output mesh %d vertices, %d indices", numVertices, numIndices);
}

int main(int argc, char* argv[])
{
	for (int i = 0; i < argc; ++i) {
		if (strcmp(argv[i], "--input") == 0) {
			if (i + 1 < argc) {
				GInputFile = argv[++i];
			}
		}
		else if (strcmp(argv[i], "--output") == 0) {
			if (i + 1 < argc) {
				GOutputFile = argv[++i];
			}
		}
	}

	if (GInputFile == nullptr) {
		printf("Error: missing input file argment!!!");
		return -1;
	}

	if (GOutputFile == nullptr) {
		printf("Error: missing output file argment!!!");
		return -1;
	}

	FILE* inFile = fopen(GInputFile, "rb");
	if (inFile == nullptr) {
		printf("Error: open input file '%s' faild!!!", GInputFile);
		return -1;
	}

	NLInMesh inMesh;
	if (!ReadInMesh(inFile, inMesh)) {
		fclose(inFile);
		return -1;
	}
	fclose(inFile);

	NLOutMesh outMesh;
	Unwrap(outMesh, &inMesh);

	FILE* outFile = fopen(GOutputFile, "wb");
	if (outFile == nullptr) {
		printf("Error: open output file '%s' faild!!!", GInputFile);
		return -1;
	}

	SaveOutMesh(outFile, outMesh);
	fclose(outFile);

	return 0;
}