#pragma once
#include "RHITypes.h"

namespace mira
{
	class OldCommandList
	{
	public:
		virtual void set_pipeline(Pipeline pipe) = 0;
		virtual void set_index_buffer(Buffer buffer) = 0;
		virtual void draw(u32 verts_per_instance, u32 instance_count, u32 vert_start, u32 instance_start) = 0;
		virtual void update_shader_args(u8 num_descriptors, u32* descriptors, QueueType queue) = 0;

		virtual void begin_renderpass(RenderPass rp) = 0;
		virtual void end_renderpass() = 0;

		virtual void add_uav_barrier(Texture resource) = 0;
		virtual void add_uav_barrier(Buffer resource) = 0;
		virtual void add_aliasing_barrier(Texture before, Texture after) = 0;
		virtual void add_transition_barrier(Buffer resource, ResourceState before, ResourceState after) = 0;
		virtual void add_transition_barrier(Texture resource, u8 subresource, ResourceState before, ResourceState after) = 0;
		virtual void flush_barriers() = 0;

		virtual void submit_barriers(u8 num_barriers, ResourceBarrier* barriers) = 0;

		virtual ~OldCommandList() {}

	};

}
