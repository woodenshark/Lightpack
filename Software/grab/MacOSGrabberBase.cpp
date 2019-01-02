/*
 * MacOSGrabber.cpp
 *
 *  Created on: 01.10.11
 *     Project: Lightpack
 *
 *  Copyright (c) 2011 Timur Sattarov, Nikolay Kudashov, Mike Shatohin
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

#include"MacOSGrabberBase.hpp"

#if defined (MAC_OS_CG_GRAB_SUPPORT) || defined(MAC_OS_AV_GRAB_SUPPORT)

#include <ApplicationServices/ApplicationServices.h>
#include "debug.h"

namespace {
	static const uint32_t kMaxDisplaysCount = 10;
}

MacOSGrabberBase::MacOSGrabberBase(QObject *parent, GrabberContext *context):
GrabberBase(parent, context)
{
}

MacOSGrabberBase::~MacOSGrabberBase()
{
	freeScreens();
}

void MacOSGrabberBase::freeScreens()
{
	for (GrabbedScreen& screen : _screensWithWidgets) {
		freeScreenImageData(screen);

		if (screen.associatedData)
			delete (MacOSScreenData*)screen.associatedData;
	}
	_screensWithWidgets.clear();
}

bool MacOSGrabberBase::allocateScreenBuffer(const ScreenInfo& screen,
											GrabbedScreen& grabScreen)
{
	const CGDirectDisplayID screenId = static_cast<CGDirectDisplayID>(reinterpret_cast<intptr_t>(screen.handle));
	const size_t width = CGDisplayPixelsWide(screenId);
	const size_t height = CGDisplayPixelsHigh(screenId);
	
	DEBUG_HIGH_LEVEL << "dimensions " << width << "x" << height << screen.handle;
	grabScreen.imgData = nullptr;
	grabScreen.imgFormat = BufferFormatArgb;
	grabScreen.screenInfo = screen;
	grabScreen.imgDataSize = 0;
	return true;
}

bool MacOSGrabberBase::getScreenInfoFromRect(const CGDirectDisplayID display,
						   const QList<GrabWidget *>& grabWidgets,
						   ScreenInfo& screenInfo)
{
	const CGRect displayRect = CGDisplayBounds(display);
	for (const GrabWidget* grabWidget : grabWidgets) {
		if (CGRectContainsPoint(displayRect, grabWidget->geometry().center().toCGPoint())) {
			const int x1 = displayRect.origin.x;
			const int y1 = displayRect.origin.y;
			const int x2 = displayRect.size.width  + x1 - 1;
			const int y2 = displayRect.size.height + y1 - 1;
			screenInfo.rect.setCoords(x1, y1, x2, y2);
			screenInfo.handle = reinterpret_cast<void *>(display);

			return true;
		}
	}
	
	return false;
}

double MacOSGrabberBase::getDisplayScalingRatio(const CGDirectDisplayID display)
{
	CGDisplayModeRef mode = CGDisplayCopyDisplayMode(display);
	if (mode == NULL)
		return 1.0;
	const double scaledWidth = static_cast<double>(CGDisplayModeGetWidth(mode));
	const double realWidth = static_cast<double>(CGDisplayModeGetPixelWidth(mode));
	CGDisplayModeRelease(mode);
	return (realWidth / scaledWidth);
}

double MacOSGrabberBase::getDisplayRefreshRate(const CGDirectDisplayID display)
{
	CGDisplayModeRef mode = CGDisplayCopyDisplayMode(display);
	if (mode == NULL)
		return 60.0;
	const double hz = CGDisplayModeGetRefreshRate(mode);
	CGDisplayModeRelease(mode);
	return hz;
}

QList<ScreenInfo>* MacOSGrabberBase::screensWithWidgets(
	QList<ScreenInfo>* result,
	const QList<GrabWidget *> &grabWidgets)
{
	CGDirectDisplayID displays[kMaxDisplaysCount];
	uint32_t displayCount = 0;

	CGError err = CGGetActiveDisplayList(kMaxDisplaysCount, displays, &displayCount);

	result->clear();
	if (err == kCGErrorSuccess) {
		for (unsigned int i = 0; i < displayCount; ++i) {
			ScreenInfo screenInfo;
			if (getScreenInfoFromRect(displays[i], grabWidgets, screenInfo))
				result->append(screenInfo);
		}

	} else {
		qCritical() << "couldn't get active displays, error code " << QString::number(err, 16);
	}
	return result;
}

bool MacOSGrabberBase::reallocate(const QList<ScreenInfo> &screens)
{
	freeScreens();
	for (const ScreenInfo& screen : screens) {
		GrabbedScreen grabScreen;
		if (!allocateScreenBuffer(screen, grabScreen)) {
			qCritical() << "couldn't allocate image buffer";
			freeScreens();
			return false;
		}
		_screensWithWidgets.append(grabScreen);

	}
	return true;
}

void MacOSGrabberBase::freeScreenImageData(GrabbedScreen& screen)
{
	screen.imgData = nullptr;
	if (screen.associatedData)
		delete ((MacOSScreenData*)screen.associatedData);
	screen.associatedData = nullptr;
}

GrabResult MacOSGrabberBase::grabScreens()
{
	for (GrabbedScreen& grabScreen : _screensWithWidgets)
	{
		freeScreenImageData(grabScreen);
		CGDirectDisplayID display = static_cast<CGDirectDisplayID>(reinterpret_cast<intptr_t>(grabScreen.screenInfo.handle));
		
		const GrabResult result = grabDisplay(display, grabScreen);

		if (result == GrabResultError) {
			qCritical() << Q_FUNC_INFO << "grabDisplay failed";
			return GrabResultError;
		} else if (result == GrabResultFrameNotReady)
			return GrabResultFrameNotReady;
	}
	return GrabResultOk;
}

#endif // MAC_OS_XX_GRAB_SUPPORT
