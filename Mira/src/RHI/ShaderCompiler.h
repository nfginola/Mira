#pragma once
#include "Types/ShaderTypes.h"

namespace mira
{
	/*
		Plan is to only support DXC anyways. Should we keep this interface?
	*/
	class ShaderCompiler
	{
	public:
		// Handle specific arguments later
		virtual std::shared_ptr<CompiledShader> compile_from_file(
			std::filesystem::path rel_path,
			ShaderType type,
			const std::string& entry_point = "main") = 0;

		virtual ~ShaderCompiler() {}
	};
	

}
