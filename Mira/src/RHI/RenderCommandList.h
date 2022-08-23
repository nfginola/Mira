#pragma once
#include "RHITypes.h"
#include <span>

#include <iostream>

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

	//enum class RenderCommandType
	//{
	//	None,
	//	Draw,
	//	SetPipeline
	//};

	//struct RenderCommand
	//{
	//	RenderCommandType type = RenderCommandType::None;
	//};

	//template <RenderCommandType CMD_TYPE>
	//struct RenderCommandTyped : public RenderCommand
	//{
	//	static const RenderCommandType s_cmd = CMD_TYPE;

	//	RenderCommandTyped() { type = s_cmd; }
	//};

	//struct RenderCommandDraw : public RenderCommandTyped<RenderCommandType::Draw>
	//{
	//	u32 verts_per_instance{ 0 };
	//	u32 instance_count{ 0 };
	//	u32 vert_start{ 0 };
	//	u32 instance_start{ 0 };
	//};

	//
	//class NewRenderCommandList
	//{
	//public:	
	//	void submit(RenderCommandDraw cmd)
	//	{
	//		auto stored = new RenderCommandDraw();
	//		*stored = cmd;
	//		m_cmds.push_back(stored);
	//	}

	//	std::vector<RenderCommand*> m_cmds;
	//};

	//struct RenderCommandDraw
	//{
	//	u32 verts_per_instance{ 0 };
	//	u32 instance_count{ 0 };
	//	u32 vert_start{ 0 };
	//	u32 instance_start{ 0 };
	//};

	/*
		Store render commands in a classic "Command + Enum" fashion to know what
		underlying type is stored.
	*/
	enum class RenderCommandType
	{
		None,
		Draw,
		SetPipeline,
		BeginRenderPass,
		EndRenderPass,
		UpdateShaderArgs
	};
	
	struct RenderCommand
	{
		RenderCommandType CMD_TYPE = RenderCommandType::None;
		RenderCommand(RenderCommandType type) : CMD_TYPE(type) {}
	};

	struct RenderCommandDraw : public RenderCommand
	{
		u32 verts_per_instance{ 0 };
		u32 instance_count{ 0 };
		u32 vert_start{ 0 };
		u32 instance_start{ 0 };

		RenderCommandDraw() : RenderCommand(RenderCommandType::Draw) {}
	};

	struct RenderCommandSetPipeline : public RenderCommand
	{
		Pipeline pipeline;

		RenderCommandSetPipeline() : RenderCommand(RenderCommandType::SetPipeline) {}
	};




	struct NewRenderCommandList
	{
	public:
		void submit(RenderCommandDraw cmd)
		{
			auto storage = new RenderCommandDraw;
			*storage = cmd;
			m_cmds.push_back(storage);
		}

		std::vector<RenderCommand*> m_cmds;
	};

}
