#pragma once
#include "../SwapChain.h"
#include "RenderDevice_DX12.h"

#include <windows.h>

namespace mira
{
	class SwapChain_DX12 : public SwapChain
	{
	public:
		SwapChain_DX12(RenderDevice_DX12* device, HWND hwnd, const std::vector<Texture>& handles_to_attach_to, bool debug_on);
		~SwapChain_DX12();

		// Public interface
		Texture get_next_draw_surface();
		void present(bool vsync);

	private:
		bool is_tearing_supported();

	private:
		RenderDevice_DX12* m_device{ nullptr };
		ComPtr<IDXGISwapChain3> m_sc;

		std::vector<Texture> m_buffers;

		bool m_tearing_is_supported{ false };
	};
}


