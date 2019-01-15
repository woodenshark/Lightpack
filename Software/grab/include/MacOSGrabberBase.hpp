/*
 * MacOSGrabber.hpp
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

#pragma once

#include "../common/defs.h"

#if defined(MAC_OS_CG_GRAB_SUPPORT) || defined(MAC_OS_AV_GRAB_SUPPORT)

#include "GrabberBase.hpp"
#include <CoreGraphics/CoreGraphics.h>

class MacOSGrabberBase : public GrabberBase
{
public:
	MacOSGrabberBase(QObject * parent, GrabberContext * grabberContext);
	virtual ~MacOSGrabberBase();
	struct MacOSScreenData
	{
		MacOSScreenData() = default;
		virtual ~MacOSScreenData() = default;
	};
	static double getDisplayScalingRatio(CGDirectDisplayID display);
	static double getDisplayRefreshRate(CGDirectDisplayID display);
protected slots:
	virtual QList< ScreenInfo > * screensWithWidgets(QList< ScreenInfo > * result, const QList<GrabWidget *> &grabWidgets);
	virtual GrabResult grabScreens();
	virtual bool reallocate(const QList<ScreenInfo> &screens);
protected:
	static bool allocateScreenBuffer(const ScreenInfo& screen, GrabbedScreen& grabScreen);
	static bool getScreenInfoFromRect(const CGDirectDisplayID display, const QList<GrabWidget *>& grabWidgets, ScreenInfo& screenInfo);
#ifndef QT_NO_DEBUG
	static void saveGrabbedScreenToBMP(const GrabbedScreen& screen);
#endif // QT_NO_DEBUG
private:
	void freeScreens();
	virtual GrabResult grabDisplay(CGDirectDisplayID display, GrabbedScreen& screen) = 0;
	virtual void freeScreenImageData(GrabbedScreen& screen);
};

#endif // MAC_OS_XX_GRAB_SUPPORT
