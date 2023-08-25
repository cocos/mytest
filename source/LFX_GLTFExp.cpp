#include "LFX_GLTFExp.h"
#include "LFX_World.h"
#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

namespace LFX {

	struct GLTFModelExp
	{
		tinygltf::Model model;
		std::map<String, int> textureMap;

		GLTFModelExp()
		{
			model.asset.version = "2.0";
			model.asset.generator = "LFX_GLTFExp";
		}

		int ExportTexture(const String& texture);
		int ExportMaterial(const Material* mtl);
		int ExportCamera(Camera* camera);
		int ExportLight(Light* light, int index);
		int ExportMesh(Mesh* mesh, int index);

		bool WriteToFile(const String& file, bool binary)
		{
			std::ofstream stream;
			stream.open(file.c_str());
			if (stream.is_open()) {
				tinygltf::TinyGLTF gltf;
				if (!gltf.WriteGltfSceneToStream(&model, stream, true, binary)) {
					return false;
				}

				return true;
			}

			return false;
		}
	};

	int GLTFModelExp::ExportTexture(const String& texture)
	{
		auto iter = textureMap.find(texture);
		if (iter != textureMap.end()) {
			return iter->second;
		}

		tinygltf::Image gltfImage;
		gltfImage.uri = texture.c_str();
		model.images.push_back(gltfImage);

		tinygltf::Texture gltfTexture;
		gltfTexture.source = (int)model.images.size() - 1;
		model.textures.push_back(gltfTexture);
		return (int)model.textures.size() - 1;
	}

	int GLTFModelExp::ExportMaterial(const Material* mtl)
	{
		tinygltf::Material gltfMaterial;

		// base color
		gltfMaterial.values["baseColorFactor"].number_array = {
			(double)mtl->Diffuse.x,
			(double)mtl->Diffuse.y,
			(double)mtl->Diffuse.z,
			(double)1.0,
		};
		// metallic
		gltfMaterial.values["metallicFactor"].number_value = (double)mtl->Metallic;
		// roughness
		gltfMaterial.values["roughnessFactor"].number_value = (double)mtl->Roughness;

		// combine metallic and roughness
		if (mtl->_pbrMapFile != "") {
			int texIdx = ExportTexture(mtl->_pbrMapFile);
			gltfMaterial.values["metallicRoughnessTexture"].json_double_value["index"] = texIdx;
		}

		// normal map
		if (mtl->_normalMapFile != "") {
			int texIdx = ExportTexture(mtl->_normalMapFile);
			gltfMaterial.values["normalTexture"].json_double_value["index"] = texIdx;
		}

		// emissive
		if (mtl->_emissiveMapFile != "") {
			int texIdx = ExportTexture(mtl->_emissiveMapFile);
			gltfMaterial.values["emissiveTexture"].json_double_value["index"] = texIdx;
		}
		if (mtl->Emissive.dot(Float3(0, 0, 0)) > 0.01f) {
			gltfMaterial.emissiveFactor.resize(3);
			gltfMaterial.emissiveFactor[0] = (double)mtl->Emissive.x;
			gltfMaterial.emissiveFactor[1] = (double)mtl->Emissive.y;
			gltfMaterial.emissiveFactor[2] = (double)mtl->Emissive.z;
		}

		// alpha mode
		if (mtl->alphaCutoff > 0) {
			gltfMaterial.alphaMode = "MASK";
		}
		gltfMaterial.alphaCutoff = mtl->alphaCutoff;

		// double sided
		//gltfMaterial.doubleSided = true;

		model.materials.push_back(gltfMaterial);
		return (int)model.materials.size() - 1;
	}

	int GLTFModelExp::ExportCamera(Camera* camera)
	{
		tinygltf::Camera gltfCamera;
		gltfCamera.name = camera->Name.c_str();
		gltfCamera.type = "perspective";
		gltfCamera.perspective.aspectRatio = 1.0f;
		gltfCamera.perspective.yfov = camera->fov;
		gltfCamera.perspective.znear = camera->zn;
		gltfCamera.perspective.zfar = camera->zf;
		model.cameras.push_back(gltfCamera);

		tinygltf::Node node;
		node.name = gltfCamera.name;
		for (int i = 0; i < 4; ++i) {
			node.matrix[i * 4 + 0] = camera->transform[i][0];
			node.matrix[i * 4 + 1] = camera->transform[i][1];
			node.matrix[i * 4 + 2] = camera->transform[i][2];
			node.matrix[i * 4 + 3] = camera->transform[i][3];
		}
		node.camera = (int)model.cameras.size() - 1;
		model.nodes.push_back(node);
		return (int)model.nodes.size() - 1;
	}

