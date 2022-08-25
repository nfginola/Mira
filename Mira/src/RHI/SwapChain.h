#pragma once
#include "RHITypes.h"

namespace mira
{
	class SwapChain
	{
	public:
		virtual Texture get_next_draw_surface() = 0;
		virtual u8 get_next_draw_surface_idx() = 0;
		virtual void present(bool vsync) = 0;
		virtual void set_clear_color(const std::array<float, 4>& clear_color) = 0;

		virtual Texture get_buffer(u8 idx) = 0;

		virtual ~SwapChain() {};
	};
}
