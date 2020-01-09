#pragma once

#include "stdafx.h"
#include "ResourceState.h"
#include "Resource.h"
#include "DescriptorHeap.h"

#include <d3d12.h>

class UploadManager;

class Texture2D : public Resource
{
public:
	Texture2D(DXGI_FORMAT format, uint32_t width, uint32_t height, void* data, uint32_t dataBPT, UploadManager& uploadMngr);
	~Texture2D();

	const DescriptorHandle& SRVHandle() const { return m_SRVHandle; }

private:
	DescriptorHandleWithIdx		m_SRVHandle;
};
