#include "Application.h"
#include "RHI/ShaderCompiler/ShaderCompiler_DXC.h"

#include "RHI/DX12/RenderDevice_DX12.h"
#include "RHI/DX12/RenderBackend_DX12.h"
#include "RHI/PipelineBuilder.h"
#include "Window/Window.h"

#include "Rendering/MeshManager.h"
#include "Rendering/GPUGarbageBin.h"

#include "Resource/AssimpImporter.h"

#include "Rendering/GPUConstantManager.h"

Application::Application()
{
	const UINT c_width = 1600;
	const UINT c_height = 900;

	auto win_proc_callback = [this](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT { return this->window_proc(hwnd, uMsg, wParam, lParam); };
	m_window = std::make_unique<Window>(GetModuleHandle(NULL), win_proc_callback, c_width, c_height);

	auto sclr = std::make_unique<mira::ShaderCompiler_DXC>();
#ifdef _DEBUG
	auto be_dx = std::make_unique<mira::RenderBackend_DX12>(true);
#else
	auto be_dx = std::make_unique<mira::RenderBackend_DX12>(false);
#endif
	auto rd = be_dx->create_device();


	mira::GPUGarbageBin bin(1);
	mira::GPUConstantManager constant_mgr(rd, &bin, 1);
	

		//// Initialize mesh manager
		//mira::MeshManager::SizeSpecification spec{};
		//spec.index_buffer_size = sizeof(u32) * 10'000'000;
		//spec.staging_size = 15'000'000;
		//spec.buffer_sizes[mira::VertexAttribute::Position] = 10'000'000;
		//spec.buffer_sizes[mira::VertexAttribute::UV] = 10'000'000;
		//spec.buffer_sizes[mira::VertexAttribute::Normal] = 10'000'000;
		//spec.buffer_sizes[mira::VertexAttribute::Tangent] = 10'000'000;
		//mira::MeshManager static_mesh_mgr(rd, &bin, spec);
		//
		//// Load mesh
		//{
		//	// Test mesh manager
		//	mira::AssimpImporter sponza("assets\\models\\Sponza_gltf\\glTF\\Sponza.gltf");
		//	auto res = sponza.get_result();

		//	mira::MeshManager::MeshSpecification load_spec{};
		//	load_spec.data[mira::VertexAttribute::Position] = res->mesh.vertex_data[mira::VertexAttribute::Position];
		//	load_spec.data[mira::VertexAttribute::UV] = res->mesh.vertex_data[mira::VertexAttribute::UV];
		//	load_spec.data[mira::VertexAttribute::Normal] = res->mesh.vertex_data[mira::VertexAttribute::Normal];
		//	load_spec.data[mira::VertexAttribute::Tangent] = res->mesh.vertex_data[mira::VertexAttribute::Tangent];
		//	load_spec.indices = res->mesh.indices;
		//	load_spec.submeshes = res->submeshes;
		//	static_mesh_mgr.load_mesh(load_spec);
		//}

		//std::cout << "Loaded!\n";


	std::array<mira::Texture, 2> bb_textures;
	std::array<mira::TextureView, 2> bb_rts;
	std::array<mira::RenderPass, 2> bb_rps;

	// Create swapchain (requires at least 2 buffers)
	mira::SwapChain* sc = rd->create_swapchain(m_window->get_hwnd(), 2);

	// Create backbuffer renderpasses
	for (u32 i = 0; i < bb_rps.size(); ++i)
	{
		bb_textures[i] = sc->get_buffer(i);

		bb_rts[i] = rd->create_view(bb_textures[i],
			mira::TextureViewDesc(
				mira::ViewType::RenderTarget,
				mira::TextureViewRange(mira::TextureViewDimension::Texture2D, mira::ResourceFormat::RGBA_8_UNORM)));

		bb_rps[i] = rd->create_renderpass(mira::RenderPassBuilder()
			.append_rt(bb_rts[i], mira::RenderPassBeginAccessType::Clear, mira::RenderPassEndingAccessType::Preserve)
			.build());
	}

	// Create fullscreen blit pipeline
	mira::Pipeline blit_pipe;
	{
		auto vs = sclr->compile_from_file("fullscreen_tri_vs.hlsl", mira::ShaderType::Vertex);
		auto ps = sclr->compile_from_file("blit_ps.hlsl", mira::ShaderType::Pixel);

		blit_pipe = rd->create_graphics_pipeline(mira::GraphicsPipelineBuilder()
			.set_shader(vs.get())
			.set_shader(ps.get())
			.append_rt_format(mira::ResourceFormat::RGBA_8_UNORM)
			.build());
	}

	struct TestCB
	{
		u32 a, b, c, d;
	};

	//TestCB data{};
	//data.a = 0.1f;
	//data.b = 0.7f;
	//data.c = 0.5f;
	//data.d = 0.8f;
	//mira::PersistentConstant constant = constant_mgr.allocate_persistent(sizeof(TestCB), (u8*)&data, sizeof(TestCB));
	//constant_mgr.execute_copies(false);

	auto [mem, idx] = constant_mgr.allocate_transient(sizeof(TestCB));
	((TestCB*)mem)->a = 0;
	((TestCB*)mem)->b = 1;
	((TestCB*)mem)->c = 1;
	((TestCB*)mem)->d = 1;

	while (m_window->is_alive())
	{
		m_window->pump_messages();

		//auto curr_bb = sc->get_next_draw_surface();
		auto curr_bb = bb_textures[sc->get_next_draw_surface_idx()];
		auto curr_bb_rp = bb_rps[sc->get_next_draw_surface_idx()];

		mira::RenderCommandList list;

		list.submit(mira::RenderCommandBarrier()
			.append(mira::ResourceBarrier::transition(curr_bb, mira::ResourceState::Present, mira::ResourceState::RenderTarget, 0))
		);
	
		list.submit(mira::RenderCommandSetPipeline(blit_pipe));

		//auto idx = constant_mgr.get_global_view(constant);
		list.submit(mira::RenderCommandUpdateShaderArgs()
			.append_constant(idx)
			.append_constant(1)
			.append_constant(1)
		);

		list.submit(mira::RenderCommandBeginRenderPass(curr_bb_rp));
		list.submit(mira::RenderCommandDraw(3, 1, 0, 0));
		list.submit(mira::RenderCommandEndRenderPass());


		list.submit(mira::RenderCommandBarrier()
			.append(mira::ResourceBarrier::transition(curr_bb, mira::ResourceState::RenderTarget, mira::ResourceState::Present, 0))
		);

		std::array<mira::CommandList, 1> list_hdls;
		list_hdls[0] = rd->allocate_command_list(mira::QueueType::Graphics);
		rd->compile_command_list(list_hdls[0], list);
		rd->submit_command_lists(list_hdls);			// Sync with constant copy

		// present to swapchain
		sc->present(false);

		bin.end_frame();

		rd->flush();			// wait for GPU frame

		bin.begin_frame();

		rd->recycle_command_list(list_hdls[0]);
	}

	rd->flush();
}

void Application::run()
{

}

LRESULT Application::window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//if (IGContext::get().win_proc(hwnd, uMsg, wParam, lParam))
	//	return true;

	switch (uMsg)
	{
	case WM_ACTIVATEAPP:
	{
		//Input::get().process_keyboard(uMsg, wParam, lParam);
		//Input::get().process_mouse(uMsg, wParam, lParam);
	}
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
	{
		//Input::get().process_mouse(uMsg, wParam, lParam);
		break;
	}

	case WM_KEYDOWN:
		//if (wParam == VK_ESCAPE)
		//{
		//	m_app_alive = false;
		//}
	case WM_KEYUP:
	case WM_SYSKEYUP:
		//Input::get().process_keyboard(uMsg, wParam, lParam);
		break;


		// Only called on pressing "X"
	case WM_CLOSE:
	{
		//std::cout << "what\n";
		//KillApp();
		break;
	}

	// Resize message
	case WM_SIZE:
	{
		/*
			This is called frequently when resizing happens.
			Hence why m_allow_resize is used so that we can defer the operations until later
		*/
		//m_resize_allowed = true;
		//m_resized_client_area.first = LOWORD(lParam);
		//m_resized_client_area.second = HIWORD(lParam);
		break;
	}
	case WM_ENTERSIZEMOVE:
	{
		//m_paused = true;
		break;
	}
	case WM_EXITSIZEMOVE:
	{
		//m_paused = false;

		//if (m_resize_allowed)
		//{
		//	m_should_resize = true;
		//	m_resize_allowed = false;
		//}
		//break;
	}

	case WM_SYSKEYDOWN:
	{
		//// Custom Alt + Enter to toggle windowed borderless (disabled for now)
		///*
		//	DO NOT DRAG IMGUI WINDOWS IN FULLSCREEN!
		//*/
		//if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
		//{
		//	m_win->set_fullscreen(!m_win->is_fullscreen());
		//	if (m_resize_allowed)
		//	{
		//		m_should_resize = true;
		//		m_resize_allowed = false;
		//	}
		//}
		break;
	}

	default:
	{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);

}
