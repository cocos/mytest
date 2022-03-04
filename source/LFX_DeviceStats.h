#include "LFX_Types.h"

namespace LFX {

	/**
	* @zh 设备信息
	*/
	struct LFX_ENTRY DeviceStats
	{
		int Processors; // 处理器个数

		DeviceStats()
			: Processors(0)
		{
		}

		static DeviceStats GetStats();
	};

}