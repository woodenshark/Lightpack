//
//  MacOSSoundManager.mm
//  Prismatik
//
//  Created by zomfg on 07/01/2019.
//

#include "MacOSSoundManager.h"
#include "PrismatikMath.hpp"
#include "debug.h"
#include <Accelerate/Accelerate.h>
#include <Foundation/Foundation.h>
#include <AVFoundation/AVFoundation.h>
#include <CoreMedia/CoreMedia.h>

namespace {
	template <typename T>
	inline void floatcpy(const char* src, const uint8_t stride, float* dest, const size_t len) {
		if (stride == 1) {
			memcpy(dest, src, len * sizeof(*dest));
			return;
		}
		const T* ptr = reinterpret_cast<const T*>(src);
		for (const T* const end = ptr + len * stride; ptr < end; ptr += stride, ++dest)
			*dest = *ptr;
	}
}

@interface AVCaptureDeviceFormat (CompatiblityCheck)
- (size_t) score;
- (BOOL) isCompatible;
@end

@implementation AVCaptureDeviceFormat (CompatiblityCheck)

- (size_t) score
{
	size_t score = 0;

	if (![self.mediaType isEqualToString:AVMediaTypeAudio])
		return 0;
	const AudioStreamBasicDescription *description = CMAudioFormatDescriptionGetStreamBasicDescription(self.formatDescription);
	if (description == NULL)
		return 0;
	if (description->mFormatID != kAudioFormatLinearPCM)
		return 0;
	
	if (description->mBitsPerChannel == 16)
		score += 3;
	else if (description->mBitsPerChannel == 24)
		score += 1;
	else if (description->mBitsPerChannel == 32)
		score += 1;
	
	if (description->mSampleRate == 44100)
		score += 3;
	else if (description->mSampleRate == 48000)
		score += 1;
	else if (description->mSampleRate == 32000)
		score += 1;

	return score;
}

- (BOOL) isCompatible
{
	return (self.score > 0);
}

@end

@interface AVCaptureDevice (CompatibilityCheck)
- (BOOL) isCompatible;
- (AVCaptureDeviceFormat *) compatibleFormat;
@end
@implementation AVCaptureDevice (CompatibilityCheck)

- (BOOL) isCompatible
{
	return ([self compatibleFormat] != nil);
}

