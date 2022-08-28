#include "Application.h"
#include "RHI/ShaderCompiler/ShaderCompiler_DXC.h"

#include "RHI/DX12/RenderDevice_DX12.h"
#include "RHI/DX12/RenderBackend_DX12.h"
#include "RHI/PipelineBuilder.h"
#include "Window/Window.h"

#include "Rendering/GPUGarbageBin.h"
#include "Rendering/GPUConstantManager.h"
#include "Rendering/MeshManager.h"
#include "Rendering/TextureManager.h"

#include "Resource/AssimpImporter.h"
#include "Resource/TextureImporter.h"


#include "../shaders/ShaderInterop_Renderer.h"


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


	std::array<mira::Texture, 2> bb_textures;
	std::array<mira::TextureView, 2> bb_rts;
	std::array<mira::RenderPass, 2> bb_rps;

	// Create swapchain (requires at least 2 buffers)
	mira::SwapChain* sc = rd->create_swapchain(m_window->get_hwnd(), 2);

	// Create depth texture (single)
	mira::Texture depth_tex;
	mira::TextureView depth_view;
	{
		mira::TextureDesc desc{};
		desc.width = c_width;
		desc.height = c_height;
		desc.usage = mira::UsageIntent::DepthStencil;
		desc.format = mira::ResourceFormat::D32_FLOAT;
		depth_tex = rd->create_texture(desc);

		depth_view = rd->create_view(depth_tex,
			mira::TextureViewDesc(
				mira::ViewType::DepthStencil, 
				mira::TextureViewRange(mira::TextureViewDimension::Texture2D, mira::ResourceFormat::D32_FLOAT).set_mips(0, 1)));
	}

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
			.add_depth_stencil(depth_view, 
				mira::RenderPassBeginAccessType::Clear, mira::RenderPassEndingAccessType::Discard,		// depth
				mira::RenderPassBeginAccessType::Discard, mira::RenderPassEndingAccessType::Discard)	// stencil
			.build());
	}
	
	// Create mesh pipeline
	mira::Pipeline mesh_pipe;
	{
		auto vs = sclr->compile_from_file("mesh_vs.hlsl", mira::ShaderType::Vertex);
		auto ps = sclr->compile_from_file("mesh_ps.hlsl", mira::ShaderType::Pixel);

		mesh_pipe = rd->create_graphics_pipeline(mira::GraphicsPipelineBuilder()
			.set_shader(vs.get())
			.set_shader(ps.get())
			.append_rt_format(mira::ResourceFormat::RGBA_8_UNORM)

			// Set depth on
			.set_depth_format(mira::DepthFormat::D32)
			.set_depth_stencil(mira::DepthStencilBuilder().set_depth_enabled(true))

			.build());
	}

	mira::GPUGarbageBin bin(1);

	// Create constant data helper
	mira::GPUConstantManager constant_mgr(rd, &bin, 3);

	// Initialize mesh manager
	mira::MeshManager::SizeSpecification spec{};
	spec.index_buffer_size = sizeof(u32) * 10'000'000;
	spec.staging_size = 15'000'000;
	spec.buffer_sizes[mira::VertexAttribute::Position] = 10'000'000;
	spec.buffer_sizes[mira::VertexAttribute::UV] = 10'000'000;
	spec.buffer_sizes[mira::VertexAttribute::Normal] = 10'000'000;
	spec.buffer_sizes[mira::VertexAttribute::Tangent] = 10'000'000;
	mira::MeshManager static_mesh_mgr(rd, &bin, spec);

	// Load sponza
	mira::MeshContainer sponza_mesh;
	{
		mira::AssimpImporter sponza("assets\\models\\Sponza_gltf\\glTF\\Sponza.gltf");
		auto res = sponza.get_result();

		mira::MeshManager::MeshSpecification load_spec{};
		load_spec.indices = res->mesh.indices;
		load_spec.submeshes = res->submeshes;
		for (auto& [attr, mem] : res->mesh.vertex_data)
			load_spec.data[attr] = mem;
		sponza_mesh = static_mesh_mgr.load_mesh(load_spec);	
	}

	// Test texture importer
	mira::TextureImporter::initialize();

	mira::TextureImporter importer("assets\\textures\\ultra.png", false);
	auto tex_res = importer.get_result();
	
	// Test texture manager
	mira::TextureManager tex_man(rd, &bin);
	auto [tex_handle, tex_view] = tex_man.allocate("ultra", tex_res->data_per_mip);
	

	u32 count{ 0 };
	std::array<mira::CommandList, 1> list_hdls;

	while (m_window->is_alive())
	{
		m_window->pump_messages();

		// Wait for GPU
		rd->flush();

		if (count != 0)
			rd->recycle_command_list(list_hdls[0]);
		bin.begin_frame();

		auto curr_bb = bb_textures[sc->get_next_draw_surface_idx()];
		auto curr_bb_rp = bb_rps[sc->get_next_draw_surface_idx()];

		mira::RenderCommandList list;

		list.submit(mira::RenderCommandBarrier()
			.append(mira::ResourceBarrier::transition(curr_bb, mira::ResourceState::Present, mira::ResourceState::RenderTarget, 0))
			.append(mira::ResourceBarrier::transition(depth_tex, count++ == 0 ? mira::ResourceState::Common : mira::ResourceState::DepthRead, mira::ResourceState::DepthWrite, 0))
		);
	
		auto [mem, mesh_table_view] = constant_mgr.allocate_transient(sizeof(ShaderInterop_MeshTable));
		((ShaderInterop_MeshTable*)mem)->submesh_md_array = static_mesh_mgr.get_submesh_metadata_buffer();
		((ShaderInterop_MeshTable*)mem)->vert_pos_array = static_mesh_mgr.get_attribute_buffer(mira::VertexAttribute::Position);
		((ShaderInterop_MeshTable*)mem)->vert_uv_array = static_mesh_mgr.get_attribute_buffer(mira::VertexAttribute::UV);
		((ShaderInterop_MeshTable*)mem)->vert_nor_array = static_mesh_mgr.get_attribute_buffer(mira::VertexAttribute::Normal);
		((ShaderInterop_MeshTable*)mem)->vert_tangent_array = static_mesh_mgr.get_attribute_buffer(mira::VertexAttribute::Tangent);
		
		auto [frame_mem, frame_view] = constant_mgr.allocate_transient(sizeof(ShaderInterop_PerFrame));
		((ShaderInterop_PerFrame*)frame_mem)->view_matrix = DirectX::XMMatrixLookAtLH({ 3.f, 4.f, 0.f }, { -2.f, 3.f, 2.f }, { 0.f, 1.f, 0.f });

#ifdef USE_REVERSE_Z
		((ShaderInterop_PerFrame*)frame_mem)->projection_matrix = DirectX::XMMatrixPerspectiveFovLH(80.f * 3.1415 / 180.f, (float)c_width / c_height, 500.f, 1.f);
#else
		((ShaderInterop_PerFrame*)frame_mem)->projection_matrix = DirectX::XMMatrixPerspectiveFovLH(80.f * 3.1415 / 180.f, (float)c_width / c_height, 0.1f, 500.f);
#endif
	
		// Draw
		list.submit(mira::RenderCommandBeginRenderPass(curr_bb_rp));
		{
			// Testing: Using same PerDraw data for all submeshes
			auto [draw_mem, draw_view] = constant_mgr.allocate_transient(sizeof(ShaderInterop_PerDraw));
			((ShaderInterop_PerDraw*)draw_mem)->world_matrix = DirectX::XMMatrixScaling(0.07f, 0.07f, 0.07f);

			list.submit(mira::RenderCommandSetPipeline(mesh_pipe));
			for (u32 sm = 0; sm < sponza_mesh.num_submeshes; ++sm)
			{
				list.submit(mira::RenderCommandUpdateShaderArgs()
					.append_constant(mesh_table_view)
					.append_constant(static_mesh_mgr.get_submesh_metadata_index(sponza_mesh.mesh, sm))
					.append_constant(frame_view)
					.append_constant(draw_view)
					.append_constant(tex_view)
				);

				const auto& submesh_md = static_mesh_mgr.get_submesh_metadata(sponza_mesh.mesh, sm);
				
				list.submit(mira::RenderCommandDrawIndexed(static_mesh_mgr.get_index_buffer(), submesh_md.index_count, 1, submesh_md.index_start, 0, 0));
			}
		}
		list.submit(mira::RenderCommandEndRenderPass());

		list.submit(mira::RenderCommandBarrier()
			.append(mira::ResourceBarrier::transition(curr_bb, mira::ResourceState::RenderTarget, mira::ResourceState::Present, 0))
			.append(mira::ResourceBarrier::transition(depth_tex, mira::ResourceState::DepthWrite, mira::ResourceState::DepthRead, 0))
		);

		list_hdls[0] = rd->allocate_command_list(mira::QueueType::Graphics);
		rd->compile_command_list(list_hdls[0], list);
		rd->submit_command_lists(list_hdls);

		// present to swapchain
		sc->present(false);

		bin.end_frame();
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
