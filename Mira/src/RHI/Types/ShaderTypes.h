#pragma once
#include "../../Common.h"

namespace mira
{
	enum class ShaderType
	{
		Vertex,
		Geometry,
		Hull,
		Domain,
		Pixel,
		Compute
	};

	enum class ShaderModel
	{
		SM_6_6
	};

	struct CompiledShader
	{
		std::vector<u8> blob;
		ShaderType shader_type;

		CompiledShader() = default;

		CompiledShader(void* data, size_t size, ShaderType shader) :
			shader_type(shader)
		{
			blob.resize(size);
			uint8_t* start = (uint8_t*)data;
			uint8_t* end = start + size;
			std::copy(start, end, blob.data());
		}
		~CompiledShader() = default;
	};

}