- (AVCaptureDeviceFormat *) compatibleFormat
{
	/*
	 Looking for LPCM 44100Hz 16/24/32bpc
	 
	 Format samples

	 // VM
	 <AVCaptureDeviceFormat: 0x600003331900> 'soun'/'lpcm' SR=192000,FF=30,BPP=8,FPP=1,BPF=8,CH=2,BPC=32,
	 <AVCaptureDeviceFormat: 0x600003331b80> 'soun'/'lpcm' SR=176400,FF=30,BPP=8,FPP=1,BPF=8,CH=2,BPC=32,
	 <AVCaptureDeviceFormat: 0x600003332620> 'soun'/'lpcm' SR=96000,FF=30,BPP=8,FPP=1,BPF=8,CH=2,BPC=32,
	 <AVCaptureDeviceFormat: 0x600003332120> 'soun'/'lpcm' SR=88200,FF=30,BPP=8,FPP=1,BPF=8,CH=2,BPC=32,
	 <AVCaptureDeviceFormat: 0x600003332260> 'soun'/'lpcm' SR=48000,FF=30,BPP=8,FPP=1,BPF=8,CH=2,BPC=32,
	 <AVCaptureDeviceFormat: 0x600003331ea0> 'soun'/'lpcm' SR=44100,FF=30,BPP=8,FPP=1,BPF=8,CH=2,BPC=32

	 // iMac
	 <AVCaptureDeviceFormat: 0x610000001270> 'soun'/'lpcm' SR=96000,FF=4,BPP=8,FPP=1,BPF=8,CH=2,BPC=24,
	 <AVCaptureDeviceFormat: 0x6100000011d0> 'soun'/'lpcm' SR=88200,FF=4,BPP=8,FPP=1,BPF=8,CH=2,BPC=24,
	 <AVCaptureDeviceFormat: 0x6100000012d0> 'soun'/'lpcm' SR=48000,FF=4,BPP=8,FPP=1,BPF=8,CH=2,BPC=24,
	 <AVCaptureDeviceFormat: 0x6100000012f0> 'soun'/'lpcm' SR=44100,FF=4,BPP=8,FPP=1,BPF=8,CH=2,BPC=24,
	 <AVCaptureDeviceFormat: 0x610000001310> 'soun'/'lpcm' SR=32000,FF=4,BPP=8,FPP=1,BPF=8,CH=2,BPC=24,
	 <AVCaptureDeviceFormat: 0x610000001330> 'soun'/'lpcm' SR=96000,FF=4,BPP=8,FPP=1,BPF=8,CH=2,BPC=20,
	 <AVCaptureDeviceFormat: 0x610000001350> 'soun'/'lpcm' SR=88200,FF=4,BPP=8,FPP=1,BPF=8,CH=2,BPC=20,
	 <AVCaptureDeviceFormat: 0x610000001370> 'soun'/'lpcm' SR=48000,FF=4,BPP=8,FPP=1,BPF=8,CH=2,BPC=20,
	 <AVCaptureDeviceFormat: 0x610000001390> 'soun'/'lpcm' SR=44100,FF=4,BPP=8,FPP=1,BPF=8,CH=2,BPC=20,
	 <AVCaptureDeviceFormat: 0x6100000013b0> 'soun'/'lpcm' SR=32000,FF=4,BPP=8,FPP=1,BPF=8,CH=2,BPC=20,
	 <AVCaptureDeviceFormat: 0x6100000013d0> 'soun'/'lpcm' SR=96000,FF=12,BPP=4,FPP=1,BPF=4,CH=2,BPC=16,
	 <AVCaptureDeviceFormat: 0x6100000013f0> 'soun'/'lpcm' SR=88200,FF=12,BPP=4,FPP=1,BPF=4,CH=2,BPC=16,
	 <AVCaptureDeviceFormat: 0x610000001410> 'soun'/'lpcm' SR=48000,FF=12,BPP=4,FPP=1,BPF=4,CH=2,BPC=16,
	 <AVCaptureDeviceFormat: 0x610000001430> 'soun'/'lpcm' SR=44100,FF=12,BPP=4,FPP=1,BPF=4,CH=2,BPC=16,
	 <AVCaptureDeviceFormat: 0x610000001450> 'soun'/'lpcm' SR=32000,FF=12,BPP=4,FPP=1,BPF=4,CH=2,BPC=16,
	 <AVCaptureDeviceFormat: 0x610000001470> 'soun'/'lpcm' SR=96000,FF=9,BPP=8,FPP=1,BPF=8,CH=2,BPC=32,
	 <AVCaptureDeviceFormat: 0x610000001490> 'soun'/'lpcm' SR=88200,FF=9,BPP=8,FPP=1,BPF=8,CH=2,BPC=32,
	 <AVCaptureDeviceFormat: 0x6100000014b0> 'soun'/'lpcm' SR=48000,FF=9,BPP=8,FPP=1,BPF=8,CH=2,BPC=32,
	 <AVCaptureDeviceFormat: 0x6100000014d0> 'soun'/'lpcm' SR=44100,FF=9,BPP=8,FPP=1,BPF=8,CH=2,BPC=32,
	 <AVCaptureDeviceFormat: 0x6100000014f0> 'soun'/'lpcm' SR=32000,FF=9,BPP=8,FPP=1,BPF=8,CH=2,BPC=32
	 */
	AVCaptureDeviceFormat *f = nil;
	size_t bestScore = 0;
	for (AVCaptureDeviceFormat *format in self.formats)
	{
		size_t score = format.score;

		if (score == 0 || score < bestScore)
			continue;
		bestScore = score;
		f = format;
	}
	return f;
}

@end

@interface MacOSNativeSoundCapture : NSObject<AVCaptureAudioDataOutputSampleBufferDelegate>

@property(nonatomic, retain) NSError* captureError;

@end

