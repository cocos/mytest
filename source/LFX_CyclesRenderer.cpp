#include "LFX_CyclesRenderer.h"
#include "LFX_World.h"

#ifdef LFX_CYLCES_RENDERER
#define CC_DLL
#include "tinyxml2.h"

namespace LFX {

	namespace
	{
		template <class T>
		std::string ToString(const T& v)
		{
			return std::to_string(v);
		}

		template <>
		std::string ToString<Float2>(const Float2& v)
		{
			return
				std::to_string(v.x) + " " +
				std::to_string(v.y);
		}

		template <>
		std::string ToString<Float3>(const Float3& v)
		{
			return
				std::to_string(v.x) + " " +
				std::to_string(v.y) + " " +
				std::to_string(v.z);
		}

		template <>
		std::string ToString<Float4>(const Float4& v)
		{
			return
				std::to_string(v.x) + " " +
				std::to_string(v.y) + " " +
				std::to_string(v.z) + " " +
				std::to_string(v.w);
		}

		tinyxml2::XMLNode* CreateNode(tinyxml2::XMLNode* node, const String& name)
		{
			auto* child = node->GetDocument()->NewElement(name.c_str());
			node->InsertEndChild(child);
			return child;
		}

		void SetAttribute(tinyxml2::XMLNode* node, const String& name, const String& value)
		{
			tinyxml2::XMLElement* elem = node ? node->ToElement() : nullptr;
			if (elem != nullptr) {
				elem->SetAttribute(name.c_str(), value.c_str());
			}
		}

		tinyxml2::XMLNode* WriteTransform(tinyxml2::XMLNode* dom, const Float3& pos)
		{
			auto* node = CreateNode(dom, "transform");
			SetAttribute(node, "translate", ToString(pos));
			return node;
		}

		tinyxml2::XMLNode* WriteIntergrator(tinyxml2::XMLNode* dom)
		{
			auto* node = CreateNode(dom, "integrator");

			struct CyclesIntegrator
			{
				int min_bounce = 0;
				int max_bounce = 4;

				int max_diffuse_bounce = 4;
				int max_glossy_bounce = 4;
				int max_transmission_bounce = 4;
				int max_volume_bounce = 4;

				int ao_bounces = 4;
				float ao_factor = 1.0f;
				float ao_distance = FLT_MAX;
				float ao_additive_factor = 0.0f;

				bool denoise = false;
			};
			CyclesIntegrator integrator;

			SetAttribute(node, "name", "LightFX");
			SetAttribute(node, "min_bounce", std::to_string(integrator.min_bounce));
			SetAttribute(node, "max_bounce", std::to_string(integrator.max_bounce));
			SetAttribute(node, "max_diffuse_bounce", std::to_string(integrator.max_diffuse_bounce));
			SetAttribute(node, "max_glossy_bounce", std::to_string(integrator.max_glossy_bounce));
			SetAttribute(node, "max_transmission_bounce", std::to_string(integrator.max_transmission_bounce));
			SetAttribute(node, "max_volume_bounce", std::to_string(integrator.max_volume_bounce));
			SetAttribute(node, "ao_bounces", std::to_string(integrator.ao_bounces));
			SetAttribute(node, "ao_factor", std::to_string(integrator.ao_factor));
			SetAttribute(node, "ao_distance", std::to_string(integrator.ao_distance));
			SetAttribute(node, "ao_additive_factor", std::to_string(integrator.ao_additive_factor));

			return node;
		}

