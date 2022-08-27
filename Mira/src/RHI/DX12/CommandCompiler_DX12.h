#pragma once
#include "../RenderCommandList.h"
#include "DX12CommonIncludes.h"

namespace mira
{
	class RenderDevice_DX12;

	class CommandCompiler_DX12
	{
	public:
		CommandCompiler_DX12(const RenderDevice_DX12* dev, ComPtr<ID3D12CommandAllocator> ator, ComPtr<ID3D12GraphicsCommandList4> cmdl, QueueType queue);

		ID3D12GraphicsCommandList4* get_list() { return m_list.Get(); }
		ID3D12CommandAllocator* get_allocator() { return m_ator.Get(); }
		QueueType get_queue_type() const { return m_queue_type; }
		
		void compile(const RenderCommandDraw& cmd);
		void compile(const RenderCommandDrawIndexed& cmd);
		void compile(const RenderCommandSetPipeline& cmd);
		void compile(const RenderCommandBeginRenderPass& cmd);
		void compile(const RenderCommandEndRenderPass& cmd);
		void compile(const RenderCommandBarrier& cmd);
		void compile(const RenderCommandCopyBuffer& cmd);
		void compile(const RenderCommandUpdateShaderArgs& cmd);

	private:
		const RenderDevice_DX12* m_dev;
		ComPtr<ID3D12CommandAllocator> m_ator;
		ComPtr<ID3D12GraphicsCommandList4> m_list;
		QueueType m_queue_type{ QueueType::None };

		// Store barriers persistently
		std::vector<std::vector<D3D12_RESOURCE_BARRIER>> m_barriers_per_submission;

		Buffer m_current_ib;


	};
}
