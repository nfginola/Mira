#include <iostream>

#include "RHI//DX12/RenderDevice_DX12.h"
#include "RHI/DX12/RenderBackend_DX12.h"
#include "RHI/DX12/ShaderCompiler_DXC.h"
#include "RHI/PipelineBuilder.h"

#include "TypedHandlePool.h"

int main()
{
	/*
		
		WIP,
		setup window,
		setup swapchain
		setup renderer (frame buffer)
	
	*/

	TypedHandlePool rhp;

	auto be_dx = std::make_unique<mira::RenderBackend_DX12>(true);
	mira::RenderDevice* rd = be_dx->create_device();
	auto sclr = std::make_unique<mira::ShaderCompiler_DXC>();

	auto bb1 = rhp.allocate_handle<mira::Texture>();
	auto bb2 = rhp.allocate_handle<mira::Texture>();
	//rd->create_swapchain(hwnd, { bb1, bb2 });


	auto blit_pipe = rhp.allocate_handle<mira::Pipeline>();
	{
		auto vs = sclr->compile_from_file("fullscreen_tri_vs.hlsl", mira::ShaderType::Vertex);
		auto ps = sclr->compile_from_file("blit_ps.hlsl", mira::ShaderType::Pixel);

		rd->create_graphics_pipeline(mira::GraphicsPipelineBuilder()
			.set_shader(vs.get())
			.set_shader(ps.get())
			.append_rt_format(mira::ResourceFormat::RGBA_8_UNORM)
			.build(), 
			blit_pipe);
	}



	auto color_tex = rhp.allocate_handle<mira::Texture>();
	mira::TextureDesc td{};
	td.width = 1600;
	td.height = 900;
	td.format = mira::ResourceFormat::RGBA_32_FLOAT;
	td.usage = mira::UsageIntent::ShaderResource | mira::UsageIntent::RenderTarget;
	td.memory_type = mira::MemoryType::Default;
	td.clear_color = { 1.f, 0.f, 0.f, 1.f };
	rd->create_texture(td, color_tex);

	
	auto clear_color_pass = mira::RenderPassBuilder()
		.append_rt(color_tex, 0, mira::RenderPassBeginAccessType::Clear, mira::RenderPassEndingAccessType::Preserve)
		.build();
	auto rp = rhp.allocate_handle<mira::RenderPass>();
	rd->create_renderpass(clear_color_pass, rp);
		

	



	auto buf1 = rhp.allocate_handle<mira::Buffer>();
	auto staging_buf = rhp.allocate_handle<mira::Buffer>();

	// buffer with properties now attached to hdl
	mira::BufferDesc bd{};
	bd.size = 256;
	bd.usage = mira::UsageIntent::Constant;
	bd.memory_type = mira::MemoryType::Default;
	rd->create_buffer(bd, buf1);	


	// record cmd list
	auto cmd_list = rd->allocate_command_list();

	cmd_list->set_pipeline(blit_pipe);

	// set render pass
	cmd_list->begin_renderpass(rp);
	cmd_list->draw(3, 1, 0, 0);
	cmd_list->end_renderpass();



	// submit cmd list
	mira::RenderCommandList* cmdls[] = { cmd_list };
	auto test = rd->submit_command_lists(1, cmdls, mira::QueueType::Graphics, true);

	rd->free_buffer(buf1);


	//rd->wait_for_gpu(std::move(*test));
	rd->flush();

	rhp.free_handle(blit_pipe);

	rd->free_texture(color_tex);



	return 0;
}