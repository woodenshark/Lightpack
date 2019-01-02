//
//  MacOSAVGrabber.cpp
//  grab
//
//  Created by zomfg on 11/12/2018.
//

#include "MacOSAVGrabber.h"

#ifdef MAC_OS_AV_GRAB_SUPPORT

#include "debug.h"
#include "GrabberContext.hpp"
#include "GrabWidget.hpp"
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <AVFoundation/AVFoundation.h>

namespace {
	static const double kScaleFactor = 0.05;
}

@interface MacOSNativeAVCapture : NSDocument <AVCaptureVideoDataOutputSampleBufferDelegate>

@property(nonatomic, retain) __attribute__((NSObject)) CVPixelBufferRef currentPixelBuffer;
@property(nonatomic, retain) NSError* captureError;

@end

@implementation MacOSNativeAVCapture {
    CGDirectDisplayID		_display;
	
	AVCaptureSession*		_captureSession;
	AVCaptureScreenInput*	_captureScreenInput; // setting scale + framerate
	BOOL _running;
	
	dispatch_queue_t	_sessionQueue;
	dispatch_queue_t	_videoDataOutputQueue;
}

- (instancetype) initWithDisplay:(CGDirectDisplayID)display
{
    self = [super init];
    if (self)
    {
		_running = NO;
        _display = display;
		
		_sessionQueue = dispatch_queue_create("com.prismatik.avcapture.session", DISPATCH_QUEUE_SERIAL);
		_videoDataOutputQueue = dispatch_queue_create("com.prismatik.avcapture.videodata", DISPATCH_QUEUE_SERIAL);
//		dispatch_set_target_queue(_videoDataOutputQueue, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0));
    }
    return self;
}

- (void) dealloc
{
	CVPixelBufferRelease(_currentPixelBuffer);
	[self destroyCaptureSession];
	
	dispatch_release(_sessionQueue);
	dispatch_release(_videoDataOutputQueue);
	
    [super dealloc];
}

- (BOOL) createCaptureSession
{
	if (_captureSession)
		return YES;
    /* Create a capture session. */
    _captureSession = [[AVCaptureSession alloc] init];
    if ([_captureSession canSetSessionPreset:AVCaptureSessionPreset320x240])
        [_captureSession setSessionPreset:AVCaptureSessionPreset320x240];
    
    /* Add the display as a capture input. */
    AVCaptureScreenInput* captureScreenInput = [[AVCaptureScreenInput alloc] initWithDisplayID:_display];
	captureScreenInput.scaleFactor = kScaleFactor / MacOSAVGrabber::getDisplayScalingRatio(_display);
	captureScreenInput.minFrameDuration = CMTimeMake(1, 1); // default to 1 FPS
    if ([_captureSession canAddInput:captureScreenInput])
        [_captureSession addInput:captureScreenInput];
	else {
		[_captureSession release];
		_captureSession = nil;
		[captureScreenInput release];
        return NO;
	}
	_captureScreenInput = captureScreenInput;
	[captureScreenInput release];

    /* Add a movie file output + delegate. */
    AVCaptureVideoDataOutput* captureDataOutput = [[AVCaptureVideoDataOutput alloc] init];
    [captureDataOutput setSampleBufferDelegate:self queue:_videoDataOutputQueue];
	captureDataOutput.alwaysDiscardsLateVideoFrames = YES;
	captureDataOutput.videoSettings = @{ (id)kCVPixelBufferPixelFormatTypeKey : @(kCVPixelFormatType_32BGRA) };
    if ([_captureSession canAddOutput:captureDataOutput])
        [_captureSession addOutput:captureDataOutput];
	else {
		[_captureSession release];
		_captureSession = nil;
		_captureScreenInput = nil;
		[captureDataOutput release];
        return NO;
	}
	[captureDataOutput release];
	
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(captureSessionNotification:) name:nil object:_captureSession];
	
    return YES;
}

- (void) destroyCaptureSession
{
	if (_captureSession) {
		[[NSNotificationCenter defaultCenter] removeObserver:self];
		[_captureSession release];
	}
	_captureSession = nil;
}

