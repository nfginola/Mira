#include "ShaderCompiler_DXC.h"
#include <dxcapi.h>

namespace mira
{
	ShaderCompiler_DXC::ShaderCompiler_DXC(ShaderModel model) :
		m_shader_model(model)
	{
		// Grab interfaces
		HRESULT hr = S_OK;

		hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&m_library));
		if (FAILED(hr))
			throw std::runtime_error("Failed to intiialize IDxcLibrary");

		hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler));
		if (FAILED(hr))
			throw std::runtime_error("Failed to initialize IDxcCompiler");

		hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils));
		if (FAILED(hr))
			throw std::runtime_error("Failed to initialize IDxcUtils");

		// Grab default include handler
		m_utils->CreateDefaultIncludeHandler(m_def_inc_hdlr.GetAddressOf());
	}

	ShaderCompiler_DXC::~ShaderCompiler_DXC()
	{
	}

	std::shared_ptr<CompiledShader> mira::ShaderCompiler_DXC::compile_from_file(std::filesystem::path rel_path, ShaderType type, const std::string& entry_point)
	{
		std::wstring entry_wstr = std::filesystem::path(entry_point).wstring();

		// Prepend directory
		rel_path = std::filesystem::path("shaders\\" + rel_path.string());

		std::wstring profile = grab_profile(type);

		// Create blob to store compiled data
		uint32_t code_page = CP_UTF8;
		ComPtr<IDxcBlobEncoding> source_blob;

		HRESULT hr;
		hr = m_library->CreateBlobFromFile(rel_path.c_str(), &code_page, &source_blob);
		if (FAILED(hr))
			throw std::runtime_error("Failed to create blob");

		// Compile
		ComPtr<IDxcOperationResult> result;
		hr = m_compiler->Compile(
			source_blob.Get(), // pSource
			rel_path.c_str(),// pSourceName
			entry_wstr.c_str(), // pEntryPoint
			profile.c_str(), // pTargetProfile
			NULL, 0, // pArguments, argCount
			NULL, 0, // pDefines, defineCount
			m_def_inc_hdlr.Get(),
			result.GetAddressOf()); // ppResult

		if (SUCCEEDED(hr))
			result->GetStatus(&hr);

		// Check error
		if (FAILED(hr))
		{
			ComPtr<IDxcBlobEncoding> errors;
			hr = result->GetErrorBuffer(&errors);
			if (SUCCEEDED(hr) && errors)
			{
				wprintf(L"Compilation failed with errors:\n%hs\n",
					(const char*)errors->GetBufferPointer());
				assert(false);
			}
		}

		ComPtr<IDxcBlob> res;
		hr = result->GetResult(res.GetAddressOf());
		if (FAILED(hr))
			assert(false);

		return std::make_shared<CompiledShader>(res->GetBufferPointer(), res->GetBufferSize(), type);
	}

	std::wstring ShaderCompiler_DXC::grab_profile(ShaderType shader_type)
	{
		std::wstring profile;
		switch (shader_type)
		{
		case ShaderType::Vertex:
			profile = L"vs";
			break;
		case ShaderType::Pixel:
			profile = L"ps";
			break;
		case ShaderType::Geometry:
			profile = L"gs";
			break;
		case ShaderType::Hull:
			profile = L"hs";
			break;
		case ShaderType::Domain:
			profile = L"ds";
			break;
		case ShaderType::Compute:
			profile = L"cs";
			break;
		default:
			assert(false);
		}
		profile += L"_";

		switch (m_shader_model)
		{
		case ShaderModel::SM_6_6:
			profile += L"6_6";
			break;
		default:
			assert(false);
		}
		return profile;
	}
}


