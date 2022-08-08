#pragma once
#include "DX12CommonIncludes.h"
#include "../ShaderCompiler.h"

struct IDxcLibrary;
struct IDxcUtils;
struct IDxcCompiler;
struct IDxcIncludeHandler;

namespace mira
{
	class ShaderCompiler_DXC final : public ShaderCompiler
	{
	public:
		ShaderCompiler_DXC(ShaderModel model = ShaderModel::SM_6_6);
		~ShaderCompiler_DXC();

		std::shared_ptr<CompiledShader> compile_from_file(
			std::filesystem::path rel_path,
			ShaderType type,
			const std::string& entry_point = "main");

	private:
		std::wstring grab_profile(ShaderType shader_type);

	private:
		ShaderModel m_shader_model;

		ComPtr<IDxcLibrary> m_library;
		ComPtr<IDxcUtils> m_utils;
		ComPtr<IDxcCompiler> m_compiler;
		ComPtr<IDxcIncludeHandler> m_def_inc_hdlr;

	};
}


