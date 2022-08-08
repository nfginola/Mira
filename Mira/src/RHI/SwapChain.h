#pragma once
#include "RHITypes.h"

namespace mira
{
	class SwapChain
	{
	public:
		virtual Texture get_next_draw_surface() = 0;
		virtual void present(bool vsync) = 0;


	};
}
