#pragma once

#include "stdafx.h"
#include "Camera.h"
#include "UploadManager.h"
#include "Buffer.h"
#include "RenderTarget.h"
#include "Geometry.h"

class Window;
class DXCommandList;

class Engine
{
public:
	Engine(const Window& window);
	Engine(const Engine& other) = delete;
	~Engine() = default;

	void RenderFrame();

private:
	void LoadGeometry();
	void LoadShaders();

	struct GlobalFrameConstatns
	{
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 viewProj;
	};

	std::unique_ptr<DepthTarget>				m_DepthBuffer;
	std::vector<std::unique_ptr<RenderTarget>>	m_BackBuffers;
	std::unique_ptr<DXCommandList>				m_CommandList;

	Camera										m_Camera;

	std::vector<std::unique_ptr<Geometry>>		m_SceneGeometry;

	ReleasedUniquePtr<ID3DBlob>					m_VertexShader;
	ReleasedUniquePtr<ID3DBlob>					m_PixelShader;

	ReleasedUniquePtr<ID3D12PipelineState>		m_PrimaryPSO;
	ReleasedUniquePtr<ID3D12RootSignature>		m_PrimaryRootSignature;

	GlobalFrameConstatns						m_FrameConstantData;
	std::array<std::unique_ptr<ConstantBuffer>, kNumFramesInFlight>		m_FrameConstBuffers;

	UploadManager								m_UploadManager;

	size_t										m_Frame;
};
