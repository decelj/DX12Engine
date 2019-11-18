#pragma once

#include "stdafx.h"
#include <d3d12.h>

class DXCompiler
{
public:
	virtual ~DXCompiler();
	DXCompiler(const DXCompiler&) = delete;

	static void Initialize();
	static void Destory();
	static DXCompiler& Instance() { return *s_Instance; }

	virtual ID3DBlob* CompileShader(
		const std::string& shaderCode, const std::string& entryPoint, const std::string& type,
		const std::vector<D3D_SHADER_MACRO>& defines) = 0;
	virtual ID3DBlob* CompileShaderFromFile(
		const std::wstring& fileName, const std::string& entryPoint, const std::string& type,
		const std::vector<D3D_SHADER_MACRO>& defines) = 0;

protected:
	DXCompiler();

	static std::unique_ptr<DXCompiler>	s_Instance;
};
