#include "LFX_Scene.h"
#include "LFX_World.h"

namespace LFX {

	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
	}

	bool BVT_AddMesh(BSPTree<Mesh*>::Node* node, Mesh* mesh)
	{
		Aabb bound = mesh->GetBound();

		if (node->aabb.Contain(bound))
		{
			if (node->child[0] != NULL && BVT_AddMesh(node->child[0], mesh))
				return true;

			if (node->child[1] != NULL && BVT_AddMesh(node->child[1], mesh))
				return true;

			node->elems.push_back(mesh);

			return true;
		}

		return false;
	}

	Aabb BVT_Optimize(BSPTree<Mesh*>::Node* node)
	{
		if (node->child[0] == NULL)
		{
			if (node->elems.size() > 0)
			{
				Aabb bound = node->elems[0]->GetBound();
				for (size_t i = 1; i < node->elems.size(); ++i)
				{
					bound.Merge(node->elems[i]->GetBound());
				}

				node->aabb = bound;
			}
		}
		else
		{
			Aabb bound = BVT_Optimize(node->child[0]);
			bound.Merge(BVT_Optimize(node->child[1]));

			node->aabb = bound;
		}

		return node->aabb;
	}

	void Scene::Build()
	{
		Aabb worldBound;

		worldBound.Invalid();
		for (auto mesh : World::Instance()->Meshes())
		{
			Aabb bound = mesh->GetBound();
			worldBound.Merge(bound);
		}

		mBVTree.Clear();
		if (worldBound.Valid())
		{
			mBVTree.Build(worldBound, LFX_BVH_LEVELS);
			for (auto mesh : World::Instance()->Meshes())
			{
				BVT_AddMesh(mBVTree.RootNode(), mesh);
			}

			BVT_Optimize(mBVTree.RootNode());
		}
	}

	bool Scene::RayCheck(Contact& contact, const Ray& ray, float len, int flags)
	{
		return _RayCheckImp(contact, ray, len, flags);
	}

	bool _occluded(BSPTree<Mesh*>::Node* node, const Ray& ray, float len)
	{
		float dist = 0;

		if (!Intersect(ray, &dist, node->aabb))
			return false;

		for (size_t i = 0; i < node->elems.size(); ++i)
		{
			if (node->elems[i]->Occluded(ray, len))
				return true;
		}

		if (node->child[0] != NULL)
		{
			if (_occluded(node->child[0], ray, len))
				return true;
		}

		if (node->child[1] != NULL)
		{
			if (_occluded(node->child[1], ray, len))
				return true;
		}

		return false;
	}

	bool _occluded(const std::vector<Terrain*>& terrains, const Ray& ray, float len)
	{
		for (auto i : terrains) {
			if (i->Occluded(ray, len)) {
				return true;
			}
		}

		return false;
	}

	bool Scene::_OccludedImp(const Ray& ray, float len, int flags)
	{
		if ((flags & LFX_MESH) && mBVTree.RootNode() != NULL && _occluded(mBVTree.RootNode(), ray, len))
			return true;
		if ((flags & LFX_TERRAIN) && _occluded(World::Instance()->Terrains(), ray, len))
			return true;

		return false;
	}

	bool Scene::Occluded(const Ray& ray, float len, int flags)
	{
		return _OccludedImp(ray, len, flags);
	}

	void _rayCheck(Contact& contract, BSPTree<Mesh*>::Node* node, const Ray& ray, float len)
	{
		float dist = 0;

		if (!Intersect(ray, &dist, node->aabb) || contract.td < dist)
			return;

		for (size_t i = 0; i < node->elems.size(); ++i)
		{
			node->elems[i]->RayCheck(contract, ray, len);
		}

		if (node->child[0] != NULL)
		{
			_rayCheck(contract, node->child[0], ray, len);
		}

		if (node->child[1] != NULL)
		{
			_rayCheck(contract, node->child[1], ray, len);
		}
	}

	void _rayCheck(Contact& contact, const std::vector<Terrain*>& terrains, const Ray& ray, float len)
	{
		for (auto i : terrains) {
			i->RayCheck(contact, ray, len);
		}
	}

	bool Scene::_RayCheckImp(Contact& contact, const Ray& ray, float len, int flags)
	{
		contact.td = FLT_MAX;
		contact.tu = 0;
		contact.tv = 0;
		contact.triIndex = -1;
		contact.entity = NULL;
		contact.backFacing = false;

		if ((flags & LFX_MESH) && mBVTree.RootNode() != NULL) {
			_rayCheck(contact, mBVTree.RootNode(), ray, len);
		}
		if ((flags & LFX_TERRAIN) && World::Instance()->Terrains().size() > 0) {
			_rayCheck(contact, World::Instance()->Terrains(), ray, len);
		}

		if (contact.entity != NULL)
		{
			Vertex a, b, c;
			if (contact.entity->GetType() == LFX_TERRAIN)
			{
				Terrain* terrain = (Terrain*)contact.entity;

				Triangle tri = terrain->_getTriangle(contact.triIndex);
				a = terrain->_getVertex(tri.Index0);
				b = terrain->_getVertex(tri.Index1);
				c = terrain->_getVertex(tri.Index2);
			}
			else
			{
				Mesh * mesh = (Mesh *)contact.entity;

				Triangle tri = mesh->_getTriangle(contact.triIndex);
				a = mesh->_getVertex(tri.Index0);
				b = mesh->_getVertex(tri.Index1);
				c = mesh->_getVertex(tri.Index2);
			}

			Float3 triNml = Float3::Normalize(Float3::Cross(c.Position - a.Position, b.Position - a.Position));
			contact.backFacing = Float3::Dot(triNml, ray.dir) >= 0.0f;
		}

		return contact.entity != NULL;
	}
}