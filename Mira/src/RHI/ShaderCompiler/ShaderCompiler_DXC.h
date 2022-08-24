#pragma once
#include "../ShaderCompiler.h"
#include <wrl/client.h>

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

		Microsoft::WRL::ComPtr<IDxcLibrary> m_library;
		Microsoft::WRL::ComPtr<IDxcUtils> m_utils;
		Microsoft::WRL::ComPtr<IDxcCompiler> m_compiler;
		Microsoft::WRL::ComPtr<IDxcIncludeHandler> m_def_inc_hdlr;

	};
}