	int GLTFModelExp::ExportLight(Light* light, int index)
	{
		tinygltf::Light gltfLight;
		gltfLight.name = "light" + std::to_string(index);
		gltfLight.color = {
			(double)light->Color.x,
			(double)light->Color.y,
			(double)light->Color.z
		};
		gltfLight.intensity = 1.0;

		if (light->Type == Light::DIRECTION) {
			gltfLight.type = "DIRECTIONAL";
			gltfLight.spot.extensions["direction"] = tinygltf::Value(
				//std::vector<double>{light->Direction.x, light->Direction.y, light->Direction.z}
			);
		}
		else if (light->Type == Light::POINT) {
			gltfLight.type = "POINT";
			gltfLight.range = light->Range;
			gltfLight.spot.extensions["size"] = tinygltf::Value(light->Size);
		}
		else if (light->Type == Light::SPOT) {
			gltfLight.type = "SPOT";
			gltfLight.range = light->Range;
			gltfLight.spot.outerConeAngle = light->SpotOuter;
			gltfLight.spot.innerConeAngle = light->SpotInner;
			gltfLight.spot.extensions["size"] = tinygltf::Value(light->Size);
		}

		gltfLight.extensions["gi"] = tinygltf::Value(light->GIEnable);
		gltfLight.extensions["shadowMask"] = tinygltf::Value(light->ShadowMask);
		gltfLight.extensions["castShadow"] = tinygltf::Value(light->CastShadow);
		gltfLight.extensions["saveShadowMask"] = tinygltf::Value(light->SaveShadowMask);

		model.lights.push_back(gltfLight);

		tinygltf::Node node;
		node.name = gltfLight.name;
		tinygltf::Value::Object lightValue;
		lightValue.insert(std::make_pair("light", (int)model.lights.size() - 1));
		node.extensions["KHR_lights_punctual"] = tinygltf::Value(lightValue);
		model.nodes.push_back(node);
		return (int)model.nodes.size() - 1;
	}

