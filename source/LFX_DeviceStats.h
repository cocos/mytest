#include "LFX_Types.h"

namespace LFX {

	/**
	* @zh �豸��Ϣ
	*/
	struct LFX_ENTRY DeviceStats
	{
		int Processors; // ����������

		DeviceStats()
			: Processors(0)
		{
		}

		static DeviceStats GetStats();
	};

}