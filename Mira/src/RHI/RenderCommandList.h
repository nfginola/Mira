#pragma once
#include "RHITypes.h"
#include <span>

namespace mira
{
	class RenderCommandList
	{
	public:
		virtual void set_pipeline(Pipeline pipe) = 0;
		virtual void set_index_buffer(Buffer buffer) = 0;
		virtual void draw(u32 verts_per_instance, u32 instance_count, u32 vert_start, u32 instance_start) = 0;
		virtual void update_shader_args(u8 num_descriptors, u32* descriptors, QueueType queue) = 0;

		virtual void begin_renderpass(RenderPass rp) = 0;
		virtual void end_renderpass() = 0;

		virtual void submit_barriers(std::span<ResourceBarrier> barriers) = 0;

		virtual ~RenderCommandList() {}

	};

}
