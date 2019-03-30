#include "LFX_Entity.h"

namespace LFX {

	Entity::Entity()
	{
		for (int i = 0; i < 8; ++i)
		{
			mUserData[i] = 0;
		}
	}

	Entity::~Entity()
	{
	}

	void Entity::SetUserData(void * data, int i)
	{
		assert(i < 8);
		mUserData[i] = data;
	}

	void * Entity::GetUserData(int i)
	{
		return mUserData[i];
	}

}
