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

#include <QGuiApplication>
#include"MacOSGrabber.hpp"

#ifdef MAC_OS_CG_GRAB_SUPPORT

#include <CoreGraphics/CGImage.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ApplicationServices/ApplicationServices.h>
#include "calculations.hpp"
#include "debug.h"

namespace {

static const size_t kBytesPerPixel = 4;
static const uint32_t kMaxDisplaysCount = 10;

bool allocateScreenBuffer(const ScreenInfo& screen,
                          const qreal pixelRatio,
                          GrabbedScreen& grabScreen)
{
    CGDirectDisplayID screenId = reinterpret_cast<intptr_t>(screen.handle);
    const size_t width = CGDisplayPixelsWide(screenId) * pixelRatio;
    const size_t height = CGDisplayPixelsHigh(screenId) * pixelRatio;
    const size_t imgSize = height * width * kBytesPerPixel;

    DEBUG_HIGH_LEVEL << "dimensions " << width << "x" << height << pixelRatio << screen.handle;
    grabScreen.imgData = nullptr;
    grabScreen.imgFormat = BufferFormatArgb;
    grabScreen.screenInfo = screen;
    grabScreen.imgDataSize = imgSize;
    return true;
}

bool getScreenInfoFromRect(const CGDirectDisplayID display,
                           const QList<GrabWidget *>& grabWidgets,
                           ScreenInfo& screenInfo)
{
    const CGRect displayRect = CGDisplayBounds(display);
    for (int k = 0; k < grabWidgets.size(); ++k) {
        const QRect& rect = grabWidgets[k]->geometry();
        const CGPoint widgetCenter = CGPointMake(rect.x() + rect.width() / 2,
                                                 rect.y() + rect.height() / 2);
        if (CGRectContainsPoint(displayRect, widgetCenter)) {
            const int x1 = displayRect.origin.x;
            const int y1 = displayRect.origin.y;
            const int x2 = displayRect.size.width  + x1 - 1;
            const int y2 = displayRect.size.height + y1 - 1;
            screenInfo.rect.setCoords( x1, y1, x2, y2);
            screenInfo.handle = reinterpret_cast<void *>(display);

            return true;
        }
    }

    return false;
}

void freeScreenImageData(GrabbedScreen& screen)
{
    screen.imgData = nullptr;
    if (screen.imageDataRef) {
        CFRelease(screen.imageDataRef);
        screen.imageDataRef = nullptr;
    }

    if (screen.displayImageRef) {
        CGImageRelease(screen.displayImageRef);
        screen.displayImageRef = nullptr;
    }
}

void toGrabbedScreen(CGImageRef imageRef, GrabbedScreen *screen)
{
    const size_t width = CGImageGetWidth(imageRef);
    const size_t height = CGImageGetHeight(imageRef);
    const size_t screenBufferSize = width * height * kBytesPerPixel;
    Q_ASSERT(screen->imgDataSize == screenBufferSize);

    screen->displayImageRef = imageRef;
    CGDataProviderRef provider = CGImageGetDataProvider(imageRef);
    screen->imageDataRef = CGDataProviderCopyData(provider);
    screen->imgData = CFDataGetBytePtr(screen->imageDataRef);
}

}

MacOSGrabber::MacOSGrabber(QObject *parent, GrabberContext *context):
    GrabberBase(parent, context)
{
}

MacOSGrabber::~MacOSGrabber()
{
    freeScreens();
}

void MacOSGrabber::freeScreens()
{
    for (int i = 0; i < _screensWithWidgets.size(); ++i) {
        auto& screen = _screensWithWidgets[i];
        freeScreenImageData(screen);

        if (screen.associatedData)
            free(screen.associatedData);
    }
    _screensWithWidgets.clear();
}

QList< ScreenInfo > * MacOSGrabber::screensWithWidgets(
    QList< ScreenInfo > * result,
    const QList<GrabWidget *> &grabWidgets)
{
    CGDirectDisplayID displays[kMaxDisplaysCount];
    uint32_t displayCount;

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

bool MacOSGrabber::reallocate(const QList<ScreenInfo> &screens)
{
    const qreal pixelRatio = ((QGuiApplication*)QCoreApplication::instance())->devicePixelRatio();
    freeScreens();
    for (int i = 0; i < screens.size(); ++i) {
        GrabbedScreen grabScreen;
        if (!allocateScreenBuffer(screens[i], pixelRatio, grabScreen)) {
            qCritical() << "couldn't allocate image buffer";
            freeScreens();
            return false;
        }
        _screensWithWidgets.append(grabScreen);

    }
    return true;
}

GrabResult MacOSGrabber::grabScreens()
{
    for (int i = 0; i < _screensWithWidgets.size(); ++i) {
        auto& grabScreen = _screensWithWidgets[i];
        freeScreenImageData(grabScreen);
        CGDirectDisplayID display = reinterpret_cast<intptr_t>(grabScreen.screenInfo.handle);
        CGImageRef imageRef = CGDisplayCreateImage(display);

        if (!imageRef) {
            qCritical() << Q_FUNC_INFO << "CGDisplayCreateImage(..) returned NULL";
            return GrabResultError;
        }

        toGrabbedScreen(imageRef, &grabScreen);
    }
    return GrabResultOk;
}

#endif // MAC_OS_CG_GRAB_SUPPORT
