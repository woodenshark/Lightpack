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
	m_initialized(false)
{
}

DDuplGrabber::~DDuplGrabber()
{
}


GrabResult DDuplGrabber::grabScreens()
{
	//_screensWithWidgets.
	return GrabResultError;
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
			qCritical("Failed to create D3D11 device: 0x%X", hr);
			return false;
		}

		IDXGIOutputPtr output;
		for (int i = 0; adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND; i++)
		{
			IDXGIOutput1Ptr output1;
			hr = output->QueryInterface(IID_IDXGIOutput1, (void**)&output1);
			if (FAILED(hr))
			{
				qCritical("Failed to cast output to IDXGIOutput1: 0x%X", hr);
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
						qCritical("Failed to duplicate output: 0x%X", hr);
						return false;
					}

					GrabbedScreen grabScreen;
					grabScreen.imgData = (unsigned char *)nullptr;
					grabScreen.imgFormat = BufferFormatArgb;
					grabScreen.screenInfo = screenInfo;
					grabScreen.associatedData = new DDuplScreenData(output, duplication, device, context);

					_screensWithWidgets.append(grabScreen);

					break;
				}
			}

		}
	}

	return true;
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


void DDuplGrabber::freeScreens()
{
	for (GrabbedScreen& screen : _screensWithWidgets)
	{
		screen.associatedData = nullptr;

		if (screen.imgData != NULL) {
			free(screen.imgData);
			screen.imgData = NULL;
			screen.imgDataSize = 0;
		}
	}
}

void DDuplGrabber::init()
{
	IDXGIFactory1Ptr factory;
	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory);
	if (FAILED(hr))
	{
		qCritical("Failed to CreateDXGIFactory1: 0x%X", hr);
		return;
	}

	IDXGIAdapter1Ptr adapter;
	for (int i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++)
	{
		m_adapters.push_back(adapter);
	}

	m_initialized = true;
}

#endif