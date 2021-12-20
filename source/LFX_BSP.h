#pragma once

#include "LFX_Math.h"

namespace LFX {

	template <class T>
	class BSPTree
	{
	public:
		enum eAxis {
			AXIS_N = 0,
			AXIS_X = 1,
			AXIS_Y = 2,
			AXIS_Z = 4,

			AXIS_ALL = AXIS_X | AXIS_Y | AXIS_Z
		};

		struct Node
		{
			int axis;
			Aabb aabb;
			std::vector<T> elems;

			Node * child[2];

			~Node()
			{
				if (child[0])
					delete child[0];
				if (child[1])
					delete child[1];
			}
		};

	public:
		BSPTree() : mRoot(NULL)
		{
		}

		~BSPTree()
		{
			Clear();
		}

		void Clear()
		{
			if (mRoot != NULL)
				delete mRoot;
			mRoot = NULL;
		}

		void Build(const Aabb & aabb, int depth, int axises = AXIS_ALL)
		{
			Clear();

			mRoot = _allocNode();
			mRoot->aabb = aabb;
			mRoot->axis = AXIS_N;
			mRoot->child[0] = NULL;
			mRoot->child[1] = NULL;

			_build(mRoot, depth, axises);
		}

		Node * RootNode()
		{
			return mRoot;
		}

	protected:
		void _build(Node * node, int depth, int axises)
		{
			if (depth < 1)
				return;

			const Aabb & aabb = node->aabb;
			Float3 size = aabb.Size();
			Float3 split1 = Float3(1, 1, 1);
			Float3 split2 = Float3(0, 0, 0);

			switch (axises)
			{
			case AXIS_X | AXIS_Y | AXIS_Z:
				if (size.x >= size.y && size.x >= size.z)
				{
					node->axis = AXIS_X;
					split1.x = 0.5f;
					split2.x = 0.5f;
				}
				else if (size.y >= size.x && size.y >= size.z)
				{
					node->axis = AXIS_Y;
					split1.y = 0.5f;
					split2.y = 0.5f;
				}
				else
				{
					node->axis = AXIS_Z;
					split1.z = 0.5f;
					split2.z = 0.5f;
				}
				break;

			case AXIS_X | AXIS_Z:
				if (size.x >= size.z)
				{
					node->axis = AXIS_X;
					split1.x = 0.5f;
					split2.x = 0.5f;
				}
				else
				{
					node->axis = AXIS_Z;
					split1.z = 0.5f;
					split2.z = 0.5f;
				}
				break;

			case AXIS_X | AXIS_Y:
				if (size.x >= size.y)
				{
					node->axis = AXIS_X;
					split1.x = 0.5f;
					split2.x = 0.5f;
				}
				else
				{
					node->axis = AXIS_Y;
					split1.y = 0.5f;
					split2.y = 0.5f;
				}
				break;

			case AXIS_X:
				node->axis = AXIS_X;
				split1.x = 0.5f;
				split2.x = 0.5f;
				break;

			case AXIS_Y:
				node->axis = AXIS_Y;
				split1.y = 0.5f;
				split2.y = 0.5f;
				break;

			case AXIS_Z:
				node->axis = AXIS_Z;
				split1.z = 0.5f;
				split2.z = 0.5f;
				break;
			}

			node->child[0] = _allocNode();
			node->child[1] = _allocNode();

			node->child[0]->aabb = Aabb(aabb.minimum, aabb.minimum + split1 * size);
			node->child[1]->aabb = Aabb(aabb.minimum + split2 * size, aabb.maximum);

			_build(node->child[0], depth - 1, axises);
			_build(node->child[1], depth - 1, axises);
		}

	protected:
		Node * _allocNode()
		{
			Node * p = new Node;
			p->aabb.minimum = Float3(0, 0, 0);
			p->aabb.maximum = Float3(0, 0, 0);
			p->axis = AXIS_N;
			p->child[0] = NULL;
			p->child[1] = NULL;

			return p;
		}

	protected:
		Node * mRoot;
	};

}