@implementation MacOSNativeSoundCapture {
	AVCaptureSession* _captureSession;
	
	dispatch_queue_t _sessionQueue;
	dispatch_queue_t _audioCaptureQueue;
	dispatch_queue_t _delegateQueue;
	
	BOOL _running;

	MacOSSoundManager* _delegate;

	// vDSP
	FFTSetup _fftsetup;
	DSPSplitComplex	_splitComplex;
	float* _window;
	float* _leftSamples;
	float* _rightSamples;
	size_t _samplePosition;
	uint8_t _log2n;
	
	void*  _contiguousBuffer;
	size_t _contiguousBufferSize;
}

- (instancetype) initWithDelegate:(MacOSSoundManager*)delegate queue:(dispatch_queue_t)queue
{
	self = [super init];
	if (self) {
		_running = NO;
		_delegate = delegate;
		dispatch_retain(queue);
		_delegateQueue = queue;
		_sessionQueue = dispatch_queue_create("com.prismatik.avcapture.session", DISPATCH_QUEUE_SERIAL);
		_log2n = log2(_delegate->fftSize() * 2);
		_fftsetup = vDSP_create_fftsetup(_log2n, kFFTRadix2);
		_window = (float *)calloc(_delegate->fftSize() * 2, sizeof(float));
		_leftSamples = (float *)calloc(_delegate->fftSize() * 2, sizeof(float));
		_rightSamples = (float *)calloc(_delegate->fftSize() * 2, sizeof(float));
		_splitComplex.realp = (float *)calloc(_delegate->fftSize() * 2, sizeof(float));
		_splitComplex.imagp = (float *)calloc(_delegate->fftSize() * 2, sizeof(float));
		
		vDSP_hann_window(_window, _delegate->fftSize() * 2, 0);// hamm, hann
		_samplePosition = 0;
		
		_contiguousBuffer = NULL;
		_contiguousBufferSize = 0;
	}
	return self;
}

- (void) dealloc
{
	self.captureError = nil;
	[_captureSession stopRunning];
	[self destroySession];
	dispatch_release(_sessionQueue);
	dispatch_release(_audioCaptureQueue);
	dispatch_release(_delegateQueue);

	vDSP_destroy_fftsetup(_fftsetup);
	free(_window);
	free(_leftSamples);
	free(_rightSamples);
	free(_splitComplex.realp);
	free(_splitComplex.imagp);
	
	if (_contiguousBuffer)
		free(_contiguousBuffer);
	
	[super dealloc];
}

- (void) createSession
{
	if (_captureSession)
		return;
	_captureSession = [[AVCaptureSession alloc] init];

//	AVCaptureDevice *defaultAudioDevice = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeAudio];
//	[self setCaptureDevice:defaultAudioDevice];
	
	AVCaptureAudioDataOutput* captureAudioDataOutput = [[AVCaptureAudioDataOutput alloc] init];
	// Put audio on its own queue to ensure that our video processing doesn't cause us to drop audio
	_audioCaptureQueue = dispatch_queue_create("com.prismatik.avcapture.audio", DISPATCH_QUEUE_SERIAL);
	dispatch_set_target_queue(_audioCaptureQueue, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0));
	[captureAudioDataOutput setSampleBufferDelegate:self queue:_audioCaptureQueue];
	
	if ([_captureSession canAddOutput:captureAudioDataOutput])
		[_captureSession addOutput:captureAudioDataOutput];
	[captureAudioDataOutput release];
	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(captureSessionNotification:) name:nil object:_captureSession];
}

- (void) destroySession
{
	if (_captureSession) {
		[[NSNotificationCenter defaultCenter] removeObserver:self];
		[_captureSession release];
	}
	_running = NO;
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
	[self destroySession];
	
	@synchronized(self) {
		self.captureError = error;
		dispatch_sync(_delegateQueue, ^{ _delegate->start(false); });
	}
}

- (void) captureSessionDidStopRunning
{
	[self destroyAudioPipeline];
}

// synchronous, blocks until the pipeline is drained, don't call from within the pipeline
- (void) destroyAudioPipeline
{
	// The session is stopped so we are guaranteed that no new buffers are coming through the video data output.
	// There may be inflight buffers on _audioCaptureQueue however.
	// Synchronize with that queue to guarantee no more buffers are in flight.
	// Once the pipeline is drained we can tear it down safely.
	dispatch_sync(_audioCaptureQueue, ^{
		
	});
}