		tinyxml2::XMLNode* WriteMesh(tinyxml2::XMLNode* dom, Mesh* mesh, int index)
		{
			tinyxml2::XMLNode* tm = WriteTransform(dom, Float3(0, 0, 0));

			auto* node = CreateNode(tm, "mesh");

			SetAttribute(node, "name", "mesh" + std::to_string(index));

			String P, UV;
			for (int i = 0; i < mesh->NumOfVertices(); ++i) {
				const auto& v = mesh->_getVertex(i);
				P += std::to_string(v.Position.x) + " " +
					std::to_string(v.Position.y) + " " +
					std::to_string(v.Position.z);
				UV += std::to_string(v.UV.x) + " " +
					std::to_string(v.UV.y);
				break;
			}
			for (int i = 1; i < mesh->NumOfVertices(); ++i) {
				const auto& v = mesh->_getVertex(i);
				P += " " + std::to_string(v.Position.x) +
					" " + std::to_string(v.Position.y) +
					" " + std::to_string(v.Position.z);
				UV += " " + std::to_string(v.UV.x) +
					" " + std::to_string(v.UV.y);
			}

			SetAttribute(node, "P", P);
			SetAttribute(node, "UV", UV);

			String verts, nverts;
			for (int i = 0; i < mesh->NumOfTriangles(); ++i) {
				const auto& t = mesh->_getTriangle(i);
				verts += std::to_string(t.Index0) + " " +
					std::to_string(t.Index1) + " " +
					std::to_string(t.Index2);
				nverts += "3";
				break;
			}
			for (int i = 1; i < mesh->NumOfTriangles(); ++i) {
				const auto& t = mesh->_getTriangle(i);
				verts += " " + std::to_string(t.Index0) +
					" " + std::to_string(t.Index1) +
					" " + std::to_string(t.Index2);
				nverts += " 3";
				break;
			}

			SetAttribute(node, "verts", verts);
			SetAttribute(node, "nverts", nverts);

			return node;
		}

		void WriteLight(tinyxml2::XMLNode* dom, Light* light, int index)
		{
			struct CyclesLight
			{
				enum Type
				{
					LIGHT_POINT,
					LIGHT_DISTANT,
					LIGHT_BACKGROUND,
					LIGHT_AREA,
					LIGHT_SPOT,
					LIGHT_TRIANGLE
				};

				Type light_type;
				Float3 strength;
				Float3 co; // color

				Float3 dir;
				float size;
				float angle;

				// area light
				Float3 axisu;
				float sizeu;
				float axisv;
				float sizev;
				bool round;
				float spread;

				// spot light
				float spot_angle;
				float spot_smooth;

				//
				bool cast_shadow;
			};

			dom = WriteTransform(dom, light->Position);

			auto* node = CreateNode(dom, "light");

			SetAttribute(node, "name", "light" + std::to_string(index));

			if (light->Type == Light::POINT) {
				SetAttribute(node, "light_type", "LIGHT_POINT");
			}
			else if (light->Type == Light::SPOT) {
				SetAttribute(node, "light_type", "LIGHT_SPOT");
			}
			else if (light->Type == Light::DIRECTION) {
				// ???
				SetAttribute(node, "light_type", "LIGHT_DISTANT");
			}

			SetAttribute(node, "co", ToString(light->Color));
			SetAttribute(node, "dir", ToString(light->Direction));

			if (light->Type == Light::POINT || light->Type == Light::SPOT) {
				SetAttribute(node, "size", ToString(light->Size));
				// ???
				//SetAttribute(node, "angle", ToString(light->Range));
			}
			if (light->Type == Light::SPOT) {
				SetAttribute(node, "spot_angle", ToString(std::acosf(light->SpotOuter)));
				// ???
				//SetAttribute(node, "spot_smooth", ToString(light->SpotFallOff));
			}

			SetAttribute(node, "cast_shadow", light->CastShadow ? "true" : "false");
		}
	}

	CylcesRenderer::CylcesRenderer()
	{
	}

	void CylcesRenderer::ExportScene()
	{
		int index = 0;
		tinyxml2::XMLDocument xdoc;
		tinyxml2::XMLNode* dom = CreateNode(&xdoc, "cycles");

		WriteIntergrator(dom);

		index = 0;
		for (auto* light : World::Instance()->GetLights()) {
			WriteLight(dom, light, ++index);
		}

		index = 0;
		for (auto* mesh : World::Instance()->GetMeshes()) {
			WriteMesh(dom, mesh, ++index);
		}

		xdoc.SaveFile("cycles.xml");
	}

}
#endif