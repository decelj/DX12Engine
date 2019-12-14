#include "stdafx.h"
#include "DXCompiler.h"
#include "HRException.h"

#include <wrl.h>
#include <dxcapi.h>
#include <d3dcompiler.h>
#include <locale>
#include <codecvt>

using Microsoft::WRL::ComPtr;

std::unique_ptr<DXCompiler> DXCompiler::s_Instance = nullptr;

namespace Impl
{
	class DIXLCompiler : public DXCompiler
	{
	public:
		DIXLCompiler();
		virtual ~DIXLCompiler();

		ID3DBlob* CompileShader(
			const std::string& shaderCode, const std::string& entryPoint, const std::string& type,
			const std::vector<D3D_SHADER_MACRO>& defines) override final;
		ID3DBlob* CompileShaderFromFile(
			const std::wstring& fileName, const std::string& entryPoint, const std::string& type,
			const std::vector<D3D_SHADER_MACRO>& defines) override final;

	private:
		ComPtr<IDxcLibrary>		m_Library;
		ComPtr<IDxcCompiler>	m_Compiler;
		HMODULE					m_DXCLib;
	};

	DIXLCompiler::DIXLCompiler()
		: DXCompiler()
		, m_Library(nullptr)
		, m_Compiler(nullptr)
		, m_DXCLib(nullptr)
	{
		constexpr const char* kLibraryName = "dxcompiler.dll";
		constexpr const char* kCreateInstanceName = "DxcCreateInstance";

		HMODULE m_DXCLib = ::LoadLibraryA(kLibraryName);
		if (m_DXCLib == nullptr)
		{
			throw std::runtime_error(std::string("Could not load ") + kLibraryName);
		}

		DxcCreateInstanceProc DXCCreateFunc = (DxcCreateInstanceProc)::GetProcAddress(m_DXCLib, kCreateInstanceName);
		if (DXCCreateFunc == nullptr)
		{
			throw std::runtime_error(std::string("Could not find function ") + kCreateInstanceName);
		}

		ThrowIfFailed(DXCCreateFunc(CLSID_DxcLibrary, __uuidof(IDxcLibrary), (void**)&m_Library));
		ThrowIfFailed(DXCCreateFunc(CLSID_DxcCompiler, __uuidof(IDxcCompiler), (void**)(&m_Compiler)));
	}

	DIXLCompiler::~DIXLCompiler()
	{
		m_Compiler = nullptr;
		m_Library = nullptr;

		::FreeLibrary(m_DXCLib);
		m_DXCLib = nullptr;
	}

	ID3DBlob* DIXLCompiler::CompileShader(
		const std::string& shaderCode, const std::string& entryPoint, const std::string& type,
		const std::vector<D3D_SHADER_MACRO>& defines)
	{
		// TODO
		assert(false);
		return nullptr;
	}

	ID3DBlob* DIXLCompiler::CompileShaderFromFile(
		const std::wstring& fileName, const std::string& entryPoint, const std::string& type,
		const std::vector<D3D_SHADER_MACRO>& defines)
	{
		// TODO
		assert(false);
		return nullptr;
	}


	class SM5Compiler : public DXCompiler
	{
	public:
		SM5Compiler() 
			: DXCompiler()
		{}
		virtual ~SM5Compiler() {}

		ID3DBlob* CompileShader(
			const std::string& shaderCode, const std::string& entryPoint, const std::string& type,
			const std::vector<D3D_SHADER_MACRO>& defines) override final;
		ID3DBlob* CompileShaderFromFile(
			const std::wstring& fileName, const std::string& entryPoint, const std::string& type,
			const std::vector<D3D_SHADER_MACRO>& defines) override final;

	private:
	};

	ID3DBlob* SM5Compiler::CompileShader(
		const std::string& shaderCode, const std::string& entryPoint, const std::string& type,
		const std::vector<D3D_SHADER_MACRO>& defines)
	{
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
		// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
		// Setting this flag improves the shader debugging experience, but still allows 
		// the shaders to be optimized and to run exactly the way they will run in 
		// the release configuration of this program.
		dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

		const std::string target = type + "_5_0";

		ID3DBlob* errorBlob = nullptr;
		ID3DBlob* shaderBlob = nullptr;
		HRESULT hr = D3DCompile2(
			shaderCode.c_str(),			// Shader text 
			shaderCode.size(),			// Text length
			nullptr,					// Name of source file for error message
			defines.data(),				// Defines
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entryPoint.c_str(),			// Main function in shader
			target.c_str(),				// Shader target version
			dwShaderFlags,
			0, 0, nullptr, 0,			// Flags and secondary data
			&shaderBlob, &errorBlob);	// Output blobs

		if (FAILED(hr))
		{
			if (errorBlob != nullptr)
			{
				OutputDebugStringA((const char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}

			return nullptr;
			ThrowIfFailed(hr);
		}

		if (errorBlob)
		{
			errorBlob->Release();
			errorBlob = nullptr;
		}

		return shaderBlob;
	}

	ID3DBlob* SM5Compiler::CompileShaderFromFile(
		const std::wstring& fileName, const std::string& entryPoint, const std::string& type,
		const std::vector<D3D_SHADER_MACRO>& defines)
	{
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
		// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
		// Setting this flag improves the shader debugging experience, but still allows 
		// the shaders to be optimized and to run exactly the way they will run in 
		// the release configuration of this program.
		dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

		const std::string target = type + "_5_0";
		const std::wstring absShaderPath = ResolveShaderPath(fileName);

		ID3DBlob* errorBlob = nullptr;
		ID3DBlob* shaderBlob = nullptr;
		HRESULT hr = D3DCompileFromFile(
			absShaderPath.c_str(),
			defines.data(),
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entryPoint.c_str(),
			target.c_str(),
			dwShaderFlags,
			0,
			&shaderBlob,
			&errorBlob
		);

		if (FAILED(hr))
		{
			if (errorBlob != nullptr)
			{
				OutputDebugStringA((const char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}

			ThrowIfFailed(hr);
			return nullptr;
		}

		if (errorBlob)
		{
			errorBlob->Release();
			errorBlob = nullptr;
		}

		return shaderBlob;
	}
}

DXCompiler::DXCompiler()
{
}

void DXCompiler::Initialize()
{
	assert(s_Instance == nullptr);
	s_Instance.reset(new Impl::SM5Compiler());
}

void DXCompiler::Destory()
{
	s_Instance.reset();
}

std::wstring DXCompiler::ResolveShaderPath(const std::wstring& shader) const
{
	namespace fs = std::filesystem;
	for (const fs::path& path : m_SearchPaths)
	{
		fs::path absPath = path / shader;
		if (fs::exists(absPath))
		{
			return absPath.make_preferred();
		}
	}

	std::string error = "Could not find shader \"";
	error += std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(shader);
	error += "\"";
	throw std::runtime_error(error);

	return std::wstring();
}
