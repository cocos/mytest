#include "uvunwarp.h"
#include <iostream>

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
		std::cerr << "Invalid number of vertex " << numVertices << "!" << std::endl;
		return false;
	}

	int numIndices = 0;
	fread(&numIndices, sizeof(int), 1, fp);
	if (numIndices <= 0) {
		std::cerr << "Invalid number of index " << numIndices << "!" << std::endl;
		return false;
	}
	if (numIndices % 3 != 0) {
		std::cerr << " Index number " << numIndices << "is not triangle list" << std::endl;
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
		std::cerr << "missing input file argment!!!" << std::endl;
		return -1;
	}

	if (GOutputFile == nullptr) {
		std::cerr << "missing output file argment!!!" << std::endl;
		return -1;
	}

	FILE* inFile = fopen(GInputFile, "rb");
	if (inFile == nullptr) {
		std::cerr << "open input file '" << GInputFile << "' faild!!!" << std::endl;
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
		std::cerr << "Error: open output file '" << GInputFile << "' faild!!!" << std::endl;
		return -1;
	}

	SaveOutMesh(outFile, outMesh);
	fclose(outFile);
	return 0;
}