- (BOOL) setCaptureDevice:(AVCaptureDevice *)audioDevice
{
	if (_running)
		[_captureSession stopRunning];
	
	AVCaptureDeviceInput *currentAudioIn = _captureSession.inputs.lastObject;
	if (currentAudioIn)
		[_captureSession removeInput:currentAudioIn];

	AVCaptureDeviceInput *audioIn = [[AVCaptureDeviceInput alloc] initWithDevice:audioDevice error:nil];

	if ([_captureSession canAddInput:audioIn]) {
		[_captureSession addInput:audioIn];
		[audioIn release];
	} else {
		_running = NO;
		[audioIn release];
		return NO;
	}

	NSError* error = nil;
	[audioDevice lockForConfiguration:&error];
	if (error == nil) {
		audioDevice.activeFormat = audioDevice.compatibleFormat; // get lpcm 44100 Hz
		[audioDevice unlockForConfiguration];
	} else {
		qWarning() << Q_FUNC_INFO << "could not set audio format: " << error.localizedDescription;
		if (!audioDevice.activeFormat.isCompatible)
		{
			qCritical() << Q_FUNC_INFO << "active format is not supported: " << audioDevice.activeFormat.description;
			_running = NO;
			return NO;
		}
	}
	if (_running)
		[_captureSession startRunning];
	return YES;
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

- (size_t) copyBuffer:(const char* const)srcBuffer
			   stride:(const uint8_t)stride
				count:(const size_t)count
		  description:(const AudioStreamBasicDescription* const)desc
			toChannel:(float* const)destBuffer
{
	if (desc->mFormatFlags & kLinearPCMFormatFlagIsSignedInteger)
	{
		if (desc->mBitsPerChannel == sizeof(short) * 8)
			vDSP_vflt16(reinterpret_cast<const short *>(srcBuffer), stride, destBuffer, 1, count);
		else if (desc->mBitsPerChannel == sizeof(vDSP_int24) * 8)
			vDSP_vflt24(reinterpret_cast<const vDSP_int24 *>(srcBuffer), stride, destBuffer, 1, count);
		else if (desc->mBitsPerChannel == sizeof(int) * 8)
			vDSP_vflt32(reinterpret_cast<const int *>(srcBuffer), stride, destBuffer, 1, count);
		else {
			DEBUG_HIGH_LEVEL << Q_FUNC_INFO << "unsupported integer BPC: " << desc->mBitsPerChannel;
			return 0;
		}
	} else if (desc->mFormatFlags & kLinearPCMFormatFlagIsFloat) {
		if (desc->mBitsPerChannel == sizeof(float) * 8) // float to float
			floatcpy<float>(srcBuffer, stride, destBuffer, count);
		else if (desc->mBitsPerChannel == sizeof(double) * 8) // double to float
			vDSP_vdpsp(reinterpret_cast<const double *>(srcBuffer), stride, destBuffer, 1, count);
		else {
			DEBUG_HIGH_LEVEL << Q_FUNC_INFO << "unsupported float BPC: " << desc->mBitsPerChannel;
			return 0;
		}
	} else {
		DEBUG_HIGH_LEVEL << Q_FUNC_INFO << "integer/float flags not set: " << desc->mFormatFlags;
		return 0;
	}
	return count;
}

- (size_t) copySamples:(const char* const)leftBuffer rightBuffer:(const char* const)rightBuffer stride:(const size_t)stride count:(size_t)count description:(const AudioStreamBasicDescription* const)desc
{
	count = MIN(count, _delegate->fftSize() * 2 - _samplePosition);
	
	if ([self copyBuffer:leftBuffer stride:stride count:count description:desc toChannel:(_leftSamples + _samplePosition)] != count)
		return 0;

	if (rightBuffer && [self copyBuffer:rightBuffer stride:stride count:count description:desc toChannel:(_rightSamples + _samplePosition)] == count) {
		// downmix to mono
		// common way of doing
		// left = (left + right) / 2
		const float _b = 1.0f;
		vDSP_vavlin(_rightSamples + _samplePosition, 1, &_b, _leftSamples + _samplePosition, 1, count);

		// left = max(left, right)
		// this alternative produces visually same-ish results
//		vDSP_vmax(_rightSamples + _samplePosition, 1, _leftSamples + _samplePosition, 1, _leftSamples + _samplePosition, 1, count); // downmix to mono
	}

	return count;
}

- (void) captureOutput:(AVCaptureOutput *)output didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection
{
	Q_UNUSED(output);
	Q_UNUSED(connection);

	const size_t numSamples = (size_t)CMSampleBufferGetNumSamples(sampleBuffer);
	if (numSamples == 0) {
		DEBUG_HIGH_LEVEL << Q_FUNC_INFO << "no samples";
		return;
	}

	CMAudioFormatDescriptionRef format = CMSampleBufferGetFormatDescription(sampleBuffer);
	if (format == NULL) {
		DEBUG_HIGH_LEVEL << Q_FUNC_INFO << "could not get audio format description";
		return;
	}

	const AudioStreamBasicDescription * const desc = CMAudioFormatDescriptionGetStreamBasicDescription(format);
	if (desc == NULL) {
		DEBUG_HIGH_LEVEL << Q_FUNC_INFO << "could not get ASBD";
		return;
	}

	if (desc->mFormatID != kAudioFormatLinearPCM) {
		DEBUG_HIGH_LEVEL << Q_FUNC_INFO << "unsupported format ID: " << desc->mFormatID;
		return;
	}

	if ((desc->mFormatFlags & kLinearPCMFormatFlagIsPacked) == 0 && ((desc->mBitsPerChannel / 8) * desc->mChannelsPerFrame) != desc->mBytesPerFrame) {
		DEBUG_HIGH_LEVEL << Q_FUNC_INFO << "unsupported format flags: " << desc->mFormatFlags;
		return;
	}

	CMBlockBufferRef audioBuffer = CMSampleBufferGetDataBuffer(sampleBuffer);
	if (audioBuffer == NULL) {
		DEBUG_HIGH_LEVEL << Q_FUNC_INFO << "could not get audioBuffer";
		return;
	}

	size_t totalLength = 0;
	if (CMBlockBufferGetDataPointer(audioBuffer, 0, NULL, &totalLength, NULL) != kCMBlockBufferNoErr || totalLength == 0) {
		DEBUG_HIGH_LEVEL << Q_FUNC_INFO << "could not get totalLength";
		return;
	}
	
	char *inSamplesLeft = NULL;
	if (totalLength > _contiguousBufferSize) {
		_contiguousBuffer = realloc(_contiguousBuffer, sizeof(*inSamplesLeft) * totalLength);
		_contiguousBufferSize = totalLength;
	}
	if (_contiguousBuffer == NULL || CMBlockBufferAccessDataBytes(audioBuffer, 0, totalLength, _contiguousBuffer, &inSamplesLeft) != kCMBlockBufferNoErr || inSamplesLeft == NULL) {
		DEBUG_HIGH_LEVEL << Q_FUNC_INFO << "could not get contiguous data bytes";
		return;
	}
	
	/**
	// This does not want to work, even from LPCM (stereo) to LPCM (mono)
	// Unless input format == output format, AudioConverterConvertBuffer ALWAYS returns -50
	// Tested with 10.13 and 10.14 SDKs

	AVAudioFormat* monoFormat = [[AVAudioFormat alloc] initWithCommonFormat:AVAudioPCMFormatFloat32 sampleRate:44100 channels:1 interleaved:NO];
	AudioConverterRef converter = NULL;

	// Using newly created AVAudioFormat.streamDescription or copying input stream description with 1 channel yields same results
	AudioConverterNew(desc, monoFormat.streamDescription, &converter);
	UInt32 ioMonoSize = totalLength; // this assumes 32bit to 32bit (or less) conversion, but even very large buffers don't help
	void *monoSamples = malloc(sizeof(*inSamplesLeft) * ioMonoSize);
	OSStatus convertStatus = AudioConverterConvertBuffer(converter, (UInt32)totalLength, (const void *)inSamplesLeft, &ioMonoSize, monoSamples);
	// convertStatus is ALWAYS "-50"
	// Documentation states:
	// 		"This function is for the special case of converting from one linear PCM format to another."
	// which is what we want and (trying to) do
	// 		"This function cannot perform sample rate conversions and cannot be used for conversion to or from most compressed formats."
	// which is what we DO NOT do
	// so in theory this should be enough to do LPCM stereo to mono

	AudioConverterDispose(converter);
	free(monoSamples);
	[monoFormat release];
	//*/

	const size_t bytesPerSample = desc->mBitsPerChannel / 8;
	const char* inSamplesRight = NULL;
	size_t stride = 1;
	if (desc->mChannelsPerFrame > 1) {
		if (desc->mFormatFlags & kLinearPCMFormatFlagIsNonInterleaved)
			inSamplesRight = inSamplesLeft + (totalLength / desc->mChannelsPerFrame); // step over all left samples
		else {
			inSamplesRight = inSamplesLeft + bytesPerSample; // step over 1 left sample
			stride = desc->mChannelsPerFrame;
		}
	}

	@synchronized (self) {
		
		for (size_t totalCopied = 0; totalCopied < numSamples;)
		{
			const size_t offset = bytesPerSample * totalCopied * stride;
			const size_t copiedSamples = [self copySamples:inSamplesLeft + offset
											   rightBuffer:(inSamplesRight ? inSamplesRight + offset : NULL)
													stride:stride
													 count:(numSamples - totalCopied)
											   description:desc];

			totalCopied += copiedSamples;
			_samplePosition += copiedSamples;
			if (copiedSamples == 0 || (_samplePosition < _delegate->fftSize() * 2))
				return;

			// Convert the real data to complex data
			// Window the samples
			vDSP_vmul(_leftSamples,  1, _window, 1, _leftSamples,  1, _delegate->fftSize() * 2);

			// copy the input to the packed complex array that the fft algo uses
			vDSP_ctoz((DSPComplex *)_leftSamples, 2, &_splitComplex, 1, _delegate->fftSize());
			
			// calculate the fft
			vDSP_fft_zrip(_fftsetup, &_splitComplex, 1, _log2n, FFT_FORWARD);
			
			const float scale = 1.0 / _delegate->fftSize();
			vDSP_vsmul(_splitComplex.realp, 1, &scale, _splitComplex.realp, 1, _delegate->fftSize());
			vDSP_vsmul(_splitComplex.imagp, 1, &scale, _splitComplex.imagp, 1, _delegate->fftSize());

			vDSP_zvabs(&_splitComplex, 1, _delegate->fft(), 1, _delegate->fftSize());

#ifndef QT_NO_DEBUG
			const float binres = desc->mSampleRate / (_delegate->fftSize() * 2);
			size_t maxbin = 0;
			float maxmag = 0.0;
			vDSP_maxvi(_delegate->fft(), 1, &maxmag, &maxbin, _delegate->fftSize());
			NSLog(@"[%zu] %.2fHz = %f\n", maxbin, static_cast<float>(binres * maxbin + binres / 2), maxmag);
			// https://www.youtube.com/watch?v=TbPh0pmNjo8
			// full volume
			// [46] 1001.29Hz = 0.289
			// windows+wasapi = 0.289404094
#endif // QT_NO_DEBUG
			
			dispatch_sync(_delegateQueue, ^{
				_delegate->updateColors();
			});
			_samplePosition = 0;
		} // for totalCopied
	} // @synchronize
}

@end

MacOSSoundManager::MacOSSoundManager(QObject *parent) : SoundManagerBase(parent)
{
	_capture = [[MacOSNativeSoundCapture alloc] initWithDelegate:this queue:dispatch_get_main_queue()];
}

MacOSSoundManager::~MacOSSoundManager()
{
	[_capture stop];
	[_capture destroySession];
	[_capture release];
}

bool MacOSSoundManager::init() {

#if __MAC_OS_X_VERSION_MAX_ALLOWED >= __MAC_10_14
	if (@available(macOS 10.14, *)) {
		AVAuthorizationStatus status = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeAudio];

		if (status == AVAuthorizationStatusNotDetermined) {
			if (m_awaitingAuthorization) {
				qDebug() << Q_FUNC_INFO << "Audio capture: already requested authorization";
				return false;
			}
			m_awaitingAuthorization = true;
			qWarning() << Q_FUNC_INFO << "Audio capture: authorization not determined, requesting access";
			__block bool isEnabledBeforeAskingAuthorization = m_isEnabled;
			[AVCaptureDevice requestAccessForMediaType:AVMediaTypeAudio completionHandler:^(BOOL granted) {
				if (granted) {
					qDebug() << Q_FUNC_INFO << "Audio capture: access granted";
					dispatch_sync(dispatch_get_main_queue(), ^{
						m_awaitingAuthorization = false;
						if (init()) {
							requestDeviceList(); // resets m_isEnabled
							if (isEnabledBeforeAskingAuthorization)
								start(isEnabledBeforeAskingAuthorization);
						}
					});
				} else
					qCritical() << Q_FUNC_INFO << "Audio capture: access denied";
			}];
		}
		m_isAuthorized = (status == AVAuthorizationStatusAuthorized);
		if (!m_isAuthorized) {
			qCritical() << Q_FUNC_INFO << "Audio capture: access denied, aborting";
			return false;
		}
	} else
#endif // __MAC_10_14
	{
		// Fallback on earlier versions
		m_isAuthorized = true;
	}
	
	[_capture createSession];
	m_isInited = true;
	return true;
}

