#include "Application.h"

#include "RHI//DX12/RenderDevice_DX12.h"
#include "RHI/DX12/RenderBackend_DX12.h"
#include "RHI/DX12/ShaderCompiler_DXC.h"
#include "RHI/PipelineBuilder.h"
#include "Window/Window.h"

#include "Handles/TypedHandlePool.h"

#include "Handles/HandleAllocator.h"

Application::Application()
{
	const UINT c_width = 1600;
	const UINT c_height = 900;

	auto win_proc_callback = [this](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
	{
		return this->window_proc(hwnd, uMsg, wParam, lParam);
	};
	m_window = std::make_unique<Window>(GetModuleHandle(NULL), win_proc_callback, c_width, c_height);

	mira::HandleAllocator rhp;
	auto sclr = std::make_unique<mira::ShaderCompiler_DXC>();
	auto be_dx = std::make_unique<mira::RenderBackend_DX12>(true);
	auto rd = be_dx->create_device();

	// Create swapchain (requires at least 2 buffers)
	mira::Texture bb_textures[]{ rhp.allocate<mira::Texture>(), rhp.allocate<mira::Texture>() };
	mira::SwapChain* sc = rd->create_swapchain(m_window->get_hwnd(), bb_textures);

	// Create fullscreen blit pipeline
	auto blit_pipe = rhp.allocate<mira::Pipeline>();
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

	// Create renderpasses for swapchain
	mira::RenderPass bb_rps[2] = { rhp.allocate<mira::RenderPass>(), rhp.allocate<mira::RenderPass>() };
	for (u32 i = 0; i < _countof(bb_rps); ++i)
	{
		/*
			we should change append_rt to take a TextureView instead.

			Its like we're baking in a Framebuffer here (array of image views)
		*/

		rd->create_renderpass(mira::RenderPassBuilder()
			.append_rt(bb_textures[i], 0, mira::RenderPassBeginAccessType::Clear, mira::RenderPassEndingAccessType::Preserve)
			.build(),
			bb_rps[i]);
	}

	// view testing
	{
		auto buf = rhp.allocate<mira::Buffer>();
		mira::BufferDesc bdesc{};
		bdesc.size = 512;
		bdesc.memory_type = mira::MemoryType::Upload;
		rd->create_buffer(bdesc, buf);

		auto v1 = rd->create_view(buf, mira::ViewType::Constant, 0, 256);
		auto v2 = rd->create_view(buf, mira::ViewType::Constant, 256, 256);

		auto handle = rd->get_global_descriptor(buf, v2);

		rd->free_buffer(buf);
	}


	while (m_window->is_alive())
	{
		m_window->pump_messages();

		auto curr_bb = sc->get_next_draw_surface();
		auto curr_bb_rp = bb_rps[sc->get_next_draw_surface_idx()];

		auto cmd_list = rd->allocate_command_list();

		mira::ResourceBarrier barrs_before[]
		{ 
			mira::ResourceBarrier::transition(curr_bb, mira::ResourceState::Present, mira::ResourceState::RenderTarget, 0) 
		};
		cmd_list->submit_barriers(barrs_before);

		cmd_list->set_pipeline(blit_pipe);
		cmd_list->begin_renderpass(curr_bb_rp);
		cmd_list->draw(3, 1, 0, 0);
		cmd_list->end_renderpass();

		mira::ResourceBarrier barrs_after[]
		{
			mira::ResourceBarrier::transition(curr_bb, mira::ResourceState::RenderTarget, mira::ResourceState::Present, 0)
		};
		cmd_list->submit_barriers(barrs_after);

		// submit cmd list
		mira::RenderCommandList* cmdls[] = { cmd_list };
		auto receipt = rhp.allocate<mira::SyncReceipt>();
		rd->submit_command_lists(cmdls, mira::QueueType::Graphics, {}, receipt);

		// present to swapchain
		sc->present(false);

		// wait for GPU frame
		//rd->flush();
		rd->wait_for_gpu(receipt);
		rhp.free(receipt);		// sync recycled internally since a CPU wait was done

		rd->recycle_command_list(cmd_list);
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
