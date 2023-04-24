#pragma once

#include "LFX_Renderer.h"

#ifdef LFX_CYLCES_RENDERER
namespace LFX {

	class CylcesRenderer
	{
	public:
		CylcesRenderer();

		void ExportScene();
	};

}
#endif