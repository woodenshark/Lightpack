/*
 * WinAPIGrabber.cpp
 *
 *	Created on: 25.07.11
 *		Project: Lightpack
 *
 *	Copyright (c) 2011 Timur Sattarov, Mike Shatohin
 *
 *	Lightpack a USB content-driving ambient lighting system
 *
 *	Lightpack is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	Lightpack is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.	If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "WinAPIGrabber.hpp"

#ifdef WINAPI_GRAB_SUPPORT

#include "../src/debug.h"
#include <cmath>
#include "calculations.hpp"
#include "../src/enums.hpp"

struct WinAPIScreenData
{
	WinAPIScreenData()
		: hScreenDC(NULL)
		, hMemDC(NULL)
		, hBitmap(NULL)
	{}
	HDC hScreenDC;
	HDC hMemDC;
	HBITMAP hBitmap;

};

WinAPIGrabber::WinAPIGrabber(QObject * parent, GrabberContext *context)
	: GrabberBase(parent, context)
{
}

WinAPIGrabber::~WinAPIGrabber()
{
	freeScreens();
}

void WinAPIGrabber::freeScreens()
{
	for (int i = 0; i < _screensWithWidgets.size(); ++i) {
		WinAPIScreenData *d = reinterpret_cast<WinAPIScreenData *>(_screensWithWidgets[i].associatedData);
		if (d->hScreenDC)
			DeleteObject(d->hScreenDC);

		if (d->hBitmap)
			DeleteObject(d->hBitmap);

		if (d->hMemDC)
			DeleteObject(d->hMemDC);

		delete d;

		if (_screensWithWidgets[i].imgData != NULL) {
			free((void*)_screensWithWidgets[i].imgData);
			_screensWithWidgets[i].imgData = NULL;
			_screensWithWidgets[i].imgDataSize = 0;
		}
	}

	_screensWithWidgets.clear();
}

QList< ScreenInfo > * WinAPIGrabber::screensWithWidgets(QList< ScreenInfo > * result, const QList<GrabWidget *> &grabWidgets)
{
	result->clear();
	for (int i = 0; i < grabWidgets.size(); ++i) {
		HMONITOR hMonitorNew = MonitorFromWindow(reinterpret_cast<HWND>(grabWidgets[i]->winId()), MONITOR_DEFAULTTONULL);

		if (hMonitorNew != NULL) {
			MONITORINFO monitorInfo;

			ZeroMemory( &monitorInfo, sizeof(MONITORINFO) );
			monitorInfo.cbSize = sizeof(MONITORINFO);

			GetMonitorInfo( hMonitorNew, &monitorInfo );

			LONG left = monitorInfo.rcMonitor.left;
			LONG top = monitorInfo.rcMonitor.top;
			LONG right = monitorInfo.rcMonitor.right;
			LONG bottom = monitorInfo.rcMonitor.bottom;
			ScreenInfo screenInfo;
			screenInfo.rect = QRect(left, top, right - left, bottom - top);
			screenInfo.handle = hMonitorNew;

			if (!result->contains(screenInfo))
				result->append(screenInfo);
		}

	}
	return result;
}

bool WinAPIGrabber::reallocate(const QList< ScreenInfo > &screens)
{
	freeScreens();

	for (int i = 0; i < screens.size(); ++i) {

		const ScreenInfo screen = screens[i];

		long width = screen.rect.width();
		long height = screen.rect.height();

		DEBUG_LOW_LEVEL << "dimensions " << width << "x" << height << screen.handle;

		WinAPIScreenData *d = new WinAPIScreenData();

		d->hScreenDC = CreateDC( TEXT("DISPLAY"), NULL, NULL, NULL );

		// Create a bitmap compatible with the screen DC
		d->hBitmap = CreateCompatibleBitmap( d->hScreenDC, width, height );

		// Create a memory DC compatible to screen DC
		d->hMemDC = CreateCompatibleDC( d->hScreenDC );

		// Select new bitmap into memory DC
		SelectObject( d->hMemDC, d->hBitmap );

		BITMAP bmp;
		memset(&bmp, 0, sizeof(BITMAP));

		// Now get the actual Bitmap
		GetObject( d->hBitmap, sizeof(BITMAP), &bmp );

		// Calculate the size the buffer needs to be
		unsigned pixelsBuffSizeNew = bmp.bmWidthBytes * bmp.bmHeight;

		DEBUG_LOW_LEVEL << Q_FUNC_INFO << "pixelsBuffSize =" << pixelsBuffSizeNew;

		// The amount of bytes per pixel is the amount of bits divided by 8
		bytesPerPixel = bmp.bmBitsPixel / 8;

		if( bytesPerPixel != 4 ){
			qCritical() << "Not 32-bit mode is not supported!" << bytesPerPixel;
		}

		GrabbedScreen grabScreen;
		grabScreen.imgDataSize = pixelsBuffSizeNew;
		grabScreen.imgData = (unsigned char*)malloc(grabScreen.imgDataSize);
		grabScreen.bytesPerRow = bmp.bmWidthBytes;
		grabScreen.imgFormat = BufferFormatArgb;
		grabScreen.screenInfo = screen;
		grabScreen.associatedData = d;
		_screensWithWidgets.append(grabScreen);
	}


	return true;
}

GrabResult WinAPIGrabber::grabScreens()
{
	foreach(GrabbedScreen screen, _screensWithWidgets) {
		QRect screenRect = screen.screenInfo.rect;
		WinAPIScreenData *d = reinterpret_cast<WinAPIScreenData *>(screen.associatedData);

		BitBlt( d->hMemDC, 0, 0, screenRect.width(), screenRect.height(), d->hScreenDC,
				screenRect.left(), screenRect.top(), SRCCOPY );

		// Get the actual RGB data and put it into pbPixelsBuff
		GetBitmapBits( d->hBitmap, (LONG)screen.imgDataSize, (unsigned char*)screen.imgData);
	}
	return GrabResultOk;
}

#endif // WINAPI_GRAB_SUPPORT