- (void) captureSessionNotification:(NSNotification *)notification
{
	dispatch_async(_sessionQueue, ^{

#if __MAC_OS_X_VERSION_MAX_ALLOWED >= __MAC_10_14
		if (@available(macOS 10.14, *)) {
			if (notification.name == AVCaptureSessionWasInterruptedNotification)
			{
				DEBUG_LOW_LEVEL << Q_FUNC_INFO << "session interrupted";
				[self captureSessionDidStopRunning];
				return;
			}
			if (notification.name == AVCaptureSessionInterruptionEndedNotification)
			{
				DEBUG_LOW_LEVEL << Q_FUNC_INFO << "session interruption ended";
				return;
			}
		}
#endif // __MAC_10_14

		if (notification.name == AVCaptureSessionDidStartRunningNotification)
		{
			DEBUG_LOW_LEVEL << Q_FUNC_INFO << "session started running";
		}
		else if (notification.name == AVCaptureSessionDidStopRunningNotification)
		{
			DEBUG_LOW_LEVEL << Q_FUNC_INFO << "session stopped running";
		}
		else if (notification.name == AVCaptureSessionRuntimeErrorNotification)
		{
			NSError *error = notification.userInfo[AVCaptureSessionErrorKey];
			qWarning() << Q_FUNC_INFO << "session runtime error: " << error.localizedDescription.UTF8String;
			[self captureSessionDidStopRunning];
			//	AVErrorDisplayWasDisabled
			//	AVErrorScreenCaptureFailed
			//	AVErrorFormatUnsupported
			//	AVErrorNoCompatibleAlternatesForExternalDisplay
			//	AVErrorSessionConfigurationChanged
			//	AVErrorSessionNotRunning
			//	AVErrorSessionWasInterrupted
			//	AVErrorUnknown
			//	AVErrorUnsupportedOutputSettings
			switch (error.code) {
				case AVErrorDisplayWasDisabled:
				case AVErrorSessionNotRunning:
					[self handleRecoverableCaptureSessionRuntimeError:error];
					break;
				default:
					[self handleNonRecoverableCaptureSessionRuntimeError:error];
					break;
			}
		}

	});
}

- (void) handleRecoverableCaptureSessionRuntimeError:(NSError *)error
{
	Q_UNUSED(error);
	if (_running)
		[_captureSession startRunning];
}

- (void) handleNonRecoverableCaptureSessionRuntimeError:(NSError *)error
{
	_running = NO;
	[self destroyCaptureSession];
	
	@synchronized(self) {
		self.captureError = error;
	}
}

- (void) captureSessionDidStopRunning
{
	[self destroyVideoPipeline];
}

// synchronous, blocks until the pipeline is drained, don't call from within the pipeline
- (void) destroyVideoPipeline
{
	// The session is stopped so we are guaranteed that no new buffers are coming through the video data output.
	// There may be inflight buffers on _videoDataOutputQueue however.
	// Synchronize with that queue to guarantee no more buffers are in flight.
	// Once the pipeline is drained we can tear it down safely.
	dispatch_sync(_videoDataOutputQueue, ^{
		self.currentPixelBuffer = NULL;
	});
}

- (bool) isRunning
{
	BOOL r = NO;
	@synchronized (self) {
		r = _running;
	}
	return r;
}

- (void) start
{
	dispatch_sync(_sessionQueue, ^{
		[_captureSession startRunning];
		_running = YES;
	});
}

- (void) stop
{
	dispatch_sync(_sessionQueue, ^{
		[_captureSession stopRunning];
		_running = NO;
	});
}

/* Set the screen input maximum frame rate. */
- (void) setMaximumScreenInputFramerate:(float)maximumFramerate
{
    CMTime minimumFrameDuration = CMTimeMake(1, (int32_t)maximumFramerate);
    /* Set the screen input's minimum frame duration. */
	@synchronized (self) {
		dispatch_async(_sessionQueue, ^{[_captureScreenInput setMinFrameDuration:minimumFrameDuration];});
	}
}

- (double) maximumPossibleFramerate
{
	return MacOSAVGrabber::getDisplayRefreshRate(_display);
}

- (void) captureOutput:(AVCaptureOutput *)output didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection
{
    Q_UNUSED(output);
    Q_UNUSED(connection);
	@synchronized (self) {
		self.currentPixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
	}
}

- (CVPixelBufferRef) getLastPixelBuffer:(NSError **)error
{
	CVPixelBufferRef buffer = NULL;
	@synchronized (self) {
		buffer = CVPixelBufferRetain(self.currentPixelBuffer);
		self.currentPixelBuffer = NULL;
		if (error)
			*error = self.captureError;
	}
	return buffer;
}

@end

#pragma mark - MacOSAVScreenData

namespace {
	struct MacOSAVScreenData : public MacOSGrabberBase::MacOSScreenData
    {
        MacOSAVScreenData() = default;
        ~MacOSAVScreenData()
        {
			setImageRef(nullptr);
        }
        
