/*
* DDGrabber.cpp
*
*  Created on: 21.12.2018
*     Project: Lightpack
*
*  Copyright (c) 2014 Patrick Siegler
*
*  Lightpack a USB content-driving ambient lighting system
*
*  Lightpack is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 2 of the License, or
*  (at your option) any later version.
*
*  Lightpack is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#include "DDuplGrabber.hpp"

#ifdef DDUPL_GRAB_SUPPORT
#include <comdef.h>
#include <dxgi1_2.h>
#include <d3d11.h>
_COM_SMARTPTR_TYPEDEF(IDXGIFactory1, __uuidof(IDXGIFactory1));
_COM_SMARTPTR_TYPEDEF(IDXGIOutput, __uuidof(IDXGIOutput));
_COM_SMARTPTR_TYPEDEF(IDXGIOutput1, __uuidof(IDXGIOutput1));
_COM_SMARTPTR_TYPEDEF(IDXGIOutputDuplication, __uuidof(IDXGIOutputDuplication));
_COM_SMARTPTR_TYPEDEF(IDXGIResource, __uuidof(IDXGIResource));
_COM_SMARTPTR_TYPEDEF(IDXGISurface1, __uuidof(IDXGISurface1));
_COM_SMARTPTR_TYPEDEF(ID3D11Device, __uuidof(ID3D11Device));
_COM_SMARTPTR_TYPEDEF(ID3D11DeviceContext, __uuidof(ID3D11DeviceContext));
_COM_SMARTPTR_TYPEDEF(ID3D11Texture2D, __uuidof(ID3D11Texture2D));

#define ACQUIRE_TIMEOUT_INTERVAL 50

struct DDuplScreenData
{
	DDuplScreenData(IDXGIOutputPtr _output, IDXGIOutputDuplicationPtr _duplication, ID3D11DevicePtr _device, ID3D11DeviceContextPtr _context)
		: output(_output), duplication(_duplication), device(_device), context(_context)
	{}

	IDXGIOutputPtr output;
	IDXGIOutputDuplicationPtr duplication;
	ID3D11DevicePtr device;
	ID3D11DeviceContextPtr context;
};

DDuplGrabber::DDuplGrabber(QObject * parent, GrabberContext *context)
	: GrabberBase(parent, context),
	m_initialized(false),
	m_lostAccess(false)
{
}

DDuplGrabber::~DDuplGrabber()
{
}

void DDuplGrabber::init()
{
	IDXGIFactory1Ptr factory;
	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory);
	if (FAILED(hr))
	{
		qCritical(Q_FUNC_INFO " Failed to CreateDXGIFactory1: 0x%X", hr);
		return;
	}

	IDXGIAdapter1Ptr adapter;
	for (int i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++)
	{
		m_adapters.push_back(adapter);
	}

	m_initialized = true;
}

bool anyWidgetOnThisMonitor(HMONITOR monitor, const QList<GrabWidget *> &grabWidgets)
{
	for (GrabWidget* widget : grabWidgets)
	{
		HMONITOR widgetMonitor = MonitorFromWindow(reinterpret_cast<HWND>(widget->winId()), MONITOR_DEFAULTTONULL);
		if (widgetMonitor == monitor)
		{
			return true;
		}
	}

	return false;
}

QList< ScreenInfo > * DDuplGrabber::screensWithWidgets(QList< ScreenInfo > * result, const QList<GrabWidget *> &grabWidgets)
{
	if (!m_initialized)
	{
		init();
	}

	result->clear();

	for (IDXGIAdapter1Ptr adapter : m_adapters)
	{
		IDXGIOutputPtr output;
		for (int i = 0; adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND; i++)
		{
			DXGI_OUTPUT_DESC outputDesc;
			output->GetDesc(&outputDesc);
			if (anyWidgetOnThisMonitor(outputDesc.Monitor, grabWidgets))
			{
				ScreenInfo screenInfo;
				screenInfo.rect = QRect(
					outputDesc.DesktopCoordinates.left,
					outputDesc.DesktopCoordinates.top,
					outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left,
					outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top);
				screenInfo.handle = outputDesc.Monitor;

				result->append(screenInfo);
			}
		}
	}
	return result;
}

bool DDuplGrabber::isReallocationNeeded(const QList< ScreenInfo > &grabScreens) const
{
	return GrabberBase::isReallocationNeeded(grabScreens) || m_lostAccess;
}

void DDuplGrabber::freeScreens()
{
	for (GrabbedScreen& screen : _screensWithWidgets)
	{
		if (screen.associatedData != NULL)
		{
			delete (DDuplScreenData*)screen.associatedData;
			screen.associatedData = NULL;
		}

		if (screen.imgData != NULL)
		{
			free(screen.imgData);
			screen.imgData = NULL;
			screen.imgDataSize = 0;
		}
	}
}

bool DDuplGrabber::reallocate(const QList< ScreenInfo > &grabScreens)
{
	if (!m_initialized)
	{
		init();
	}

	freeScreens();

	for (IDXGIAdapter1Ptr adapter : m_adapters)
	{
		ID3D11DevicePtr device;
		ID3D11DeviceContextPtr context;
		D3D_FEATURE_LEVEL featureLevel;
		HRESULT hr = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &device, &featureLevel, &context);
		if (FAILED(hr))
		{
			qCritical(Q_FUNC_INFO " Failed to create D3D11 device: 0x%X", hr);
			return false;
		}

		IDXGIOutputPtr output;
		for (int i = 0; adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND; i++)
		{
			IDXGIOutput1Ptr output1;
			hr = output->QueryInterface(IID_IDXGIOutput1, (void**)&output1);
			if (FAILED(hr))
			{
				qCritical(Q_FUNC_INFO " Failed to cast output to IDXGIOutput1: 0x%X", hr);
				return false;
			}

			DXGI_OUTPUT_DESC outputDesc;
			output->GetDesc(&outputDesc);
			for (const ScreenInfo& screenInfo : grabScreens)
			{
				if (screenInfo.handle == outputDesc.Monitor)
				{
					IDXGIOutputDuplicationPtr duplication;
					hr = output1->DuplicateOutput(device, &duplication);
					if (FAILED(hr))
					{
						qCritical(Q_FUNC_INFO " Failed to duplicate output: 0x%X", hr);
						return false;
					}

					GrabbedScreen grabScreen;
					grabScreen.imgData = (unsigned char *)NULL;
					grabScreen.imgFormat = BufferFormatArgb;
					grabScreen.screenInfo = screenInfo;
					grabScreen.associatedData = new DDuplScreenData(output, duplication, device, context);

					_screensWithWidgets.append(grabScreen);

					break;
				}
			}

		}
	}

	m_lostAccess = false;

	return true;
}

BufferFormat mapDXGIFormatToBufferFormat(DXGI_FORMAT format)
{
	switch (format)
	{
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			return BufferFormatArgb;
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			return BufferFormatAbgr;
		default:
			return BufferFormatUnknown;
	}
}

GrabResult DDuplGrabber::grabScreens()
{
	try
	{
		for (GrabbedScreen& screen : _screensWithWidgets)
		{
			if (screen.associatedData == NULL)
			{
				return GrabResultError;
			}

			DDuplScreenData* screenData = (DDuplScreenData*)screen.associatedData;
			DXGI_OUTDUPL_FRAME_INFO frameInfo;
			IDXGIResourcePtr resource;
			HRESULT hr = screenData->duplication->AcquireNextFrame(ACQUIRE_TIMEOUT_INTERVAL, &frameInfo, &resource);
			if (hr == DXGI_ERROR_WAIT_TIMEOUT)
			{
				return GrabResultFrameNotReady;
			}
			else if (hr == DXGI_ERROR_ACCESS_LOST)
			{
				m_lostAccess = true;
				qWarning(Q_FUNC_INFO " Lost Access to desktop 0x%X, requesting realloc", screen.screenInfo.handle);
				return GrabResultFrameNotReady;
			}
			else if (FAILED(hr))
			{
				qCritical(Q_FUNC_INFO " Failed to AcquireNextFrame: 0x%X", hr);
				return GrabResultError;
			}

			ID3D11Texture2DPtr texture;
			hr = resource->QueryInterface(IID_ID3D11Texture2D, (void**)&texture);
			if (FAILED(hr))
			{
				qCritical(Q_FUNC_INFO " Failed to cast resource to ID3D11Texture2D: 0x%X", hr);
				return GrabResultError;
			}

			D3D11_TEXTURE2D_DESC desc;
			texture->GetDesc(&desc);

			if (desc.Width != screen.screenInfo.rect.width() || desc.Height != screen.screenInfo.rect.height())
			{
				qCritical(Q_FUNC_INFO " Dimension mismatch: screen %d x %d, texture %d x %d",
					screen.screenInfo.rect.width(),
					screen.screenInfo.rect.height(),
					desc.Width,
					desc.Height);
				return GrabResultError;
			}

			size_t sizeNeeded = desc.Height * desc.Width * 4; // Assumes 4 bytes per psxel
			if (screen.imgData == NULL)
			{
				screen.imgData = (unsigned char*)malloc(sizeNeeded);
				screen.imgDataSize = sizeNeeded;
			}
			else if (screen.imgDataSize != sizeNeeded)
			{
				qCritical(Q_FUNC_INFO " Unexpected buffer size %d where %d is expected", screen.imgDataSize, sizeNeeded);
				return GrabResultError;
			}

			D3D11_TEXTURE2D_DESC texDesc;
			ZeroMemory(&texDesc, sizeof(texDesc));
			texDesc.Width = desc.Width;
			texDesc.Height = desc.Height;
			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			texDesc.Usage = D3D11_USAGE_STAGING;
			texDesc.Format = desc.Format;
			texDesc.BindFlags = 0;
			texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			texDesc.MiscFlags = 0;
			ID3D11Texture2DPtr textureCopy;
			hr = screenData->device->CreateTexture2D(&texDesc, NULL, &textureCopy);
			if (FAILED(hr))
			{
				qCritical(Q_FUNC_INFO " Failed to CreateTexture2D: 0x%X", hr);
				return GrabResultError;
			}

			screenData->context->CopyResource(textureCopy, texture);

			IDXGISurface1Ptr surface;
			hr = textureCopy->QueryInterface(IID_IDXGISurface1, (void**)&surface);
			if (FAILED(hr))
			{
				qCritical(Q_FUNC_INFO " Failed to cast textureCopy to IID_IDXGISurface1: 0x%X", hr);
				return GrabResultError;
			}

			DXGI_MAPPED_RECT map;
			hr = surface->Map(&map, DXGI_MAP_READ);
			if (FAILED(hr))
			{
				qCritical(Q_FUNC_INFO " Failed to get surface map: 0x%X", hr);
				return GrabResultError;
			}

			for (unsigned int i = 0; i < desc.Height; i++)
			{
				memcpy_s(screen.imgData + (i * desc.Width) * 4, desc.Width * 4, map.pBits + i*map.Pitch, desc.Width * 4);
			}

			screen.imgFormat = mapDXGIFormatToBufferFormat(desc.Format);

			screenData->duplication->ReleaseFrame();
		}
	}
	catch (_com_error e)
	{
		qCritical(Q_FUNC_INFO " COM Error: 0x%X", e.Error());
		return GrabResultError;
	}

	return GrabResultOk;
}

#endif