#pragma once
#include "RHITypes.h"
#include <span>

#include <iostream>

namespace mira
{
	enum class RenderCommandType
	{
		None,

		Draw,
		DrawIndexed,
		SetPipeline,
		BeginRenderPass,
		EndRenderPass,
		Barrier,
		UpdateShaderArgs,

		CopyBuffer,
		CopyBufferToImage
	};

	struct RenderCommand
	{
		RenderCommandType type{ RenderCommandType::None };		// Keep track of underlying command
	};


	// Helper to easily apply struct to enum matching.
	template <RenderCommandType CMD_TYPE>
	struct RenderCommandTyped : public RenderCommand
	{
		// Additionally, we store statically for ease of use
		/*
			Instead of:

				switch(cmd->type)
				{
					case RenderCommandType::Draw:
						auto cmd = static_cast<RenderCommandDraw*>(cmd);
				}

			Where the connection between the struct and enum are implicitly connected, we can make it explicit by
			statically storing the enum for each struct type.
		
				switch(cmd.type)
				{
					case RenderCommandDraw::TYPE:
						auto cmd = static_cast<RenderCommandDraw*>(cmd);
				}
		*/

		static const RenderCommandType TYPE = CMD_TYPE;		
		RenderCommandTyped() { type = TYPE; }
	};

	struct RenderCommandDraw : public RenderCommandTyped<RenderCommandType::Draw>
	{
		u32 verts_per_instance{ 0 };
		u32 instance_count{ 0 };
		u32 vert_start{ 0 };
		u32 instance_start{ 0 };

		RenderCommandDraw() = default;
		RenderCommandDraw(u32 verts_per_instance_in, u32 instance_count_in, u32 vert_start_in, u32 instance_start_in) :
			verts_per_instance(verts_per_instance_in),
			instance_count(instance_count_in),
			vert_start(vert_start_in),
			instance_start(instance_start_in) {}
	};

	struct RenderCommandDrawIndexed : public RenderCommandTyped<RenderCommandType::DrawIndexed>
	{
		Buffer index_buffer;

		u32 indices_per_instance{ 0 };
		u32 instance_count{ 0 };
		u32 index_start{ 0 };
		u32 vertex_start{ 0 };
		u32 instance_start{ 0 };

		RenderCommandDrawIndexed() = default;
		RenderCommandDrawIndexed(Buffer index_buffer_in, u32 indices_per_instance_in, u32 instance_count_in, u32 index_start_in, u32 vertex_start_in, u32 instance_start_in) :
			index_buffer(index_buffer_in),
			indices_per_instance(indices_per_instance_in),
			instance_count(instance_count_in),
			index_start(index_start_in),
			vertex_start(vertex_start_in),
			instance_start(instance_start_in) {}
	};

	struct RenderCommandSetPipeline : public RenderCommandTyped<RenderCommandType::SetPipeline>
	{
		Pipeline pipeline;

		RenderCommandSetPipeline() = default;
		RenderCommandSetPipeline(Pipeline pipeline_in) : pipeline(pipeline_in) {}
	};

	struct RenderCommandBeginRenderPass : public RenderCommandTyped<RenderCommandType::BeginRenderPass>
	{
		RenderPass rp;

		RenderCommandBeginRenderPass() = default;
		RenderCommandBeginRenderPass(RenderPass rp_in) : rp(rp_in) {}
	};

	struct RenderCommandEndRenderPass: public RenderCommandTyped<RenderCommandType::EndRenderPass>
	{
		u8 nothing;
	};

	struct RenderCommandBarrier : public RenderCommandTyped<RenderCommandType::Barrier>
	{
		std::vector<ResourceBarrier> barriers;

		RenderCommandBarrier() = default;
		RenderCommandBarrier& append(const ResourceBarrier& barr) { barriers.push_back(barr); return *this;  }
	};

	struct RenderCommandCopyBuffer : public RenderCommandTyped<RenderCommandType::CopyBuffer>
	{
		Buffer src, dst;
		u64 src_offset{ 0 };
		u64 dst_offset{ 0 };
		u64 size{ 0 };

		RenderCommandCopyBuffer() = default;
		RenderCommandCopyBuffer(Buffer src_in, u64 src_offset_in, Buffer dst_in, u64 dst_offset_in, u64 size_in) :
			src(src_in),
			dst(dst_in),
			src_offset(src_offset_in),
			dst_offset(dst_offset_in),
			size(size_in) {}
	};

	struct RenderCommandCopyBufferToImage : public RenderCommandTyped<RenderCommandType::CopyBufferToImage>
	{
		Buffer src;
		Texture dst;

		// Destination properties
		u32 dst_subresource{ 0 };
		std::tuple<u32, u32, u32> dst_topleft;

		// Source properties (describes the incoming data)
		u64 src_offset{ 0 };		// offset in src buffer (?)
		ResourceFormat src_format{ ResourceFormat::Unknown };
		u32 src_width{ 0 };
		u32 src_height{ 0 };
		u32 src_depth{ 1 };
		u32 src_rowpitch{ 0 };
		// src BOX optional

		RenderCommandCopyBufferToImage() = default;

	};

	struct RenderCommandUpdateShaderArgs : public RenderCommandTyped<RenderCommandType::UpdateShaderArgs>
	{
		std::array<u32, 10> constants{};
		u32 num_constants{ 0 };

		RenderCommandUpdateShaderArgs() = default;
		RenderCommandUpdateShaderArgs& append_constant(u32 constant) { constants[num_constants++] = constant; assert(num_constants < 10); return *this; }
	};


	struct RenderCommandList
	{
	public:
		// Note that internal will consume the command! (move)
		// We don't explicitly use rvalue to simplify interface (avoid explicit std::move)
		template <typename Command>
		void submit(const Command& cmd)
		{
			// Future notes: Given that a command list is a hot code path. Consider grabbing memory from a custom allocator for command storage instead of using make_shared.
			// We'll keep it simple until we come across performance issues for now
			auto storage = std::make_shared<Command>();
			*storage = std::move(cmd);	// Avoid potential expensive copies (e.g vector in 'Command')

			// Compiler will warn us if Command is not a subclass of RenderCommand :)
			m_cmds.push_back(storage);
		}

		const std::vector<std::shared_ptr<RenderCommand>>& get_commands() const { return m_cmds; };
		bool empty() const { return m_cmds.empty(); }

	private:
		std::vector<std::shared_ptr<RenderCommand>> m_cmds;
	};



}
