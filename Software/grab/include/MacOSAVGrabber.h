//
//  MacOSAVGrabber.h
//  grab
//
//  Created by zomfg on 18/12/2018.
//
#pragma once

#include "../common/defs.h"

#ifdef MAC_OS_AV_GRAB_SUPPORT

#include "MacOSGrabberBase.hpp"
#include <QMap>
// http://philjordan.eu/article/mixing-objective-c-c++-and-objective-c++
#include <objc/objc-runtime.h>
#ifdef __OBJC__
@class MacOSNativeAVCapture;
#else
typedef struct objc_object MacOSNativeAVCapture;
#endif

class MacOSAVGrabber : public MacOSGrabberBase
{
public:
	MacOSAVGrabber(QObject *parent, GrabberContext *context);
	virtual ~MacOSAVGrabber();

	DECLARE_GRABBER_NAME("MacOSAVGrabber")

	void setGrabInterval(int msec);
	void startGrabbing();
	void stopGrabbing();
    
protected slots:
	virtual GrabResult grabDisplay(const CGDirectDisplayID display, GrabbedScreen& screen);
private:
	QMap<CGDirectDisplayID, MacOSNativeAVCapture*> _captures;
};

#endif /* MAC_OS_AV_GRAB_SUPPORT */
