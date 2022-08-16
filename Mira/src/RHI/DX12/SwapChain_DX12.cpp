#include "SwapChain_DX12.h"

namespace mira
{
	SwapChain_DX12::SwapChain_DX12(RenderDevice_DX12* device, HWND hwnd, std::span<Texture> handles_to_attach_to, bool debug_on) :
		m_device(device)
	{
		assert(handles_to_attach_to.size() >= 2);

		HRESULT hr{ S_OK };

		// Swapchain may force a flush on the associated queue (requires direct queue, to be specific)
		// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12HelloWorld/src/HelloTriangle/D3D12HelloTriangle.cpp#L95
		// https://docs.microsoft.com/en-us/windows/win32/api/dxgi1_2/nf-dxgi1_2-idxgifactory2-createswapchainforhwnd

		DXGI_SWAP_CHAIN_DESC1 scd{};
		scd.Width = 0;
		scd.Height = 0;
		scd.BufferCount = (u32)handles_to_attach_to.size();
		scd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;				// Requires manual gamma correction before presenting
		scd.SampleDesc.Count = 1;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		scd.Flags = m_tearing_is_supported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		// Grab IDXGIFactory4 for CreateSwapChainForHwnd
		ComPtr<IDXGIFactory2> fac;
		UINT factoryFlags = debug_on ? DXGI_CREATE_FACTORY_DEBUG : 0;
		hr = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(fac.GetAddressOf()));
		HR_VFY(hr);
		ComPtr<IDXGIFactory4> fac4;
		hr = fac.As(&fac4);
		HR_VFY(hr);

		ComPtr<IDXGISwapChain1> sc;
		hr = fac->CreateSwapChainForHwnd(
			m_device->get_queue(D3D12_COMMAND_LIST_TYPE_DIRECT),
			hwnd,
			&scd,
			nullptr,			// no fullscreen
			nullptr,			// no output restrictions
			sc.GetAddressOf());
		HR_VFY(hr);

		// Grab SwapChain4 to get access to GetBackBufferIndex
		hr = sc.As(&m_sc);
		HR_VFY(hr);

		// Disable alt-enter for now
		hr = fac->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
		HR_VFY(hr);

		for (uint32_t i = 0; i < handles_to_attach_to.size(); ++i)
		{
			ComPtr<ID3D12Resource> buffer;
			hr = m_sc->GetBuffer(i, IID_PPV_ARGS(buffer.GetAddressOf()));
			HR_VFY(hr);
			m_device->register_swapchain_texture(buffer, handles_to_attach_to[i]);		// register textures
			m_buffers.push_back(handles_to_attach_to[i]);
		}
	}
	
	SwapChain_DX12::~SwapChain_DX12()
	{
		for (auto bb : m_buffers)
			m_device->free_texture(bb);
	}
	Texture SwapChain_DX12::get_next_draw_surface()
	{
		u32 index = m_sc->GetCurrentBackBufferIndex();
		return m_buffers[index];
	}
	u8 SwapChain_DX12::get_next_draw_surface_idx()
	{
		return (u8)m_sc->GetCurrentBackBufferIndex();
	}
	void SwapChain_DX12::set_clear_color(const std::array<float, 4>& clear_color)
	{
		for (const auto& bb : m_buffers)
			m_device->set_clear_color(bb, clear_color);
	}
	void SwapChain_DX12::present(bool vsync)
	{
		u32 flags{ 0 };
		if (!vsync && m_tearing_is_supported)
			flags |= DXGI_PRESENT_ALLOW_TEARING;

		HRESULT hr{ S_OK };
		hr = m_sc->Present(vsync, flags);
		HR_VFY(hr);
	}
	bool SwapChain_DX12::is_tearing_supported()
	{
		BOOL allowed = FALSE;
		ComPtr<IDXGIFactory4> fac4;
		if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&fac4))))
		{
			ComPtr<IDXGIFactory5> fac5;
			if (SUCCEEDED(fac4.As(&fac5)))
			{
				auto hr = fac5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowed, sizeof(allowed));
				allowed = SUCCEEDED(hr) && allowed;
			}
		}
		return allowed;
	}
}

