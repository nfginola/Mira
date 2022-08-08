#pragma once
#include "../RenderBackend.h"
#include "DX12CommonIncludes.h"

namespace mira
{
	class RenderBackend_DX12 : public RenderBackend
	{
	public:
		RenderBackend_DX12(bool debug = false);

		RenderDevice* create_device() override;

	private:
		void create_adapter_factory();
		void select_adapter();
		void check_feature_support();

		struct FinalDebug
		{
			~FinalDebug();
		};

	private:
		bool m_debug_on{ false };

		std::unordered_map<RenderDevice*, ComPtr<ID3D12Device>> m_devices;
		FinalDebug m_debug_print;
		std::unordered_map<RenderDevice*, std::unique_ptr<RenderDevice>> m_render_devices;
		
		ComPtr<IDXGIFactory6> m_dxgi_fac;
		ComPtr<IDXGIAdapter> m_adapter;
		DXGI_ADAPTER_DESC m_adapter_desc;

	};
}

