#pragma once
#include "../RHI/RenderResourceHandle.h"

namespace mira
{
	class GPUGarbageBin;

	class Renderer
	{
	public:		


	private:
		std::unique_ptr<GPUGarbageBin> m_gpu_bin;
	};
}


