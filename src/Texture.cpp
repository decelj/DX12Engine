#include "stdafx.h"
#include "Texture.h"
#include "UploadManager.h"
#include "DXDevice.h"


Texture2D::Texture2D(DXGI_FORMAT format, uint32_t width, uint32_t height, void* data, uint32_t dataBPT, UploadManager& uploadMngr)
	: Resource(ResourceDimension::Texture2D, format, width, height, 1, ResourceState::CopyDest, ResourceFlags::None)
{
	uploadMngr.UploadDataTo(data, dataBPT, *this);
	m_SRVHandle = DXDevice::Instance().CreateSRVHandle(m_Resource.get(), format);
}

Texture2D::~Texture2D()
{
	DXDevice::Instance().ReleaseSRVDescriptor(m_SRVHandle);
}
