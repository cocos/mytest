#include "uvunwarp.h"
#include "xaunwrap.h"
#include <iostream>

using namespace LFX;

const char* GInputFile = nullptr;
const char* GOutputFile = nullptr;

#define _XA_UNWRAP

/**
* NLInMesh File Struct
* numVertices (int)
* numIndices (int)
* vertexData (Float3 * numVertices)
* indexData (int * numIndices)
*/
bool NLReadInMesh(FILE* fp, NLInMesh& mesh)
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
* NLOutMesh File Struct
* numVertices (int)
* vertexData (Float2 * numVertices)
* numIndices (int)
* indexData (int * numIndices)
*/
void NLSaveOutMesh(FILE* fp, NLOutMesh& mesh)
{
	int numVertices = mesh.uvs.size();
	fwrite(&numVertices, sizeof(int), 1, fp);
	fwrite(mesh.uvs.data(), sizeof(Float2), mesh.uvs.size(), fp);

	int numIndices = mesh.indices.size();
	fwrite(&numIndices, sizeof(int), 1, fp);
	fwrite(mesh.indices.data(), sizeof(int), mesh.indices.size(), fp);

	printf("Info: Output mesh %d vertices, %d indices", numVertices, numIndices);
}

/**
* NLInMesh File Struct
* numVertices (int)
* numNormals (int)
* numIndices (int)
* numFaceMaterials (int)
* vertexData (Float3 * numVertices)
* indexData (int * numIndices)
*/
bool XAReadInMesh(FILE* fp, XAInMesh& mesh)
{
	int numVertices = 0;
	fread(&numVertices, sizeof(int), 1, fp);
	if (numVertices <= 0) {
		std::cerr << "Invalid number of position " << numVertices << "!" << std::endl;
		return false;
	}

	int numNormals = 0;
	fread(&numNormals, sizeof(int), 1, fp);
	if (numNormals != 0 && numNormals != numVertices) {
		std::cerr << "Invalid number of normal " << numNormals << "!" << std::endl;
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

	int numFaceMaterials = 0;
	fread(&numFaceMaterials, sizeof(int), 1, fp);
	if (numFaceMaterials != 0 && numFaceMaterials % 3 != 0) {
		std::cerr << " Face materials number " << numIndices << "is not match to face" << std::endl;
		return false;
	}

	// Read goemotry
	mesh.positions.resize(numVertices);
	mesh.normals.resize(numNormals);
	mesh.indices.resize(numIndices);
	mesh.faceMaterials.resize(numFaceMaterials);
	fread(mesh.positions.data(), sizeof(Float3), mesh.positions.size(), fp);
	if (!mesh.normals.empty()) {
		fread(mesh.normals.data(), sizeof(Float3), mesh.normals.size(), fp);
	}
	fread(mesh.indices.data(), sizeof(int), mesh.indices.size(), fp);
	if (!mesh.faceMaterials.empty()) {
		fread(mesh.faceMaterials.data(), sizeof(Float3), mesh.faceMaterials.size(), fp);
	}

	printf("Info: Input mesh %d vertices, %d indices", numVertices, numIndices);
	return true;
}

/**
* NLOutMesh File Struct
* numVertices (int)
* vertexData (Float2 * numVertices)
* vertexRef (int * numVertices)
* numIndices (int)
* indexData (int * numIndices)
*/
void XASaveOutMesh(FILE* fp, XAOutMesh& mesh)
{
	int numVertices = mesh.vertices.size();
	fwrite(&numVertices, sizeof(int), 1, fp);
	for (int i = 0; i < numVertices; ++i) {
		fwrite(&mesh.vertices[i].uv, sizeof(Float2), 1, fp);
	}
	for (int i = 0; i < numVertices; ++i) {
		fwrite(&mesh.vertices[i].ref, sizeof(int), 1, fp);
	}

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

#ifdef _XA_UNWRAP
	XAInMesh inMesh;
	if (!XAReadInMesh(inFile, inMesh)) {
		fclose(inFile);
		return -1;
	}
	fclose(inFile);

	XAOutMesh outMesh;
	XAUnwrap(outMesh, &inMesh);

	FILE* outFile = fopen(GOutputFile, "wb");
	if (outFile == nullptr) {
		std::cerr << "Error: open output file '" << GInputFile << "' faild!!!" << std::endl;
		return -1;
	}

	XASaveOutMesh(outFile, outMesh);
	fclose(outFile);
#else
	NLInMesh inMesh;
	if (!NLReadInMesh(inFile, inMesh)) {
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

	NLSaveOutMesh(outFile, outMesh);
	fclose(outFile);
#endif

	return 0;
}