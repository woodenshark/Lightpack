/*
 * MacOSCGGrabber.cpp
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

#include"MacOSCGGrabber.hpp"

#ifdef MAC_OS_CG_GRAB_SUPPORT

#include "debug.h"

namespace {

	struct MacOSCGScreenData : public MacOSGrabberBase::MacOSScreenData
	{
		MacOSCGScreenData() = default;
		~MacOSCGScreenData()
		{
			setImageDataRefs(nullptr, nullptr);
		}
		
		void setImageDataRefs(const CFDataRef dataRef, const CGImageRef imageRef)
		{
			if (imageDataRef)
				CFRelease(imageDataRef);
			imageDataRef = dataRef;

			if (displayImageRef)
				CGImageRelease(displayImageRef);
			displayImageRef = imageRef;
		}

		CGImageRef displayImageRef{nullptr};
		CFDataRef imageDataRef{nullptr};
	};

	void toGrabbedScreen(const CGImageRef imageRef, GrabbedScreen& screen, const CGDirectDisplayID display)
	{
		CGDataProviderRef provider = CGImageGetDataProvider(imageRef);

		if (!screen.associatedData)
			screen.associatedData = new MacOSCGScreenData();
		((MacOSCGScreenData*)screen.associatedData)->setImageDataRefs(CGDataProviderCopyData(provider), imageRef);
		// get pixel ratio of the actual display being grabbed
		screen.scale = MacOSCGGrabber::getDisplayScalingRatio(display);
		screen.bytesPerRow = CGImageGetBytesPerRow(imageRef);
		screen.imgDataSize = screen.bytesPerRow * CGImageGetHeight(imageRef);
		screen.imgData = CFDataGetBytePtr(((MacOSCGScreenData*)screen.associatedData)->imageDataRef);
	}
}

MacOSCGGrabber::MacOSCGGrabber(QObject *parent, GrabberContext *context):
	MacOSGrabberBase(parent, context)
{
}

GrabResult MacOSCGGrabber::grabDisplay(const CGDirectDisplayID display, GrabbedScreen& screen)
{
	const CGImageRef imageRef = CGDisplayCreateImage(display);

	if (!imageRef) {
		qCritical() << Q_FUNC_INFO << "CGDisplayCreateImage(..) returned NULL";
		return GrabResultError;
	}

	toGrabbedScreen(imageRef, screen, display);

	return GrabResultOk;
}

#endif // MAC_OS_CG_GRAB_SUPPORT