	int GLTFModelExp::ExportMesh(Mesh* mesh, int index)
	{
		// vertex format: position normal uv [uv2]
		int vertexSize = sizeof(Float3) + sizeof(Float3) + sizeof(Float2);
		if (mesh->GetLightingMapSize() > 0) {
			vertexSize += sizeof(Float2);
		}
		
		const bool useLightmap = (mesh->GetLightingMapSize() > 0);
		const int bufferStartIndex = (int)model.buffers.size();
		const int bufferViewStartIndex = (int)model.bufferViews.size();
		const int vertexBufferAccessorStartIndex = (int)model.accessors.size();

		// export vertex buffer
		tinygltf::Buffer vertexBuf;
		vertexBuf.data.resize(mesh->NumOfVertices() * vertexSize);
		{
			uint8* vertexData = vertexBuf.data.data();
			for (const auto& v : mesh->_getVertexBuffer()) {
				// position
				memcpy(vertexData, &v.Position, sizeof(v.Position));
				vertexData += sizeof(v.Position);

				// normal
				memcpy(vertexData, &v.Normal, sizeof(v.Normal));
				vertexData += sizeof(v.Normal);
				
				// uv1
				memcpy(vertexData, &v.UV, sizeof(v.UV));
				vertexData += sizeof(v.UV);

				// uv2
				if (useLightmap) {
					memcpy(vertexData, &v.LUV, sizeof(v.LUV));
					vertexData += sizeof(v.LUV);
				}
			}

			tinygltf::BufferView vertexBufView;
			vertexBufView.buffer = bufferStartIndex;
			vertexBufView.byteOffset = 0;
			vertexBufView.byteLength = mesh->NumOfVertices() * vertexSize;
			vertexBufView.byteStride = vertexSize;
			model.bufferViews.push_back(vertexBufView);

			tinygltf::Accessor positionAccessor;
			positionAccessor.bufferView = bufferViewStartIndex;
			positionAccessor.byteOffset = 0;
			positionAccessor.count = mesh->NumOfVertices();
			positionAccessor.type = TINYGLTF_TYPE_VEC3;
			positionAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
			model.accessors.push_back(positionAccessor);

			tinygltf::Accessor normalAccessor;
			normalAccessor.bufferView = bufferViewStartIndex;
			normalAccessor.byteOffset = positionAccessor.byteOffset + sizeof(Float3);
			normalAccessor.count = mesh->NumOfVertices();
			normalAccessor.type = TINYGLTF_TYPE_VEC3;
			normalAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
			model.accessors.push_back(normalAccessor);

			tinygltf::Accessor uvAccessor;
			uvAccessor.bufferView = bufferViewStartIndex;
			uvAccessor.byteOffset = normalAccessor.byteOffset + sizeof(Float3);
			uvAccessor.count = mesh->NumOfVertices();
			uvAccessor.type = TINYGLTF_TYPE_VEC2;
			uvAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
			model.accessors.push_back(normalAccessor);

			if (useLightmap) {
				tinygltf::Accessor uvAccessor1;
				uvAccessor1.bufferView = bufferViewStartIndex;
				uvAccessor1.byteOffset = uvAccessor.byteOffset + sizeof(Float2);
				uvAccessor1.count = mesh->NumOfVertices();
				uvAccessor1.type = TINYGLTF_TYPE_VEC2;
				uvAccessor1.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
				model.accessors.push_back(normalAccessor);
			}
		}
		model.buffers.push_back(vertexBuf);

		// export index buffer
		tinygltf::Buffer indexBuf;
		indexBuf.data.resize(mesh->NumOfTriangles() * 3 * sizeof(int));
		{
			int* indexData = (int*)indexBuf.data.data();
			for (const auto& i : mesh->_getTriangleBuffer()) {
				*indexData++ = i.Index0;
				*indexData++ = i.Index1;
				*indexData++ = i.Index2;
			}

			tinygltf::BufferView indexBufView;
			indexBufView.buffer = bufferStartIndex + 1;
			indexBufView.byteOffset = 0;
			indexBufView.byteLength = mesh->NumOfTriangles() * 3 * sizeof(int);
			indexBufView.byteStride = sizeof(int);
			model.bufferViews.push_back(indexBufView);
		}
		model.buffers.push_back(indexBuf);

		int lastMaterial = -1;
		int startIndex = 0;
		tinygltf::Mesh gltfMesh;
		gltfMesh.name = "mesh" + std::to_string(index);
		for (int i = 0; i < mesh->NumOfTriangles(); ++i) {
			const auto& triangle = mesh->_getTriangle(i);
			if (i == 0) {
				lastMaterial = triangle.MaterialId;
			}

			if (lastMaterial != triangle.MaterialId || i == mesh->NumOfTriangles() - 1) {
				tinygltf::Primitive gltfPrim;
				gltfPrim.mode = TINYGLTF_MODE_TRIANGLES;
				gltfPrim.attributes.insert(std::make_pair("POSITION", vertexBufferAccessorStartIndex + 0));
				gltfPrim.attributes.insert(std::make_pair("NORMAL", vertexBufferAccessorStartIndex + 1));
				gltfPrim.attributes.insert(std::make_pair("TEXCOORD_0", vertexBufferAccessorStartIndex + 2));
				if (useLightmap) {
					gltfPrim.attributes.insert(std::make_pair("TEXCOORD_1", vertexBufferAccessorStartIndex + 3));
				}

				tinygltf::Accessor accessor;
				accessor.bufferView = bufferViewStartIndex + 1;
				accessor.byteOffset = startIndex * sizeof(int);
				accessor.count = i - startIndex;
				accessor.type = TINYGLTF_TYPE_SCALAR;
				accessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
				model.accessors.push_back(accessor);

				gltfPrim.indices = (int)model.accessors.size() - 1;
				
				gltfPrim.material = ExportMaterial(&mesh->_getMaterial(lastMaterial));

				gltfMesh.primitives.push_back(gltfPrim);
				startIndex = i;
				lastMaterial = triangle.MaterialId;
			}
		}
		model.meshes.push_back(gltfMesh);

		tinygltf::Node node;
		node.name = gltfMesh.name;
		node.mesh = (int)model.meshes.size() - 1;
		node.extensions["lightMapSize"] = tinygltf::Value(mesh->GetLightingMapSize());
		node.extensions["castShadow"] = tinygltf::Value(mesh->GetCastShadow());
		node.extensions["recieveShadow"] = tinygltf::Value(mesh->GetRecieveShadow());
		model.nodes.push_back(node);
		return (int)model.nodes.size() - 1;
	}

	bool GLTFExp::Export()
	{
		GLTFModelExp exp;
		for (auto* camera : LFX::World::Instance()->GetCameras()) {
			exp.ExportCamera(camera);
		}

		int index = 0;
		for (auto* mesh : LFX::World::Instance()->GetMeshes()) {
			exp.ExportMesh(mesh, index++);
		}

		index = 0;
		for (auto* mesh : LFX::World::Instance()->GetLights()) {
			exp.ExportLight(mesh, index++);
		}

		return exp.WriteToFile("lfx.gltf", false);
	}

}