void MacOSSoundManager::populateDeviceList(QList<SoundManagerDeviceInfo>& deviceList, int& recommendedIdx)
{
	__block QList<SoundManagerDeviceInfo>& devices = deviceList;
	__block int& recommended = recommendedIdx;

	__block AVCaptureDevice *defaultDevice = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeAudio];

	[[AVCaptureDevice devicesWithMediaType:AVMediaTypeAudio] enumerateObjectsUsingBlock:^(AVCaptureDevice* device, NSUInteger index, BOOL *stop)
	{
		Q_UNUSED(stop);
		BOOL compatible = [device isCompatible];

		QString name = QString(device.localizedName.UTF8String);
		if (!compatible)
			name += tr(" (not supported)");

		devices.append(SoundManagerDeviceInfo(name, index));

		if (device == defaultDevice && compatible)
			recommended = index;
		else if (compatible)
			recommended = index;
	}];

	if (recommended == -1)
		qCritical() << Q_FUNC_INFO << "No supported devices (LPCM, 44100Hz, 16/24/32 BPC)";
}

void MacOSSoundManager::start(bool isEnabled)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << isEnabled;

	if (m_isEnabled == isEnabled)
		return;

	m_isEnabled = isEnabled;

	if (m_isEnabled)
	{
		if (!m_isInited)
		{
			if (m_awaitingAuthorization)
				return;
			if (!init()) {
				m_isEnabled = false;
				return;
			}
		}

		NSArray* devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeAudio];
		AVCaptureDevice* device = nil;

		if (m_device == -1 || (int)devices.count <= m_device) {
			// Fallback: first supported device (recommended)
			for (AVCaptureDevice* d in devices) {
				if (d.isCompatible) {
					device = d;
					break;
				}
			}
			
			if (device == nil) {
				qWarning() << Q_FUNC_INFO << "No saved device. No auto fallback device found.";
			} else {
				DEBUG_LOW_LEVEL << Q_FUNC_INFO << "No saved device. Falling back to first supported device: " << device.description.UTF8String << " (" << device.activeFormat.description.UTF8String << ")";
				m_device = [devices indexOfObject:device];
			}
		} else {
			device = devices[m_device];
			if (!device.isCompatible) {
				qCritical() << Q_FUNC_INFO << "Selected device is not supported: " << device.description.UTF8String << " (" << device.activeFormat.description.UTF8String << ")";
				return;
			}
		}

		if (device)
		{
			if (![_capture setCaptureDevice:device])
			{
				qCritical() << Q_FUNC_INFO << "Failed to set device: " << device.description.UTF8String << " (" << device.activeFormat.description.UTF8String << ")";
				return;
			}
			[_capture start];
		}
	}
	else
	{
		[_capture stop];
	}

	if (m_visualizer == nullptr)
		return;
	if (m_isEnabled)
		m_visualizer->start();
	else
		m_visualizer->stop();
}
