#include "RenderBackend_DX12.h"
#include "RenderDevice_DX12.h"

// AgilitySDK
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 606; }

extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }

namespace mira
{
	RenderBackend_DX12::RenderBackend_DX12(bool debug) :
		m_debug_on(debug)
	{
		create_adapter_factory();
		select_adapter();
		check_feature_support();
	}

	RenderDevice* RenderBackend_DX12::create_device()
	{
		HRESULT hr{ S_OK };
		ComPtr<ID3D12Device5> dev;
		if (m_debug_on)
		{
			ComPtr<ID3D12Debug1> debug1;
			hr = D3D12GetDebugInterface(IID_PPV_ARGS(debug1.GetAddressOf()));
			HR_VFY(hr);

			debug1->SetEnableGPUBasedValidation(true);
			debug1->EnableDebugLayer();
		}

		hr = D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(dev.GetAddressOf()));
		HR_VFY(hr);

		auto render_device = std::make_unique<RenderDevice_DX12>(dev, m_adapter.Get(), m_debug_on);
		auto ret = render_device.get();

		m_devices.insert({ render_device.get(), dev });
		m_render_devices.insert({ render_device.get(), std::move(render_device)});
		
		return ret;
	}

	void RenderBackend_DX12::create_adapter_factory()
	{
		HRESULT hr{ S_OK };

		ComPtr<IDXGIFactory2> dxgi_fac2;
		hr = CreateDXGIFactory2(m_debug_on ? DXGI_CREATE_FACTORY_DEBUG : 0, IID_PPV_ARGS(dxgi_fac2.GetAddressOf()));
		HR_VFY(hr);

		hr = dxgi_fac2.As(&m_dxgi_fac);
		HR_VFY(hr);
	}

	void  RenderBackend_DX12::select_adapter()
	{
		HRESULT hr{ S_OK };

		// Pick first best adapter
		hr = m_dxgi_fac->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(m_adapter.GetAddressOf()));
		HR_VFY(hr);

		m_adapter->GetDesc(&m_adapter_desc);
		HR_VFY(hr);
	}

	void RenderBackend_DX12::check_feature_support()
	{
		HRESULT hr{ S_OK };
		ComPtr<ID3D12Device5> dev;

		hr = D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(dev.GetAddressOf()));
		HR_VFY(hr);

		// Check feature support..

	}

	RenderBackend_DX12::FinalDebug::~FinalDebug()
	{
		// Using DXGI debug device for reporting
		// If good, we should only have the Device reference left
		ComPtr<IDXGIDebug1> dxgi_debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgi_debug.GetAddressOf()))))
		{
			// https://docs.microsoft.com/en-us/windows/win32/direct3ddxgi/dxgi-debug-id
			// Need to link to dxguid.lib for DXGI_DEBUG_ALL
			auto hr = dxgi_debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_ALL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
			//auto hr = dxgi_debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));

			if (FAILED(hr))
				assert(false);
		}
	}

};