        void setImageRef(CVPixelBufferRef _displayImageRef)
        {
            if (_displayImageRef == displayImageRef)
                return;
			if (displayImageRef) {
				CVPixelBufferUnlockBaseAddress(displayImageRef, kCVPixelBufferLock_ReadOnly);
				CVPixelBufferRelease(displayImageRef);
			}
            displayImageRef = CVPixelBufferRetain(_displayImageRef);
			if (displayImageRef)
				CVPixelBufferLockBaseAddress(displayImageRef, kCVPixelBufferLock_ReadOnly);
        }
        
		CVPixelBufferRef displayImageRef{nullptr};
    };
    
    void toGrabbedScreen(CVPixelBufferRef imageRef, GrabbedScreen& screen)
    {
        if (!screen.associatedData)
            screen.associatedData = new MacOSAVScreenData;
        ((MacOSAVScreenData*)screen.associatedData)->setImageRef(imageRef);
        screen.bytesPerRow = CVPixelBufferGetBytesPerRow(imageRef);
		screen.scale = kScaleFactor;
        screen.imgData = (unsigned char*)CVPixelBufferGetBaseAddress(imageRef);
        screen.imgDataSize = CVPixelBufferGetHeight(imageRef) * screen.bytesPerRow;
    }
}


#pragma mark - MacOSAVGrabber

MacOSAVGrabber::MacOSAVGrabber(QObject *parent, GrabberContext *context):
MacOSGrabberBase(parent, context)
{
}

MacOSAVGrabber::~MacOSAVGrabber()
{
	foreach (MacOSNativeAVCapture* capture, _captures)
	{
		[capture stop];
		[capture release];
	}
	_captures.clear();
}

void MacOSAVGrabber::setGrabInterval(int msec)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO <<    this->metaObject()->className();
    m_timer->setInterval(msec);

	double framerate = 1000.0 / msec;
    foreach (MacOSNativeAVCapture* capture, _captures)
	{
		double maxCaptureFramerate = [capture maximumPossibleFramerate];
		[capture setMaximumScreenInputFramerate:MIN(framerate, maxCaptureFramerate == 0.0 ? framerate : maxCaptureFramerate)];
	}
}

void MacOSAVGrabber::startGrabbing()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << this->metaObject()->className();
    grabScreensCount = 0;

	if (_screensWithWidgets.empty())
	{
		QList<ScreenInfo> screens2Grab;
		screensWithWidgets(&screens2Grab, *_context->grabWidgets);
		reallocate(screens2Grab);
	}

	foreach (MacOSNativeAVCapture* capture, _captures)
    {
        [capture stop];
        [capture release];
    }
	_captures.clear();
	foreach (const GrabbedScreen& grabScreen, _screensWithWidgets)
	{
        CGDirectDisplayID display = static_cast<CGDirectDisplayID>(reinterpret_cast<intptr_t>(grabScreen.screenInfo.handle));
        MacOSNativeAVCapture* capture = [[MacOSNativeAVCapture alloc] initWithDisplay:display];
		if ([capture createCaptureSession]) {
			[capture setMaximumScreenInputFramerate:(m_timer->interval() > 0 ? 1000.0f / m_timer->interval() : 1.0f)];
			_captures.insert(display, capture);
			[capture start];
		}
    }

	m_timer->start();
}

void MacOSAVGrabber::stopGrabbing()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << this->metaObject()->className();
    DEBUG_MID_LEVEL << "grabbed" << grabScreensCount << "frames";
    m_timer->stop();
	
    foreach (MacOSNativeAVCapture* capture, _captures) {
        [capture stop];
		[capture release];
    }
	_captures.clear();
}

GrabResult MacOSAVGrabber::grabDisplay(const CGDirectDisplayID display, GrabbedScreen& screen)
{
	MacOSNativeAVCapture* capture = _captures.value(display, nullptr);
	if (capture == nullptr)
	{
		qCritical() << Q_FUNC_INFO << "No capture found for display";
		return GrabResultError;
	}

	NSError* error = nil;
	CVPixelBufferRef imageRef = [capture getLastPixelBuffer:&error];
	
	if (!imageRef) {
		if (error) {
			qCritical() << Q_FUNC_INFO << "[capture getLastPixelBuffer:] returned error: " << error.localizedDescription.UTF8String;
			return GrabResultError;
		}
		return GrabResultFrameNotReady;
	}

	toGrabbedScreen(imageRef, screen);
	CVPixelBufferRelease(imageRef);

    return GrabResultOk;
}

#endif // MAC_OS_AV_GRAB_SUPPORT
